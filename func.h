#ifndef _FUNC_H_
#define _FUNC_H_
#include <sys/stat.h>

#include <regex>
#include <string>

#include "command.h"

Parser get_cmd() {
  Parser p;
  std::function<bool(std::string)> isNumber = [](const std::string &x) -> bool {
    return std::all_of(x.cbegin(), x.cend(),
                       [](const char c) -> bool { return std::isdigit(c); });
  };
  p.set(":exit", [](const std::string &, UI *ui, Editor *) -> void {
    ui->clear();
    ui->update();
    exit(0);
  });

  p.set(":save", [](const std::string &args, UI *ui, Editor *editor) -> void {
    if (args.empty()) {
      // ui calling
      ui->render_log(
          ColorText(":save [FILEPATH]. Continue: [ANY]", "\x1b[31m").output());
      ui->update();
      _getch();
      return;
    }
    std::string path = args;
    if (path[0] == '\"' && path[path.length() - 1] == '\"') {
      path = path.substr(1, path.length() - 2);
    }

    std::ofstream f;
    std::function<bool(std::string)> fileExistence =
        [](const std::string &file) -> bool {
      struct stat buffer {};
      return (stat(file.c_str(), &buffer) == 0);
    };

    if (fileExistence(path)) {
      ui->render_log(
          ColorText("File exists. Replace? Yes: [Y] No: [N]", "\x1b[32;40m")
              .output());
      ui->update();
      switch (_getch()) {
        case 'y':
        case 'Y': {
          f.open(path);
          break;
        }
        case 'n':
        case 'N': {
          return;
        }
      }
    } else {
      f.open(path);
    }

    if (!f.is_open()) {
      ui->render_log(
          ColorText("Error while opening file. Continue: [ANY]", "\x1b[31m")
              .output());
      ui->update();
      _getch();
      return;
    }

    f << editor->project.to_string();
    f.flush();
    f.close();

    ui->render_log(
        ColorText("File saved to " + path + ". Continue: [ANY]", "\x1b[32;40m")
            .output());
    ui->update();
    _getch();
  });
  // 你为什么不直接 std::stoi？
  p.set(
      ":play",
      [isNumber](const std::string &args, UI *ui, Editor *editor) -> void {
        size_t end = 0;
        try {
          end = std::stoi(args);
        } catch (...) {
          // 不是 number 的情况
          end = editor->project.notes.size();
        }
        if (isNumber(args) && !args.empty()) end = std::stoi(args);
        // 缩小 tmp 的生命周期(尽可能地)
        double total = 0;
        for (size_t tmp = editor->count; tmp < end; tmp++) {
          total += editor->project.notes[tmp].Length / 480.0 /
                   editor->project.tempo * 60.0;
        }
        for (; editor->count < end; editor->count++) {
          editor->render(ui);
          ui->render_log(ColorText("Playing " +
                                       std::to_string(editor->count + 1) + "/" +
                                       std::to_string(end) + " (" +
                                       std::to_string(total) + "s)",
                                   "\x1b[32;40m")
                             .output());
          ui->update();
          std::this_thread::sleep_for(std::chrono::nanoseconds((
              size_t)((float)editor->project.notes[editor->count].Length /
                      480.0f / editor->project.tempo * 60.0f * 1000000000.0f)));
        }
        ui->render_log(
            ColorText("Playback finished. Continue: [ANY]", "\x1b[32;40m")
                .output());
        ui->update();
        _getch();
      });

  p.set(":set", [](const std::string &args, UI *ui, Editor *editor) -> void {
    std::function<bool(std::string)> isNumber =
        [](const std::string &x) -> bool {
      return std::all_of(x.cbegin(), x.cend(),
                         [](const char c) -> bool { return std::isdigit(c); });
    };

    if (args.empty())
      ui->render_log(
          ColorText(":set [VALUE]. Continue: [ANY]", "\x1b[31m").output());
    switch (editor->column) {
      case 0:
        break;
      case 1:
        if (isNumber(args))
          editor->project.notes[editor->count].NoteNum = std::stoi(args);
        break;
      case 2:
        editor->project.notes[editor->count].Lyric = args;
        break;
      case 3:
        if (isNumber(args))
          editor->project.notes[editor->count].Length = std::stoi(args);
        break;
      case 4:
        if (isNumber(args))
          editor->project.notes[editor->count].Velocity = std::stoi(args);
        break;
      case 5:
        editor->project.notes[editor->count].Flags = args;
        break;
    }
  });
  p.set(":find", [](const std::string &cmd, UI *ui, Editor *editor) -> void {
    // 教我multi params
    std::vector<std::string> args = splitBy(cmd, ' ');
    // arguments should be sent in this format:
    // [REGEX]
    // 先去洗澡再说
    // 参数个数检查 父母回来了 哦我挂着吧 摸摸？

    // 那我写replace好了 .. 我电脑没装git ;)
    // 在指定 note 上按r null 劳斯来了真的
    // #include <iostream> // 包含头文件，不用管
    // using namespace std; // 不用管  // 来力，等我穿件衣服
    // int main () {
    //   cout << "Hello World" << endl;
    //   // 语法 cout << "文字" << endl;
    //   int a = 1; // Python: a: int = 1
    //   // 省略类型：auto a = 1;
    //   cin >> a; // a = int(input())
    // }

    // 我该怎么取得project 不知道 editor 吧 project 变 public 得了
    // editor->project is a public member
    // 那我先把Project变成pub咯 √
    // 洗完澡就来 先挂着（？） [15min] // C&CPP 100% 0基础 妈呀 洗澡先() 我也
    // 对了，你让威尔老师 评价一下这个作品 我OOP本当下手 // 我OOP本当下手
    // 想要给note加match高亮，鸡巴，怎么越来越复杂了
    // 什么，同时 match 多条？y
    // null 老师怎么过来了 可以教你 想把威尔找来 知道 明天
  });
  p.set(":load", [](const std::string &args, UI *ui, Editor *editor) -> void {
    if (args.empty()) {
      // ui calling
      ui->render_log(
          ColorText("Argument error. Press any key to continue...", "\x1b[31m")
              .output());
      ui->update();
      _getch();
      return;
    }
    std::string path = args;
    if (path[0] == '\"' && path[path.length() - 1] == '\"') {
      path = path.substr(1, path.length() - 2);
    }
    std::ifstream f(path);
    if (!f) {
      ui->render_log(
          ColorText("File not found. Press any key to continue...", "\x1b[31m")
              .output());
      ui->update();
      _getch();
      return;
    }
    // 这里可以抽象吗？k
    std::string raw = std::string(std::istreambuf_iterator<char>(f),
                                  std::istreambuf_iterator<char>());
    f.close();
    Project p;
    try {
      p = parse(ini_decode(raw));
    } catch (...) {
      ui->render_log(
          ColorText("File format error. Press any key to continue...",
                    "\x1b[31m")
              .output());
      ui->update();
      _getch();
      return;
    }
    editor->load(p);
  });
  p.set_default([](const std::string &, UI *ui, Editor *) -> void {
    ui->render_log(
        ColorText("Command not found. Continue: [ANY]", "\x1b[31m").output());
    ui->update();
    _getch();
  });
  return p;
}
#endif  //_FUNC_H_
