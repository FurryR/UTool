#ifndef _EDITOR_H
#define _EDITOR_H

#include <conio.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <regex>
#include <thread>
#include <utility>

#include "parser.h"
#include "project.h"
#include "screen.h"

/**
 * @brief 通过 NoteNum 返回对应音符名称
 *
 * @param note_num
 * @return std::string
 */
std::string get_key_name(int note_num) {
    constexpr std::array<const char *, 12> ref{
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    if (note_num < 24 || note_num > 108) return "??";
    return ref[(note_num - 24) % 12] +
           std::to_string((size_t)ceil((note_num - 24) / 12.0));

    // 鸡巴忘了
    //  *要考试了。
}
size_t get_note_num(const std::string &key) {
    constexpr std::array<const char *, 12> ref{
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    std::string key_name = key.substr(0, key.length() - 1);
    size_t octave = key[key.length() - 1] - '0';
    std::array<const char *, 12>::const_iterator f =
        std::find(ref.cbegin(), ref.cend(), key_name);
    if (f == ref.cend() || octave < 1 || octave > 9) {
        return 0;
    }
    return f - ref.cbegin() + octave * 12 + 12;
}
// size_t get_length(double second, double tempo) {
//     // tick / 480.0 / tempo * 60.0 = seconds
//     // tick / 480.0 / tempo        = seconds / 60.0
//     // tick / 480.0                = seconds / 60.0 * tempo
//     // tick                        = seconds / 60.0 * tempo * 480.0
//     return (size_t)(second / 60.0 * tempo * 480.0);
// }
/**
 * @brief 彩色文字支持
 */
typedef struct ColorText {
    std::string prefix;
    std::string content;

    std::vector<Character> output() const {
        std::vector<Character> ret = std::vector<Character>(content.length());
        for (size_t i = 0; i < content.length(); i++)
            ret[i] = Character(content[i], prefix);
        return ret;
    }

    ColorText() = default;

    ColorText(const std::string &content, const std::string &prefix)
        : prefix(prefix), content(content) {}
} ColorText;

/**
 * @brief 获得窗口大小
 *
 * @return Coord x和y，代表窗口大小
 */
Coord getsize() {
    Coord ret = Coord(0, 0);
    std::cout << "\x1b[s\x1b[9999;9999H\x1b[6n\x1b[u";
    _getch();
    _getch();
    for (char ch; (ch = _getch()) != ';'; ret.y = ret.y * 10 + (ch - '0'))
        ;
    for (char ch; (ch = _getch()) != 'R'; ret.x = ret.x * 10 + (ch - '0'))
        ;
    return ret;
}

typedef struct UI {
    /**
     * @brief 显示上方的信息。
     *
     * @param project 当前的工程。
     * @param page    当前的页面
     * @param total   总共的页面数。
     * @param dirty   是否绘制*
     */
    void render_bar(const Project &project, size_t page, size_t total,
                    bool dirty) {
        // 进行一些邪术的施展
        // 当前绘制的位置
        size_t x = 0;  // currentX
        render_text(&x, 0, ColorText("upet-tui", "\x1b[38;2;114;159;207m"));
        x += 1;
        render_text(
            &x, 0,
            ColorText(dirty ? project.project_name + "*" : project.project_name,
                      ""));
        x += 2;
        render_text(&x, 0, ColorText("Tempo:", "\x1b[38;2;114;159;207m"));
        x += 1;
        render_text(&x, 0, ColorText(std::to_string(project.tempo), ""));
        x += 2;
        render_text(&x, 0,
                    ColorText("Global Flags:", "\x1b[38;2;114;159;207m"));
        x += 1;
        render_text(&x, 0, ColorText(project.global_flags, ""));
        x += 2;
        render_text(&x, 0, ColorText("Notes:", "\x1b[38;2;114;159;207m"));
        x += 1;
        render_text(&x, 0, ColorText(std::to_string(project.notes.size()), ""));
        x = 0;
        // Line 2
        render_text(&x, 1, ColorText("Note", "\x1b[30;47m"));
        x += (16 - 4) + 1;
        render_text(&x, 1, ColorText("NoteNum", "\x1b[30;47m"));
        x += (16 - 7) + 1;
        render_text(&x, 1, ColorText("Lyric", "\x1b[30;47m"));
        x += (16 - 5) + 1;
        render_text(&x, 1, ColorText("Length", "\x1b[30;47m"));
        x += (16 - 6) + 1;
        render_text(&x, 1, ColorText("Velocity", "\x1b[30;47m"));
        x += (16 - 8) + 1;
        render_text(&x, 1, ColorText("Flags", "\x1b[30;47m"));
        x += 1;
        render_text(&x, 1, ColorText("Page:", ""));
        x += 1;
        render_text(
            &x, 1,
            ColorText(std::to_string(page + 1) + "/" + std::to_string(total),
                      ""));

        // std::cout << "\x1b"
        //           << "c";
        // 必要なし
        // std::cout << "\x1b[30;47m"
        //           << "uPET Editor:"
        //           << "\x1b[0m " << project.project_name
        //           << "  \x1b[30;47mTempo:\x1b[0m " << project.tempo
        //           << "  \x1b[30;47mGlobal Flags:\x1b[0m "
        //           << project.global_flags << std::endl;

        // std::cout << "\x1b[30;47m" // 先不要动
        //           << "Note" << std::setw(8) << "NoteNum" << std::setw(8)
        //           << "Lyric" << std::setw(16) << "Length" << std::setw(16)
        //           << "Velocity" << std::setw(8) << "Flags"
        //           << "\x1b[0m"
        //           << "  Page: " << (page + 1) << "/" << total << std::endl;
    }

    /**
     * @brief 渲染指定的音符。
     *
     * @param note  音符。
     * @param count 音符的位置（从0开始）。
     * @param y     需要渲染的 y 轴。
     * @param column 当前高亮的列位置。
     * @param tempo 当前项目的 bpm。
     * @param show_sec 是否显示秒数。
     * @param show_act 是否显示实际的 key 值。
     * @param highlight 是否高亮当前的行数。
     */
    void render_note(const Note &note, size_t count, size_t y, size_t column,
                     double tempo, bool show_sec, bool show_act,
                     bool highlight) {
        // pos + page * (size().y - 3)
        // column 可能是 0 - 4，对应 NoteNum，Lyric 等
        // 0: NoteNum, 1: Lyric, 2: Length, 3: Velocity, 4: Flags

        std::string lyric_base;
        std::string hi_base;
        std::string temp;
        // 当前绘制的位置
        size_t x = 0;  // currentX
        if (note.Lyric == "R") {
            lyric_base = "\x1b[38;2;64;64;64m";
        }

        hi_base = lyric_base + (highlight ? "\x1b[30;47m" : "");
        // 序号
        temp = std::to_string(count + 1);
        render_text(&x, y, ColorText(temp, lyric_base));
        x += (16 - temp.length()) + 1;  // 自适应，对照上方
        // 音高
        temp = show_act ? (get_key_name(note.NoteNum) + " (" +
                           std::to_string(note.NoteNum) + ")")
                        : get_key_name(note.NoteNum);
        render_text(&x, y,
                    ColorText(temp, (column == 0) ? hi_base : lyric_base));
        x += (16 - temp.length()) + 1;
        // 歌词(可能为空)
        temp = note.Lyric.empty() ? "  " : note.Lyric;
        render_text(
            &x, y,
            ColorText(temp.length() >= 16 ? (temp.substr(0, 13) + "...") : temp,
                      lyric_base +
                          (temp.length() >= 16 ? "\x1b[38;2;252;233;79m" : "") +
                          ((column == 1) ? hi_base : lyric_base)));
        x += (16 - (temp.length() >= 16 ? 16 : temp.length())) + 1;
        // 长度
        temp = std::to_string(note.Length) +
               (show_sec ? (" (" +
                            std::to_string((note.Length / 480.0 / tempo * 60.0))
                                .substr(0, 5) +
                            "s)")
                         : "");
        render_text(&x, y,
                    ColorText(temp, (column == 2) ? hi_base : lyric_base));
        x += (16 - temp.length()) + 1;
        // ???
        temp = std::to_string(note.Velocity);
        render_text(&x, y,
                    ColorText(temp, (column == 3) ? hi_base : lyric_base));
        x += (16 - temp.length()) + 1;
        // flags(可能为空)
        temp = note.Flags.empty() ? "  " : note.Flags;
        render_text(
            &x, y,
            ColorText(temp.length() >= 16 ? (temp.substr(0, 13) + "...") : temp,
                      lyric_base +
                          (temp.length() >= 16 ? "\x1b[38;2;252;233;79m" : "") +
                          ((column == 4) ? hi_base : lyric_base)));
    }

    /**
     * @brief 显示 log。
     *
     * @param text 文本。
     */
    void render_log(const std::vector<Character> &text) {
        size_t x = 0;
        for (; x < screen->size().x; x++) {
            screen->set(Coord(x, screen->size().y - 1), Character(0));
        }
        x = 0;
        for (auto &&s : text) {
            screen->set(Coord(x, screen->size().y - 1), s);
            x++;
        }
    }

    /**
     * @brief 更新画面。
     */
    void update() { screen->show(); }

    /**
     * @brief 清屏。
     */
    void clear() { screen->clear(); }

    /**
     * @brief 获得窗口的大小。
     *
     * @return const Coord& 对窗口大小的只读引用。
     */
    const Coord &size() const noexcept { return screen->size(); }

    explicit UI(Screen *screen) : screen(screen) {}

   private:
    void render_text(size_t *x, size_t y, const ColorText &text) {
        for (auto &&s : text.output()) {
            screen->set(Coord(*x, y), s);
            (*x)++;
        }
    }

    Screen *screen;
} UI;
/**
 * @brief 辅助函数——文本光标
 *
 * @param str 用户输入的文字
 * @param index 光标位置。最多为 str.length()。
 * @return std::vector<Character> 带颜色的字符。
 */
std::vector<Character> text_cursor(const std::string &str, size_t index) {
    std::vector<Character> tmp;
    for (size_t i = 0; i < str.length(); i++) {
        if (i == index)
            tmp.push_back(Character(str[i], "\x1b[38;5;247m\x1b[47m"));
        else
            tmp.push_back(Character(str[i]));
    }
    if (index == str.length()) tmp.push_back(Character(0, "\x1b[47m"));
    return tmp;
}
typedef enum class ActionType {
    InsertBefore = 0,
    InsertAfter = 1,
    Remove = 2,
    Modify = 3,
} ActionType;
typedef struct Action {
    std::string value;
    size_t index;
    ActionType action;
    Action() = default;
    Action(const ActionType &action, const std::string &value, size_t index)
        : value(value), index(index), action(action) {}
};
typedef struct Editor {
    size_t count;
    size_t column;

    /**
     * @brief 加载指定的 Project 实例。
     *
     * @param p Project 实例
     */
    void load(const Project &p) {
        _project = p;
        _dirty = false;
        count = 0;
    }
    /**
     * @brief 根据 filename 打开文件。解析错误时会抛出错误。
     *
     * @param filename 文件名
     * @return true 打开成功。
     * @return false 打开失败。
     */
    bool open(const std::string &filename) {
        std::ifstream f(filename);
        if (!f) return false;
        std::string raw = std::string(std::istreambuf_iterator<char>(f),
                                      std::istreambuf_iterator<char>());
        f.close();
        load(Project::parse(INI::parse(raw)));
        _path = filename;
        return true;
    }
    /**
     * @brief 保存到指定文件名。
     *
     * @param filename 文件名。
     * @return true 保存成功。
     * @return false 保存失败。
     */
    bool save(const std::string &filename) {
        std::ofstream f(filename);
        if (!f) {
            return false;
        }
        f << project().to_string();
        f.close();
        _path = filename;
        _dirty = false;
        return true;
    }
    /**
     * @brief 输入的处理事件。
     *
     * @param ui UI 实例。
     * @param input 输入的字符。
     * @return bool 是否更新屏幕。
     */
    bool process(UI *ui, int input) {
        switch (input) {
            case 224: {
                // 功能键？
                // 草 竟然可以这样写 sure
                switch (_getch()) {
                    case 72: {
                        // 上键
                        if (count > 0) count--;
                        break;
                    }
                    case 80: {
                        // 下一个音符
                        if (count < project().notes.size() - 1) count++;
                        break;
                    }
                    case 75: {
                        // 左键（上一个单元格）
                        if (column == 0)
                            column = 4;
                        else
                            column--;
                        break;
                    }
                    case 77: {
                        // 右键（下一个单元格）
                        if (column == 4)
                            column = 0;
                        else
                            column++;
                        break;
                    }
                    case 73: {
                        // PageUp（上一页）
                        if (count > (ui->size().y - 3))
                            count -= ui->size().y - 3;
                        else
                            count = 0;
                        break;
                    }
                    case 81: {
                        // PageDown（下一页）
                        if (count + (ui->size().y - 3) <
                            project().notes.size() - 1) {
                            count += ui->size().y - 3;
                        } else {
                            count = project().notes.size() - 1;
                        }
                        break;
                    }
                }
                break;
            }
            case 'T':
            case 't': {
                // 显示tick转换为的时长
                // 显示音高对应的音阶
                // 通用：转换
                if (column == 0)
                    toggle_show_actual_notenum();
                else if (column == 2)
                    toggle_show_sec();

                break;
            }
            case 'N':
            case 'n': {
                // 在前面插入音符
                insert_note_before();
                break;
            }
            case 'M':
            case 'm': {
                // 在后面插入音符
                insert_note_after();
                break;
            }
            case '\b': {
                // 删除当前音符
                remove_note();
                break;
            }
            case 'I':
            case 'i': {
                // 编辑当前单元格
                if (count < _project.notes.size()) {
                    std::string tmp = note_str(_project.notes[count], column);
                    size_t cursor = tmp.length();
                    int key;
                    // somewhat looks like a cursor
                    std::vector<Character> t =
                        ColorText(" -> ", "\x1b[47m\x1b[30m").output();
                    t.push_back(Character(0));
                    std::vector<Character> cursor_tmp =
                        text_cursor(tmp, cursor);
                    t.insert(t.cend(), cursor_tmp.cbegin(), cursor_tmp.cend());
                    ui->render_log(t);
                    ui->update();
                    while ((key = getch())) {
                        if (key == '\r') {
                            switch (column) {
                                case 0: {
                                    // NoteNum 的修改
                                    try {
                                        if (_show_actual_notenum) {
                                            size_t note = get_note_num(tmp);
                                            if (!note) throw nullptr;
                                            _project.notes[count].NoteNum =
                                                note;
                                        } else
                                            _project.notes[count].NoteNum =
                                                std::stoi(tmp);
                                    } catch (...) {
                                        t = ColorText(" -> ",
                                                      "\x1b[47m\x1b[30m")
                                                .output();
                                        t.push_back(Character(0));
                                        cursor_tmp =
                                            ColorText("Value error", "\x1b[31m")
                                                .output();
                                        t.insert(t.cend(), cursor_tmp.cbegin(),
                                                 cursor_tmp.cend());
                                        ui->render_log(t);
                                        ui->update();
                                        return false;
                                    }
                                    break;
                                }
                                case 1: {
                                    // 歌词的修改
                                    _project.notes[count].Lyric = tmp;
                                    break;
                                }
                                case 2: {
                                    // 长度的修改(Tick based)
                                    try {
                                        _project.notes[count].Length =
                                            std::stoi(tmp);
                                    } catch (...) {
                                        t = ColorText(" -> ",
                                                      "\x1b[47m\x1b[30m")
                                                .output();
                                        t.push_back(Character(0));
                                        cursor_tmp =
                                            ColorText("Value error", "\x1b[31m")
                                                .output();
                                        t.insert(t.cend(), cursor_tmp.cbegin(),
                                                 cursor_tmp.cend());
                                        ui->render_log(t);
                                        ui->update();
                                        return false;
                                    }
                                    break;
                                }
                                case 3: {
                                    // 子音速度修改
                                    try {
                                        _project.notes[count].Velocity =
                                            std::stoi(tmp);
                                    } catch (...) {
                                        t = ColorText(" -> ",
                                                      "\x1b[47m\x1b[30m")
                                                .output();
                                        t.push_back(Character(0));
                                        cursor_tmp =
                                            ColorText("Value error", "\x1b[31m")
                                                .output();
                                        t.insert(t.cend(), cursor_tmp.cbegin(),
                                                 cursor_tmp.cend());
                                        ui->render_log(t);
                                        ui->update();
                                        return false;
                                    }
                                    break;
                                }
                                case 4: {
                                    // flags 的修改
                                    _project.notes[count].Flags = tmp;
                                    break;
                                }
                            }
                            _dirty = true;
                            break;
                        } else if (key == '\x1b') {
                            break;
                        } else if (key == '\b') {
                            if (cursor > 0) {
                                tmp = tmp.substr(0, cursor - 1) +
                                      tmp.substr(cursor);
                                cursor--;
                            }
                        } else if (key == 224) {
                            switch (_getch()) {
                                case 75: {
                                    // 左键
                                    if (cursor > 0) cursor--;
                                    break;
                                }
                                case 77: {
                                    if (cursor < tmp.length()) cursor++;
                                    break;
                                }
                            }
                        } else if (key) {
                            tmp.insert(tmp.cbegin() + cursor, key);
                            cursor++;
                        }
                        t = ColorText(" -> ", "\x1b[47m\x1b[30m").output();
                        t.push_back(Character(0));
                        cursor_tmp = text_cursor(tmp, cursor);
                        t.insert(t.cend(), cursor_tmp.cbegin(),
                                 cursor_tmp.cend());
                        ui->render_log(t);
                        ui->update();
                    }
                }
                break;
            }
        }
        return true;
    }

    /**
     * @brief 获得页数。
     *
     * @param ui ui 对象。
     * @return size_t 页数。
     */
    size_t page_count(UI *ui) const noexcept {
        if (_project.notes.size() % (ui->size().y - 3) == 0 &&
            !_project.notes.empty())
            return _project.notes.size() / (ui->size().y - 3);
        else
            return _project.notes.size() / (ui->size().y - 3) + 1;
    }
    /**
     * @brief 获得路径
     *
     * @return const std::string& 路径
     */
    const std::string &path() const noexcept { return _path; }
    /**
     * @brief 渲染界面。
     *
     * @param ui UI 实例
     */
    void render(UI *ui) const {
        ui->clear();
        // Much better than before
        // 我们需要根据当前选择的位置计算出页面
        // page = (size_t)(count / (ui->size().y - 3));
        // 一定是正整数的情况下，就用 size_t 或者 unsigned int 吧。k
        ui->render_bar(_project, (size_t)(count / (ui->size().y - 3)),
                       page_count(ui), dirty());
        size_t y;
        size_t i = (size_t)(count / (ui->size().y - 3)) *
                   (ui->size().y - 3);  // 是这样写吗？?
        // 去写一下高亮看看？how
        for (y = 2; y < ui->size().y - 1 && i < _project.notes.size();
             y++, i++) {
            ui->render_note(_project.notes[i], i, y, column, _project.tempo,
                            _show_sec, _show_actual_notenum, i == count);
        }
        std::vector<Character> t =
            ColorText(" -> ", "\x1b[47m\x1b[30m").output();
        t.push_back(Character(0));
        std::vector<Character> tmp =
            count < _project.notes.size()
                ? ColorText(note_str(_project.notes[count], column), "")
                      .output()
                : ColorText("n/a", "\x1b[31m").output();
        t.insert(t.cend(), tmp.cbegin(), tmp.cend());
        ui->render_log(t);
        // done elegant complicated
    }
    /**
     * @brief 获得 dirty flag
     *
     * @return true 文件已被更改
     * @return false 文件未被更改
     */
    bool dirty() const noexcept { return _dirty; }
    const Project &project() const noexcept { return _project; }
    Editor()
        : count(0),
          column(0),
          _show_sec(true),
          _show_actual_notenum(true),
          _dirty(false) {}

   private:
    Project _project;
    std::string _path;
    bool _show_sec;
    bool _show_actual_notenum;
    bool _edit_column;
    bool _dirty;
    /**
     * @brief 辅助函数——获得单元格内容
     *
     * @param note note
     * @param column 列
     * @return std::string 单元格的实际内容
     */
    std::string note_str(const Note &note, size_t column) const noexcept {
        switch (column) {
            case 0: {
                if (_show_actual_notenum) return get_key_name(note.NoteNum);
                return std::to_string(note.NoteNum);
            }
            case 1: {
                return note.Lyric;
            }
            case 2: {
                // if (_show_sec)
                //     return std::to_string(
                //                (note.Length / 480.0 / project.tempo * 60.0))
                //                .substr(0, 5) +
                //            "s";
                return std::to_string(note.Length);
            }
            case 3: {
                return std::to_string(note.Velocity);
            }
            case 4: {
                return note.Flags;
            }
        }
        return "??";
    }
    /**
     * @brief 切换 show_sec flag。
     */
    void toggle_show_sec() noexcept { _show_sec = !_show_sec; }

    /**
     * @brief 切换 _show_actual_notenum
      flag。
     */
    void toggle_show_actual_notenum() noexcept {
        _show_actual_notenum = !_show_actual_notenum;
    }

    void remove_note() {
        if (!_project.notes.empty()) {
            _dirty = true;
            _project.notes.erase(_project.notes.cbegin() + count);
        }
        if (count > 0) count -= 1;
    }
    void insert_note_before() {
        _dirty = true;
        _project.notes.insert(_project.notes.cbegin() + count, Note());
        if (count < _project.notes.size() - 1) {
            count++;
        }
    }
    void insert_note_after() {
        _dirty = true;
        if (_project.notes.empty())
            _project.notes.insert(_project.notes.cbegin() + count, Note());
        else
            _project.notes.insert(_project.notes.cbegin() + count + 1, Note());
        if (count < _project.notes.size() - 1) {
            count++;
        }
    }
} Editor;

#endif  //_EDITOR_H
