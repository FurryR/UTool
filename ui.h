#ifndef _UI_H_
#define _UI_H_

#include <algorithm>
#include <cmath>
#include <string>

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
           std::to_string((size_t((note_num - 24) / 12.0) + 1));
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
/**
 * @brief 通用渲染文字。
 *
 * @param screen Screen 对象。
 * @param x x坐标。渲染结束后会移动到文字尾。
 * @param y y坐标。
 * @param text 文本。
 */
void render_text(Screen *screen, size_t *x, size_t y,
                 const std::vector<Character> &text) {
    for (auto &&s : text) {
        screen->set(Coord(*x, y), s);
        (*x)++;
    }
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
        render_text(&x, 0,
                    ColorText("upet-tui", "\x1b[38;2;114;159;207m").output());
        x += 1;
        render_text(
            &x, 0,
            ColorText(dirty ? project.project_name + "*" : project.project_name,
                      "")
                .output());
        x += 2;
        render_text(&x, 0,
                    ColorText("Tempo:", "\x1b[38;2;114;159;207m").output());
        x += 1;
        render_text(&x, 0,
                    ColorText(std::to_string(project.tempo), "").output());
        x += 2;
        render_text(
            &x, 0,
            ColorText("Global Flags:", "\x1b[38;2;114;159;207m").output());
        x += 1;
        render_text(&x, 0, ColorText(project.global_flags, "").output());
        x += 2;
        render_text(&x, 0,
                    ColorText("Notes:", "\x1b[38;2;114;159;207m").output());
        x += 1;
        render_text(
            &x, 0,
            ColorText(std::to_string(project.notes.size()), "").output());
        x = 0;
        // Line 2
        render_text(&x, 1, ColorText("Note", "\x1b[30;47m").output());
        x += (16 - 4) + 1;
        render_text(&x, 1, ColorText("NoteNum", "\x1b[30;47m").output());
        x += (16 - 7) + 1;
        render_text(&x, 1, ColorText("Lyric", "\x1b[30;47m").output());
        x += (16 - 5) + 1;
        render_text(&x, 1, ColorText("Length", "\x1b[30;47m").output());
        x += (16 - 6) + 1;
        render_text(&x, 1, ColorText("Velocity", "\x1b[30;47m").output());
        x += (16 - 8) + 1;
        render_text(&x, 1, ColorText("Flags", "\x1b[30;47m").output());
        x += 1;
        render_text(&x, 1, ColorText("Page:", "").output());
        x += 1;
        render_text(
            &x, 1,
            ColorText(std::to_string(page + 1) + "/" + std::to_string(total),
                      "")
                .output());

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
        render_text(&x, y, ColorText(temp, lyric_base).output());
        x += (16 - temp.length()) + 1;  // 自适应，对照上方
        // 音高
        temp = show_act ? (get_key_name(note.NoteNum) + " (" +
                           std::to_string(note.NoteNum) + ")")
                        : get_key_name(note.NoteNum);
        render_text(
            &x, y,
            ColorText(temp, (column == 0) ? hi_base : lyric_base).output());
        x += (16 - temp.length()) + 1;
        // 歌词(可能为空)
        temp = note.Lyric.empty() ? "  " : note.Lyric;
        render_text(
            &x, y,
            ColorText(temp.length() >= 16 ? (temp.substr(0, 13) + "...") : temp,
                      lyric_base +
                          (temp.length() >= 16 ? "\x1b[38;2;252;233;79m" : "") +
                          ((column == 1) ? hi_base : lyric_base))
                .output());
        x += (16 - (temp.length() >= 16 ? 16 : temp.length())) + 1;
        // 长度
        temp = std::to_string(note.Length) +
               (show_sec ? (" (" +
                            std::to_string((note.Length / 480.0 / tempo * 60.0))
                                .substr(0, 5) +
                            "s)")
                         : "");
        render_text(
            &x, y,
            ColorText(temp, (column == 2) ? hi_base : lyric_base).output());
        x += (16 - temp.length()) + 1;
        // ???
        temp = std::to_string(note.Velocity);
        render_text(
            &x, y,
            ColorText(temp, (column == 3) ? hi_base : lyric_base).output());
        x += (16 - temp.length()) + 1;
        // flags(可能为空)
        temp = note.Flags.empty() ? "  " : note.Flags;
        render_text(
            &x, y,
            ColorText(temp.length() >= 16 ? (temp.substr(0, 13) + "...") : temp,
                      lyric_base +
                          (temp.length() >= 16 ? "\x1b[38;2;252;233;79m" : "") +
                          ((column == 4) ? hi_base : lyric_base))
                .output());
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
        render_text(&x, screen->size().y - 1, text);
    }
    /**
     * @brief 创建子 UI。
     *
     * @tparam T 子 UI 的类型。
     * @return T 子 UI 实例。
     */
    template <typename T>
    T sub_ui() {
        return T(screen);
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
    void render_text(size_t *x, size_t y, const std::vector<Character> &text) {
        ::render_text(screen, x, y, text);
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
/**
 * @brief 通用 SelectUI（子 UI）
 */
typedef struct SelectUI {
    /**
     * @brief 显示上方的提示文本。
     *
     * @param text 文本。
     */
    void render_bar(const std::vector<Character> &text) {
        size_t x = 0;
        render_text(&x, 0, text);
    }
    /**
     * @brief 渲染选择。
     *
     * @param text 选择文本。
     * @param select 高亮。
     */
    void render_choice(const std::vector<std::string> &text, size_t select) {
        size_t x = 0;
        for (size_t i = 0; i < text.size(); i++) {
            x = 0;
            if (i == select) {
                render_text(
                    &x, i + 1,
                    ColorText(std::to_string(i + 1) + ". " + text[i], "\x1b[30;47m")
                        .output());
            } else {
                render_text(
                    &x, i + 1,
                    ColorText(std::to_string(i + 1) + ". " + text[i], "").output());
            }
        }
    }
    /**
     * @brief 渲染底部提示。
     *
     * @param text 文字。
     */
    void render_log(const std::vector<Character> &text) {
        size_t x = 0;
        for (; x < screen->size().x; x++) {
            screen->set(Coord(x, screen->size().y - 1), Character(0));
        }
        x = 0;
        render_text(&x, screen->size().y - 1, text);
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

    explicit SelectUI(Screen *screen) : screen(screen) {}

   private:
    void render_text(size_t *x, size_t y, const std::vector<Character> &text) {
        ::render_text(screen, x, y, text);
    }
    Screen *screen;
} SelectUI;
#endif