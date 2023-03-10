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
     * @brief ??????????????? Project ?????????
     *
     * @param p Project ??????
     */
    void load(const Project &p) {
        _action.clear();
        _project = p;
        _dirty = false;
        count = 0;
    }
    /**
     * @brief ?????? filename ????????????????????????????????????????????????
     *
     * @param filename ?????????
     * @return true ???????????????
     * @return false ???????????????
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
     * @brief ???????????????????????????
     *
     * @param filename ????????????
     * @return true ???????????????
     * @return false ???????????????
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
     * @brief ????????????????????????
     *
     * @param ui UI ?????????
     * @param input ??????????????????
     * @return bool ?????????????????????
     */
    bool process(UI *ui, int input) {
        switch (input) {
            case 224: {
                // ????????????
                // ??? ????????????????????? sure
                switch (_getch()) {
                    case 72: {
                        // ??????
                        if (count > 0) count--;
                        break;
                    }
                    case 80: {
                        // ???????????????
                        if (count < project().notes.size() - 1) count++;
                        break;
                    }
                    case 75: {
                        // ??????????????????????????????
                        if (column == 0)
                            column = 4;
                        else
                            column--;
                        break;
                    }
                    case 77: {
                        // ??????????????????????????????
                        if (column == 4)
                            column = 0;
                        else
                            column++;
                        break;
                    }
                    case 73: {
                        // PageUp???????????????
                        if (count > (ui->size().y - 3))
                            count -= ui->size().y - 3;
                        else
                            count = 0;
                        break;
                    }
                    case 81: {
                        // PageDown???????????????
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
                // ??????tick??????????????????
                // ???????????????????????????
                // ???????????????
                if (column == 0)
                    toggle_show_actual_notenum();
                else if (column == 2)
                    toggle_show_sec();

                break;
            }
            case 'N':
            case 'n': {
                // ?????????????????????
                insert_note_before();
                break;
            }
            case 'M':
            case 'm': {
                // ?????????????????????
                insert_note_after();
                break;
            }
            case '\b': {
                // ??????????????????
                remove_note();
                break;
            }
            case 'I':
            case 'i': {
                // ?????????????????????
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
                                    // NoteNum ?????????
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
                                    // ???????????????
                                    Note after = project().notes[count];
                                    after.Lyric = tmp;
                                    action(Action(ActionType::Modify,
                                                  project().notes[count], after,
                                                  count));
                                    break;
                                }
                                case 2: {
                                    // ???????????????(Tick based)
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
                                    // ??????????????????
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
                                    // flags ?????????
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
                                    // ??????
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
     * @brief ???????????????
     *
     * @param ui ui ?????????
     * @return size_t ?????????
     */
    size_t page_count(UI *ui) const noexcept {
        if (project().notes.size() % (ui->size().y - 3) == 0 &&
            !project().notes.empty())
            return project().notes.size() / (ui->size().y - 3);
        else
            return project().notes.size() / (ui->size().y - 3) + 1;
    }
    /**
     * @brief ????????????
     *
     * @return const std::string& ??????
     */
    const std::string &path() const noexcept { return _path; }
    /**
     * @brief ???????????????
     *
     * @param ui UI ??????
     */
    void render(UI *ui) const {
        ui->clear();
        // Much better than before
        // ??????????????????????????????????????????????????????
        // page = (size_t)(count / (ui->size().y - 3));
        // ??????????????????????????????????????? size_t ?????? unsigned int ??????k
        ui->render_bar(project(), (size_t)(count / (ui->size().y - 3)),
                       page_count(ui), dirty());
        size_t y;
        size_t i = (size_t)(count / (ui->size().y - 3)) *
                   (ui->size().y - 3);  // ???????????????????
        // ???????????????????????????how
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
     * @brief ?????? dirty flag
     *
     * @return true ??????????????????
     * @return false ??????????????????
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
     * @brief ???????????????????????????????????????
     *
     * @param note note
     * @param column ???
     * @return std::string ????????????????????????
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
     * @brief ?????? show_sec flag???
     */
    void toggle_show_sec() noexcept { _show_sec = !_show_sec; }

    /**
     * @brief ?????? _show_actual_notenum
      flag???
     */
    void toggle_show_actual_notenum() noexcept {
        _show_actual_notenum = !_show_actual_notenum;
    }
} Editor;

#endif  //_EDITOR_H
