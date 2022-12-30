#ifndef _FUNC_H_
#define _FUNC_H_
#include "command.h"
#include <regex>
#include <string>

// execute 备份
// 说好的写物理呢 不会写
/*
int [[deprecated]] execute(const std::string &command) {
    if (command.empty())
        return 0;
    auto cm = splitBy(command, '>');

    if (cm[0] == "next" || cm[0] == "n") {
        if (page < page_count)
            page += 1;
        reload();
        return 0;
    }

    if (cm[0] == "lastpage" || cm[0] == "lp") {
        page = page_count;
        return 0;
    }

    if (cm[0] == "firstpage" || cm[0] == "fp") {
        page = 0;
        return 0;
    }

    if (cm[0] == "prev" || cm[0] == "p") {
        if (page > 0)
            page -= 1;
        reload();
        return 0;
    }

    if (cm[0] == "load" || cm[0] == "l") {
        if (cm.size() != 2)
            return 2;

        if (cm[1].substr(0, 1) == "\"" &&
            cm[1].substr(cm[1].size() - 1, 1) == "\"") {
            cm[1] = cm[1].substr(
                1, cm[1].size() - 2); // 但是还是优化一下好一点 follow me
                                      // 路径里不允许>所以 、、看看solution
            // 因为直接把文件拖进来 路径里有空格的话 会带引号
            // l>main.ust
        }
        std::ifstream f(cm[1]);

        if (!f.is_open())
            return 3;

        std::string raw = std::string(std::istreambuf_iterator<char>(f),
                                      std::istreambuf_iterator<char>());

        INI_Object ini = ini_decode(raw);
        Project _p = parse(ini);
        ini.clear();
        load(_p);
        return 0;
    }

    if (cm[0] == "goto" || cm[0] == "g") {
        int tmp;
        if (cm.size() != 2)
            return 2;
        if (cm[1].substr(0, 1) == "p") {
            page = std::stoi(cm[1].substr(1, cm[1].size() - 1)) - 1;
            if (page < 0)
                page = 0;
            if (page > page_count)
                page = page_count;
        } else if (std::regex_match(cm[1], std::regex("^[0-9]+$"))) {
            tmp = std::stoi(cm[1]);
            if (tmp < 0)
                tmp = 0;
            if (tmp > p.notes.size() - 1)
                tmp = (int)(p.notes.size() - 1);
            page = (int)(tmp / 30);
        }

        reload();
        return 0;
    }

    if (cm[0] == "play" || cm[0] == "pl") {
        int line = 3;
        for (page = 0; page < page_count; page++) {
            for (; line < 33; line++) {
                std::cout << "\x1b[" + std::to_string(line) + ";1H";
                if (line != 3) {
                    std::cout << "\x1b[0m";
                    std::cout << "\x1b[" + std::to_string(line - 1) + ";1H";
                    outNote(line - 4 + page * 30);
                }
                std::cout << "\x1b[30;47m";
                outNote(line - 3 + page * 30);
                std::this_thread::sleep_for(std::chrono::nanoseconds(
                    (int)(60.0f * 1000000000.0f / p.tempo *
                          ((float)p.notes[line - 3 + page * 30].Length /
                           480.0f))));
            }
            line = 3; // here
            reload();
        }
        std::cout << "\x1b[0m\nDone";
        getch();
        page = 0;
        return 0;
    }
    return 1;
}
*/
// 函数定义内嵌入函数定义是禁止的语法 如下 C++ 11 已禁止想死，string甚至没有变换大小写的method。
// 这是？一些代码

// 
Parser get_cmd() {
    Parser p;
    p.set(":exit",
          [](const std::string &, UI *, Editor *) -> void { exit(0); });

    p.set(":redo", [](const std::string &, UI *, Editor *) -> void {});
    p.set(":play", [](const std::string &, UI *, Editor *) -> void {});
    
    p.set(":find", [](const std::string &cmd, UI *ui, Editor *editor) -> void {
        // 教我multi params
        std::vector<std::string> args = splitBy(cmd, ' ');
        // arguments should be sent in this format: 
        // [PROPERTY] [REGEX]
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

    p.set(":shownum", [](const std::string &, UI *, Editor *editor) -> void {
        editor->toggle_show_actual_notenum();
    }); // 有快捷键就不用命令了吧...？
    p.set(":showsec", [](const std::string &, UI *, Editor *editor) -> void {
        editor->toggle_show_sec();
    }); // 要不做成快捷键吧？k
    p.set(":load", [](const std::string &args, UI *ui, Editor *editor) -> void {
        if (args.empty()) {
            // ui calling
            ui->render_log(
                ColorText("Argument error. Press any key to continue...",
                          "\x1b[31m")
                    .output());
            ui->update();
            _getch();
        }
        std::string path = args;
        if (path[0] == '\"' && path[path.length() - 1] == '\"') {
            path = path.substr(1, path.length() - 2);
        }
        std::ifstream f(path);

        if (!f.is_open()) {
            ui->render_log(
                ColorText("File not found. Press any key to continue...",
                          "\x1b[31m")
                    .output());
            _getch();
        }
        // 这里可以抽象吗？k
        std::string raw = std::string(std::istreambuf_iterator<char>(f),
                                      std::istreambuf_iterator<char>());
        f.clear();
        INI_Object ini = ini_decode(raw);
        Project _p = parse(ini);
        ini.clear();
        editor->load(_p);
    });
    p.set_default([](const std::string &, UI *ui, Editor *) -> void {
        ui->render_log(
            ColorText("Command not found. Press any key to continue...",
                      "\x1b[31m")
                .output());
        ui->update();
        _getch();
    });
    return p;
}
#endif //_FUNC_H_
