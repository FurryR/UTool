#ifndef _FUNC_H_
#define _FUNC_H_

#include <conio.h>
#include <sys/stat.h>
#include <windows.h>

#include <regex>
#include <string>

#include "audio.h"
#include "awacorn/awacorn.h"
#include "awacorn/promise.h"
#include "command.h"
#include "selector.h"

/**
 * @brief 异步的 getch。
 *
 * @return AsyncFn<Promise::Promise<int>> 异步的 getch 结果
 */
Awacorn::AsyncFn<Promise::Promise<int>> async_getch() {
    // 返回一个 async 包装，然后我们可以使用 ev->run(async_getch())
    // 这样的语法，非常方便。
    return [](Awacorn::EventLoop *ev) {
        Promise::Promise<int> pm;  // 创建一个 Promise(承诺，回调式)
        ev->create(
            [pm](Awacorn::EventLoop *ev,
                 const Awacorn::Interval *task) -> void {
                if (_kbhit()) {  // 如果检测到输入
                    pm.resolve(
                        _getch());  // 解决这个 Promise（我们可以用 Promise.then
                                    // 来实现回调注册（链式调用））
                    ev->clear(task);  // 清除这个 task
                }
            },
            std::chrono::milliseconds(
                100));  // 创建一个检测输入的定期循环事件(100ms)。
        // 定期检测大概间隔多少？越大的间隔会导致输入延迟越严重，但占用率越低；越小的间隔会让输入延迟更低，但占用率更高。
        return pm;  // this is awacorn rather strange attempt to understand
                    // 打算以后去找个电子厂混混日子
                    // 10ms 怎么样？还是需要更低？ok的
        // 这个蛮简单的，想学也可以学会！(我可以教你)*objection.wav*
    };
}
bool file_exist(const std::string &file) {
    struct stat buffer {};
    return (stat(file.c_str(), &buffer) ==
            0);  // I'm sorry for this strange solution
}
std::map<std::string, Parser> plugin;
Parser get_cmd() {
    Parser p;

    p.set(
        "q", [](const std::string &, Parser *, UI *ui, Editor *editor) -> bool {
            if (editor->dirty()) {
                ui->render_log(
                    ColorText(
                        "You have unsaved changes. Quit without saving? (y/n)",
                        "\x1b[32m")
                        .output());
                ui->update();
                switch (_getch()) {
                    case 'Y':
                    case 'y': {
                        break;
                    }
                    default: {
                        return true;
                    }
                }
            }
            ui->clear();
            ui->update();
            exit(0);
        });
    p.set(
        "save",
        [](const std::string &args, Parser *, UI *ui, Editor *editor) -> bool {
            std::string path;
            if (args.empty()) {
                path = editor->path();
            } else
                path = args;
            if (path[0] == '\"' && path[path.length() - 1] == '\"') {
                path = path.substr(1, path.length() - 2);
            }
            if (path.empty()) {
                ui->render_log(
                    ColorText("E: No file name", "\x1b[31m").output());
                ui->update();
                return false;
            }
            if (file_exist(path) && path != editor->path()) {
                ui->render_log(
                    ColorText("File already exists. Overwrite? (y/n)",
                              "\x1b[32m")
                        .output());
                ui->update();
                switch (_getch()) {
                    case 'y':
                    case 'Y': {
                        break;
                    }
                    default: {
                        return true;
                    }
                }
            }
            if (!editor->save(path)) {
                ui->render_log(
                    ColorText("E: Error while opening file", "\x1b[31m")
                        .output());
                ui->update();
                return false;
            }
            ui->render_log(
                ColorText("File saved to " + path, "\x1b[32m").output());
            ui->update();
            return false;
        });

    p.set(
        "play",
        [](const std::string &args, Parser *, UI *ui, Editor *editor) -> bool {
            size_t end;
            try {
                end = std::stoi(args);
            } catch (...) {
                // 不是 number 的情况
                end = editor->project().notes.size();
            }
            // 缩小 tmp 的生命周期(尽可能地)
            if (end >= editor->project().notes.size())
                end = editor->project().notes.size();

            double total = 0;
            for (size_t tmp = editor->count; tmp < end; tmp++) {
                total += editor->project().notes[tmp].Length / 480.0 /
                         editor->project().tempo * 60.0;
            }
            Awacorn::EventLoop ev;  // 创建一个 EventLoop
            const Awacorn::Event *next = nullptr;
            Promise::Promise<void> next_pm;
            std::function<void()> play_fn = [ui, editor, end, &total, &next,
                                             &play_fn, &next_pm,
                                             &ev]() -> void {
                editor->render(ui);
                ui->render_log(
                    ColorText("Playing " + std::to_string(editor->count + 1) +
                                  "/" + std::to_string(end) + " (" +
                                  std::to_string(total) + "s) ([esc] Quit)",
                              "\x1b[32m")
                        .output());
                ui->update();
                if (editor->count < end) {
                    std::chrono::nanoseconds tmp = std::chrono::nanoseconds(
                        (size_t)(editor->project().notes[editor->count].Length /
                                 480.0 / editor->project().tempo * 60.0 *
                                 1000000000));
                    std::pair<const Awacorn::Event *, Promise::Promise<void>>
                        ret = ev.run(Audio::play_note(
                            editor->project().notes[editor->count].Lyric,
                            editor->project().notes[editor->count].NoteNum,
                            tmp < std::chrono::milliseconds(1)
                                ? tmp
                                : tmp - std::chrono::milliseconds(1)));
                    next = ret.first;
                    next_pm = ret.second.then<void>(play_fn);
                    editor->count++;
                } else {
                    if (editor->count > 0) editor->count--;
                    ui->render_log(
                        ColorText(
                            "Playback finished. Press any key to continue...",
                            "\x1b[32m")
                            .output());
                    ui->update();
                    next = nullptr;
                }
            };
            std::function<Promise::Promise<void>(int)> getch_fn =
                [&ev, &next, &getch_fn, &next_pm, &editor](int result) {
                    if (result == '\x1b' || (!next)) {
                        Audio::panic();
                        if (next) ev.clear(next);
                        next_pm.reject(std::exception());
                        return Promise::resolve<void>();
                    }
                    return ev.run(async_getch()).then<void>(getch_fn);
                };
            next = ev.create(
                [play_fn](Awacorn::EventLoop *,
                          const Awacorn::Event *) -> void { play_fn(); },
                std::chrono::nanoseconds(0));  // 创建第一个歌词的任务，立即执行
            ev.run(async_getch()).then<void>(getch_fn);
            Audio::init();
            timeBeginPeriod(1);
            ev.start();  // 等待播放完成或者中断
            timeEndPeriod(1);
            Audio::close();
            return true;
        });
    p.set("version",
          [](const std::string &, Parser *, UI *ui, Editor *) -> bool {
              ui->render_log(
                  ColorText("upet-tui ver1a by ookamitai & FurryR, 2023", "")
                      .output());
              ui->update();
              return false;
          });
    p.set("help", [](const std::string &, Parser *, UI *ui, Editor *) -> bool {
        ui->render_log(ColorText("See https://github.com/ookamitai/upet-tui "
                                 "for more information",
                                 "")
                           .output());
        ui->update();
        return false;
    });
    p.set(
        "load",
        [](const std::string &args, Parser *, UI *ui, Editor *editor) -> bool {
            if (args.empty()) {
                // ui calling
                ui->render_log(
                    ColorText("Usage: load [FILE]", "\x1b[31m").output());
                ui->update();
                return false;
            }
            std::string path = args;
            if (path[0] == '\"' && path[path.length() - 1] == '\"') {
                path = path.substr(1, path.length() - 2);
            }
            if (editor->dirty()) {
                ui->render_log(
                    ColorText(
                        "You have unsaved changes. Close without saving? (y/n)",
                        "\x1b[32m")
                        .output());
                ui->update();
                switch (_getch()) {
                    case 'Y':
                    case 'y': {
                        break;
                    }
                    default: {
                        return true;
                    }
                }
            }
            try {
                if (!editor->open(path)) {
                    ui->render_log(
                        ColorText("E: File not found", "\x1b[31m").output());
                    ui->update();
                    return false;
                }
            } catch (...) {
                ui->render_log(
                    ColorText("E: File format error", "\x1b[31m").output());
                ui->update();
                return false;
            }
            return true;
        });

    p.set("close",
          [](const std::string &, Parser *, UI *ui, Editor *editor) -> bool {
              if (editor->dirty()) {
                  ui->render_log(
                      ColorText("You have unsaved changes. Close without "
                                "saving? (y/n)",
                                "\x1b[32m")
                          .output());
                  ui->update();
              }
              switch (_getch()) {
                  case 'y':
                  case 'Y': {
                      editor->load(Project());
                      break;
                  }
              }
              return true;
          });
    p.set("undo",
          [](const std::string &, Parser *, UI *ui, Editor *editor) -> bool {
              if (!editor->undo()) {
                  ui->render_log(
                      ColorText("E: Undo failed", "\x1b[31m").output());
                  ui->update();
                  return false;
              } else {
                  return true;
              }
          });
    p.set("redo",
          [](const std::string &, Parser *, UI *ui, Editor *editor) -> bool {
              if (!editor->redo()) {
                  ui->render_log(
                      ColorText("E: Redo failed", "\x1b[31m").output());
                  ui->update();
                  return false;
              } else {
                  return true;
              }
          });
    p.set(
        "plugin",
        [](const std::string &, Parser *parser, UI *baseui,
           Editor *editor) -> bool {
            SelectUI subui = baseui->sub_ui<SelectUI>();
            bool refresh = true;
            std::vector<std::string> tmplist;
            tmplist.reserve(plugin.size());
            for (auto &&i : plugin) {
                tmplist.push_back(i.first);
            }
            Selector selector("upet-tui plugin manager Plugins: " +
                                  std::to_string(plugin.size()),
                              tmplist,
                              "[enter] Load [backspace] Unload [esc] Quit");
            while (1) {
                if (refresh)
                    selector.render(&subui);
                else
                    refresh = true;
                subui.update();
                int g = _getch();
                switch (g) {
                    case '\r': {
                        // 添加
                        std::string tmp = "";
                        size_t cursor = tmp.length();
                        int key;
                        std::vector<Character> t =
                            ColorText(" Path ", "\x1b[47m\x1b[30m").output();
                        t.push_back(Character(0));
                        std::vector<Character> cursor_tmp =
                            text_cursor(tmp, cursor);
                        t.insert(t.cend(), cursor_tmp.cbegin(),
                                 cursor_tmp.cend());
                        subui.render_log(t);
                        subui.update();
                        while ((key = getch())) {
                            if (key == '\r') {
                                if (tmp.empty()) break;
                                if (tmp[0] == '\"' &&
                                    tmp[tmp.length() - 1] == '\"') {
                                    tmp = tmp.substr(1, tmp.length() - 2);
                                }
                                HMODULE module = LoadLibraryA(tmp.c_str());
                                if (!module) {
                                    subui.render_log(
                                        ColorText("E: Error while opening file",
                                                  "\x1b[31m")
                                            .output());
                                    subui.update();
                                    refresh = false;
                                    break;
                                }
                                FARPROC m = GetProcAddress(module, "_export");
                                if (!m) {
                                    subui.render_log(
                                        ColorText("E: Module didn't register "
                                                  "itself (_export function)",
                                                  "\x1b[31m")
                                            .output());
                                    subui.update();
                                    refresh = false;
                                    break;
                                }
                                std::pair<std::string, Parser> result =
                                    ((std::pair<std::string, Parser>(*)())m)();
                                plugin.insert(std::make_pair(result.first,
                                                             result.second));
                                std::string t = std::move(result.first);
                                parser->set(
                                    t,
                                    [t](const std::string &arg, Parser *parser,
                                        UI *ui, Editor *editor) -> bool {
                                        return plugin.at(t).execute(arg, ui,
                                                                    editor);
                                    });
                                selector.choice.push_back(t);
                                selector.render(&subui);
                                subui.render_log(
                                    ColorText("Loaded module '" + t + "'", "")
                                        .output());
                                subui.update();
                                refresh = false;
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
                            t = ColorText(" Path ", "\x1b[47m\x1b[30m")
                                    .output();
                            t.push_back(Character(0));
                            cursor_tmp = text_cursor(tmp, cursor);
                            t.insert(t.cend(), cursor_tmp.cbegin(),
                                     cursor_tmp.cend());
                            subui.render_log(t);
                            subui.update();
                        }
                        break;
                    }
                    case '\b': {
                        // 删除
                        if (selector.choice.empty()) break;
                        parser->remove(selector.choice[selector.current]);
                        plugin.erase(selector.choice[selector.current]);
                        std::string t =
                            std::move(selector.choice[selector.current]);
                        selector.choice.erase(selector.choice.cbegin() +
                                              selector.current);
                        if (selector.current > 0) selector.current--;
                        selector.render(&subui);
                        subui.render_log(
                            ColorText("Unloaded module '" + t + "'", "")
                                .output());

                        subui.update();
                        refresh = false;
                        break;
                    }
                    case '\x1b': {
                        // 退出
                        return true;
                    }
                    default: {
                        selector.process(g);
                        break;
                    }
                }
            }
        });
    p.set_default([](const std::string &, Parser *, UI *ui, Editor *) -> bool {
        ui->render_log(ColorText("E: Command not found", "\x1b[31m").output());
        ui->update();
        return false;
    });
    return p;
}

#endif  //_FUNC_H_
