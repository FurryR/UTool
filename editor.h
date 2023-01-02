#ifndef _EDITOR_H
#define _EDITOR_H

#include <conio.h>

#include <fstream>
#include <iomanip>
#include <regex>
#include <thread>
#include <utility>

#include "logger.h"
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
  constexpr std::array<const char *, 12> ref{"C",  "C#", "D",  "D#", "E",  "F",
                                             "F#", "G",  "G#", "A",  "A#", "B"};
  if (note_num < 24 || note_num > 108) return "??";
  return ref[(note_num - 24) % 12] +
         std::to_string((int)((note_num - 24) / 12 + 1));
  // 鸡巴忘了
  //  *要考试了。
}
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
  ColorText(std::string content, std::string prefix)
      : prefix(std::move(prefix)), content(std::move(content)) {}
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
  for (char ch; (ch = (char)_getch()) != ';'; ret.y = ret.y * 10 + (ch - '0'))
    ;
  for (char ch; (ch = (char)_getch()) != 'R'; ret.x = ret.x * 10 + (ch - '0'))
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
   */
  void render_bar(const Project &project, size_t page, size_t total) {
    // 进行一些邪术的施展
    // 当前绘制的位置
    size_t x = 0;  // currentX
    render_text(&x, 0, ColorText("upet-ng", "\x1b[30;47m"));
    x += 1;
    render_text(
        &x, 0,
        ColorText(project.project_name.empty() ? "None" : project.project_name,
                  ""));
    x += 2;
    render_text(&x, 0, ColorText("Tempo:", "\x1b[30;47m"));
    x += 1;
    render_text(&x, 0, ColorText(std::to_string(project.tempo), ""));
    x += 2;
    render_text(&x, 0, ColorText("Global Flags:", "\x1b[30;47m"));
    x += 1;
    render_text(&x, 0, ColorText(project.global_flags, ""));
    // 第二行
    x = 0;
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
    x += 2;
    render_text(&x, 1, ColorText("Page:", ""));
    x += 1;
    render_text(
        &x, 1,
        ColorText(std::to_string(page + 1) + "/" + std::to_string(total), ""));

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
                   double tempo, bool show_sec, bool show_act, bool highlight) {
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

    temp = std::to_string(count + 1);
    render_text(&x, y, ColorText(temp, (column == 0) ? hi_base : lyric_base));
    x += (16 - temp.length()) + 1;  // 自适应，对照上方
    temp = show_act ? (get_key_name(note.NoteNum) + " (" +
                       std::to_string(note.NoteNum) + ")")
                    : get_key_name(note.NoteNum);
    render_text(&x, y, ColorText(temp, (column == 1) ? hi_base : lyric_base));
    x += (16 - temp.length()) + 1;
    render_text(
        &x, y,
        ColorText(note.Lyric.length() >= 16 ? (note.Lyric.substr(0, 13) + "...")
                                            : note.Lyric,
                  lyric_base + (note.Lyric.length() >= 16 ? "\x1b[33m" : "") +
                      ((column == 2) ? hi_base : lyric_base)));
    x += (16 - (note.Lyric.length() >= 16 ? 16 : note.Lyric.length())) + 1;
    temp = std::to_string(note.Length) +
           (show_sec ? (" (" +
                        std::to_string((note.Length / 480.0 / tempo * 60.0))
                            .substr(0, 5) +
                        "s)")
                     : "");
    render_text(&x, y, ColorText(temp, (column == 3) ? hi_base : lyric_base));
    x += (16 - temp.length()) + 1;
    temp = std::to_string(note.Velocity);
    render_text(&x, y, ColorText(temp, (column == 4) ? hi_base : lyric_base));
    x += (16 - temp.length()) + 1;
    render_text(
        &x, y,
        ColorText(note.Flags.length() >= 16 ? (note.Flags.substr(0, 13) + "...")
                                            : note.Flags,
                  lyric_base + (note.Flags.length() >= 16 ? "\x1b[33m" : "") +
                      ((column == 5) ? hi_base : lyric_base)));
    // x += (16 - (note.Flags.length() >= 16 ? 16 : note.Flags.length())) +
    // 1;

    // std::cout
    //     << (pos + page * (size().y - 3) + 1) << "\t"
    //     << p.notes[pos + page * (size().y - 3)].NoteNum << "\t"
    //     << ((p.notes[pos + page * (size().y - 3)].Lyric.length() >= 15)
    //             ? "\x1b[33m" +
    //                   p.notes[pos + page * (size().y - 3)].Lyric.substr(
    //                       0, 12) +
    //                   "..." + "\x1b[0m"
    //             : p.notes[pos + page * (size().y - 3)].Lyric)
    //     << ((p.notes[pos + page * (size().y - 3)].Lyric.length() >= 15)
    //             ? "\t"
    //             : "\t\t")
    //     << p.notes[pos + page * (size().y - 3)].Length << "\t\t"
    //     << p.notes[pos + page * (size().y - 3)].Velocity << "\t\t"
    //     << ((p.notes[pos + page * (size().y - 3)].Flags.length() >= 20)
    //             ? "\x1b[33m" +
    //                   p.notes[pos + page * (size().y - 3)].Flags.substr(
    //                       0, 17) +
    //                   "..." + "\x1b[0m"
    //             : p.notes[pos + page * (size().y - 3)].Flags)
    //     << std::endl;
    // std::cout << "\x1b[0m";
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

typedef struct Editor {
  Project project;
  size_t count;
  size_t column;
  /**
   * @brief 加载指定的 Project 实例。
   *
   * @param p Project 实例
   */
  void load(const Project &p) {
    project = p;
    count = 0;
  }
  /**
   * @brief 输入的处理事件。
   *
   * @param ui UI 实例。
   * @param input 输入的字符。
   */
  void process(UI *ui, char input) {
    switch (input) {
      case 'W':
      case 'w': {
        // 上一个音符
        if (count > 0) count--;
        break;
      }
      case 'S':
      case 's': {
        // 下一个音符
        if (count < project.notes.size() - 1) count++;
        break;
      }
      case 'A':
      case 'a': {
        // 上一页
        if (count > (ui->size().y - 3))
          count -= ui->size().y - 3;
        else
          count = 0;
        break;
      }
      case 'D':
      case 'd': {
        // 下一页
        if (count + (ui->size().y - 3) < project.notes.size() - 1) {
          count += ui->size().y - 3;
        } else {
          count = project.notes.size() - 1;
        }
        break;
      }
      case 'T':
      case 't': {
        toggle_show_sec();
        break;
      }
      case 'Q':
      case 'q': {
        toggle_show_actual_notenum();
        break;
      }
      case '\t': {
        // 切换 column
        column++;
        if (column == 6) column = 0;
        break;
      }

    }
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


  /**
   * @brief 获得页数。
   *
   * @param ui ui 对象。
   * @return size_t 页数。
   */
  size_t page_count(UI *ui) const noexcept {
    return project.notes.size() / (ui->size().y - 3) + 1;
  }
  /**
   * @brief 渲染界面。
   *
   * @param ui UI 实例
   */
  void render(UI *ui) {
    ui->clear();
    // Much better than before
    // 我们需要根据当前选择的位置计算出页面
    // page = (size_t)(count / (ui->size().y - 3));
    // 一定是正整数的情况下，就用 size_t 或者 unsigned int 吧。k
    ui->render_bar(project, (size_t)(count / (ui->size().y - 3)),
                   page_count(ui));
    size_t y;
    size_t i = (size_t)(count / (ui->size().y - 3)) *
               (ui->size().y - 3);  // 是这样写吗？?
    // 去写一下高亮看看？how
    for (y = 2; y < ui->size().y - 1 && i < project.notes.size(); y++, i++) {
      ui->render_note(project.notes[i], i, y, column, project.tempo, _show_sec,
                      _show_actual_notenum, i == count);
    }

    // done elegant
  }
  Editor() : count(0), column(0), _show_sec(true), _show_actual_notenum(true) {}

 private:
  bool _show_sec;
  bool _show_actual_notenum;
} Editor;

#endif  //_EDITOR_H
