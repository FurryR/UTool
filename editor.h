#ifndef _EDITOR_H_
#define _EDITOR_H_
#include <conio.h>

#include <fstream>
#include <utility>

#include "ui.h"

typedef enum class ActionType {
    InsertBefore = 0,
    InsertAfter = 1,
    Remove = 2,
    Modify = 3,
} ActionType;
// std::string actiontype2str(const ActionType &action) {
//     switch (action) {
//         case ActionType::InsertBefore: {
//             return "insert before";
//         }
//         case ActionType::InsertAfter: {
//             return "insert after";
//         }
//         case ActionType::Remove: {
//             return "remove";
//         }
//         case ActionType::Modify: {
//             return "modify";
//         }
//     }
//     return "unknown";
// }
typedef struct Action {
    Action() = default;
    Action(const ActionType &action, const Note &before, const Note &after,
           size_t index)
        : before(before), after(after), index(index), action(action) {}
    void undo(Project *proj, size_t *count) const {
        switch (action) {
            case ActionType::InsertBefore: {
                proj->notes.erase(proj->notes.cbegin() + index);
                if (*count > 0) *count -= 1;
                break;
            }
            case ActionType::InsertAfter: {
                proj->notes.erase(proj->notes.cbegin() + index + 1);
                if (*count > 0) *count -= 1;
                break;
            }
            case ActionType::Remove: {
                proj->notes.insert(proj->notes.cbegin() + index, before);
                if (*count < proj->notes.size() - 1) {
                    *count++;
                }
                break;
            }
            case ActionType::Modify: {
                proj->notes[index] = before;
                break;
            }
        }
    }
    void redo(Project *proj, size_t *count) const {
        switch (action) {
            case ActionType::InsertBefore: {
                proj->notes.insert(proj->notes.cbegin() + index, Note());
                if (*count < proj->notes.size() - 1) {
                    count++;
                }
                break;
            }
            case ActionType::InsertAfter: {
                if (proj->notes.empty())
                    proj->notes.insert(proj->notes.cbegin() + index, Note());
                else
                    proj->notes.insert(proj->notes.cbegin() + index + 1,
                                       Note());
                if (*count < proj->notes.size() - 1) {
                    count++;
                }
                break;
            }
            case ActionType::Remove: {
                if (!proj->notes.empty()) {
                    proj->notes.erase(proj->notes.cbegin() + index);
                }
                if (*count > 0) *count -= 1;
                break;
            }
            case ActionType::Modify: {
                proj->notes[index] = after;
                break;
            }
        }
    }

   private:
    Note before;
    Note after;
    size_t index;
    ActionType action;
} Action;
typedef struct ActionManager {
    ActionManager() : _action(), _current(0) {}
    bool undo(Project *proj, size_t *count) {
        if (_action.empty() || _current == 0) return false;
        _action[_current - 1].undo(proj, count);
        _current--;
        return true;
    }
    bool redo(Project *proj, size_t *count) {
        if (_action.empty() || _current == _action.size()) return false;
        _action[_current].redo(proj, count);
        _current++;
        return true;
    }
    void action(Project *proj, const Action &act, size_t *count) {
        if (_current != 0 && _current != _action.size()) {
            while (_action.cbegin() + _current - 1 != _action.cend()) {
                _action.erase(_action.cbegin() + _current - 1);
            }
        }
        act.redo(proj, count);
        _action.push_back(act);
        _current = _action.size();
    }
    void clear() {
        _action.clear();
        _current = 0;
    }

   private:
    std::vector<Action> _action;
    size_t _current;
} ActionManager;
typedef struct Editor {
    size_t count;
    size_t column;

    /**
     * @brief 加载指定的 Project 实例。
     *
     * @param p Project 实例
     */
    void load(const Project &p) {
        _action.clear();
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
    bool undo() { return _action.undo(&_project, &count); }
    bool redo() { return _action.redo(&_project, &count); }
    void action(const Action &act) { _action.action(&_project, act, &count); }
    void action_norec(const Action &act) {
        _action.clear();
        act.redo(&_project, &count);
    }
    void remove_note() {
        if (!project().notes.empty()) {
            _dirty = true;
            action(Action(ActionType::Remove, project().notes[count], Note(),
                          count));
        }
    }
    void insert_note_before() {
        _dirty = true;
        action(Action(ActionType::InsertBefore, Note(), Note(), count));
    }
    void insert_note_after() {
        _dirty = true;
        action(Action(ActionType::InsertAfter, Note(), Note(), count));
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
                if (count < project().notes.size()) {
                    std::string tmp = note_str(project().notes[count], column);
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
                                        Note after = project().notes[count];
                                        if (_show_actual_notenum) {
                                            size_t note = get_note_num(tmp);
                                            if (!note) throw nullptr;
                                            after.NoteNum = note;
                                        } else
                                            after.NoteNum = std::stoi(tmp);
                                        action(Action(ActionType::Modify,
                                                      project().notes[count],
                                                      after, count));
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
                                    Note after = project().notes[count];
                                    after.Lyric = tmp;
                                    action(Action(ActionType::Modify,
                                                  project().notes[count], after,
                                                  count));
                                    break;
                                }
                                case 2: {
                                    // 长度的修改(Tick based)
                                    try {
                                        Note after = project().notes[count];
                                        after.Length = std::stoi(tmp);
                                        action(Action(ActionType::Modify,
                                                      project().notes[count],
                                                      after, count));
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
                                        Note after = project().notes[count];
                                        after.Velocity = std::stoi(tmp);
                                        action(Action(ActionType::Modify,
                                                      project().notes[count],
                                                      after, count));
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
                                    Note after = project().notes[count];
                                    after.Flags = tmp;
                                    action(Action(ActionType::Modify,
                                                  project().notes[count], after,
                                                  count));
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
        if (project().notes.size() % (ui->size().y - 3) == 0 &&
            !project().notes.empty())
            return project().notes.size() / (ui->size().y - 3);
        else
            return project().notes.size() / (ui->size().y - 3) + 1;
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
        ui->render_bar(project(), (size_t)(count / (ui->size().y - 3)),
                       page_count(ui), dirty());
        size_t y;
        size_t i = (size_t)(count / (ui->size().y - 3)) *
                   (ui->size().y - 3);  // 是这样写吗？?
        // 去写一下高亮看看？how
        for (y = 2; y < ui->size().y - 1 && i < project().notes.size();
             y++, i++) {
            ui->render_note(project().notes[i], i, y, column, project().tempo,
                            _show_sec, _show_actual_notenum, i == count);
        }
        std::vector<Character> t =
            ColorText(" -> ", "\x1b[47m\x1b[30m").output();
        t.push_back(Character(0));
        std::vector<Character> tmp =
            count < project().notes.size()
                ? ColorText(note_str(project().notes[count], column), "")
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
    ActionManager _action;
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
} Editor;

#endif  //_EDITOR_H
