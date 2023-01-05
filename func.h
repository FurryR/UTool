#ifndef _FUNC_H_
#define _FUNC_H_

#include <conio.h>
#include <sys/stat.h>
#include <windows.h>

#include <regex>
#include <string>

#include "awacorn/awacorn.h"
#include "awacorn/promise.h"
#include "command.h"

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
                100));  // 创建一个检测输入的定期循环事件(10ms)。
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
Parser get_cmd() {
    Parser p;
    p.set("q", [](const std::string &, UI *ui, Editor *editor) -> bool {
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
    p.set("save", [](const std::string &args, UI *ui, Editor *editor) -> bool {
        std::string path;
        if (args.empty()) {
            path = editor->path();
        }
        path = args;
        if (path[0] == '\"' && path[path.length() - 1] == '\"') {
            path = path.substr(1, path.length() - 2);
        }
        if (file_exist(path) && path != editor->path()) {
            ui->render_log(
                ColorText("File already exists. Overwrite? (y/n)", "\x1b[32m")
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
                ColorText("E: Error while opening file", "\x1b[31m").output());
            ui->update();
            return false;
        }
        ui->render_log(ColorText("File saved to " + path, "\x1b[32m").output());
        ui->update();
        return false;
    });

    p.set("play", [](const std::string &args, UI *ui, Editor *editor) -> bool {
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
        std::function<void(Awacorn::EventLoop *, const Awacorn::Event *)>
            play_fn = [ui, editor, end, &total, &next, &play_fn](
                          Awacorn::EventLoop *ev,
                          const Awacorn::Event *task) -> void {
            editor->render(ui);
            ui->render_log(
                ColorText("Playing " + std::to_string(editor->count + 1) + "/" +
                              std::to_string(end) + " (" +
                              std::to_string(total) + "s) ([q] Quit)",
                          "\x1b[32m")
                    .output());
            ui->update();
            if (editor->count < end) {
                // next = ev->create(
                //     play_fn,
                //     std::chrono::nanoseconds(
                //         (size_t)(editor->project().notes[editor->count].Length
                //         /
                //                  480.0 / editor->project().tempo * 60.0 *
                //                  1000000000)) -
                //                  std::chrono::nanoseconds(300000));
                // editor->count++;
                std::chrono::nanoseconds tmp = std::chrono::nanoseconds(
                    (size_t)(editor->project().notes[editor->count].Length /
                             480.0 / editor->project().tempo * 60.0 *
                             1000000000));
                // next = ev->create(play_fn, tmp);
                next = ev->create(play_fn,
                                  tmp < std::chrono::milliseconds(1)
                                      ? tmp
                                      : tmp - std::chrono::milliseconds(1));
                editor->count++;
            } else {
                ui->render_log(
                    ColorText("Playback finished. Press any key to continue...",
                              "\x1b[32m")
                        .output());
                ui->update();
                next = nullptr;
            }
        };
        // shared_ptr 是带引用计数的智能指针。由于多个事件需要获得
        // ev，同时还要保证 ev 的生命周期，故使用 shared_ptr。 我是傻逼，忘了
        // ev.start() 是 blocking 的了
        std::function<Promise::Promise<void>(int)> getch_fn =
            [&ev, &next, &getch_fn](int result) {
                if (result == 'q' || result == 'Q' || (!next)) {
                    if (next) ev.clear(next);
                    return Promise::resolve<void>();
                }
                return ev.run(async_getch()).then<void>(getch_fn);
            };
        next = ev.create(
            play_fn,
            std::chrono::nanoseconds(0));  // 创建第一个歌词的任务，立即执行
        ev.run(async_getch()).then<void>(getch_fn);
        timeBeginPeriod(1);
        ev.start();  // 等待播放完成或者中断
        timeEndPeriod(1);
        return true;
    });
    p.set("version", [](const std::string &, UI *ui, Editor *) -> bool {
        ui->render_log(
            ColorText("upet-tui ver1a by ookamitai & FurryR, 2023", "")
                .output());
        ui->update();
        return false;
    });
    p.set("help", [](const std::string &, UI *ui, Editor *) -> bool {
        ui->render_log(ColorText("see https://github.com/ookamitai/upet-tui "
                                 "for more information",
                                 "")
                           .output());
        ui->update();
        return false;
    });
    p.set("load", [](const std::string &args, UI *ui, Editor *editor) -> bool {
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

    p.set("close", [](const std::string &, UI *ui, Editor *editor) -> bool {
        if (editor->dirty()) {
            ui->render_log(ColorText("You have unsaved changes. Close without "
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

    p.set_default([](const std::string &, UI *ui, Editor *) -> bool {
        ui->render_log(ColorText("E: Command not found", "\x1b[31m").output());
        ui->update();
        return false;
    });
    return p;
}

#endif  //_FUNC_H_
