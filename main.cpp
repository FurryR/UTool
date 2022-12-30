#include "editor.h"
#include "func.h"
#include "logger.h"
#include "parser.h"
#include "project.h"
#include <fstream>
#include <iostream>
#include <windows.h>
void main_ui(Screen *screen) {
    UI ui(screen);
    Editor editor;
    Parser parser = get_cmd();
    UINT cp = GetACP();
    bool flag = false;
    if (cp != 932) {
        SetConsoleOutputCP(932);
        // logger.log_info(
        //    "Your code page is not UTAU-friendly. Switched to 932.\n");
    }
    while (true) {
        editor.render(&ui);
        if ((!flag) && cp != 932) {
            ui.render_log(
                ColorText("Your code page (" + std::to_string(cp) +
                              ") is not UTAU-friendly. Switched to 932.\n",
                          "\x1b[33m")
                    .output());
        }
        int g = _getch();
        if (g == ':') {
            // 命令系统
            std::string tmp = ":";
            char key;
            ui.render_log(ColorText(tmp, "").output());
            ui.update();
            while ((key = getch())) {
                if (key == '\r') {
                    parser.execute(tmp, &ui, &editor);
                    break;
                } else if (key == '\x1b') {
                    break;
                } else if (key == '\b') {
                    if (tmp.length() > 1)
                        tmp.pop_back();
                } else {
                    tmp += key;
                }
                ui.render_log(ColorText(tmp, "").output());
                ui.update();
            }
        } else {
            editor.process(&ui, g); // 好会写 没需求就删了，不留任何额外参数。（来自 Lightpad）
        }
    }
    _getch();
    // test?
}

int main() {
#ifdef _WIN32
    HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hStd, &dwMode);
    dwMode |= 0x4;
    SetConsoleMode(hStd, dwMode);
#endif
    // use \x1b or \x1b instead of SetConsoleTextAttribute
    // For example: \x1b[45m
    // HWND console = GetConsoleWindow();
    // RECT r;
    // GetWindowRect(console, &r);
    // MoveWindow(console, r.left, r.top, 800, 600, TRUE);

    // Logger logger;

    // std::cout << "uPET ver0.1b1 - UTAU Project Editing Tool\n";
    // std::cout
    //     << "Copyright ookamitai, FurryR, 2022-2023. All rights
    //     reserved.\n\n";

    // std::string filename = "main.ust";
    // std::ifstream f(filename);

    // UINT cp = GetACP();
    // logger.log_info("Code page: " + std::to_string(cp) + "\n");
    // if (cp != 932) {
    //  SetConsoleOutputCP(932);
    // logger.log_info(
    //    "Your code page is not UTAU-friendly. Switched to 932.\n");
    // }
    // // 从这里开始不再允许使用 logger。

    // logger.log_info("Looking for main.ust...\n");

    // while (!f.is_open()) {
    //     logger.log_error("File could not be read.\n");
    //     logger.log_prompt("Enter new file path:");
    //     std::getline(std::cin, filename);
    //     if (filename.substr(0, 1) == "\"" &&
    //         filename.substr(filename.size() - 1, 1) == "\"") {
    //         filename = filename.substr(1, filename.size() - 2);
    //         logger.log_info("Recognized file path as: " + filename + "\n");
    //     }
    //     f.open(filename);
    // }

    // logger.log_info("Found file.\n");
    // std::string raw = std::string(std::istreambuf_iterator<char>(f),
    //                               std::istreambuf_iterator<char>());
    // INI_Object ini = ini_decode(raw);
    // logger.log_info("UST file parsed, ini file key count: " +
    //                 std::to_string(ini.size()) + "\n");
    // // ini 里面是数据

    // logger.log_info("Converting to project...\n");
    // logger.log_info("Thanks to FurryR's BlazinglyFast(R) code for making this
    // "
    //                 "part faster!\n");
    // Project pj = parse(ini);
    // // save to same file
    // f.close();
    // logger.log_info("File closed.\n");

    // // Project pj = parse(ini);
    // logger.log_info("Conversion finished, notes in project: " +
    //                 std::to_string(pj.notes.size()) + "\n");
    // ini.clear();
    // logger.log_info("ini object cleared.\n");
    // logger.log_info("Loaded: " + pj.project_name + "\n");

    // logger.log_info("Loading Editor...\n");
    // logger.log_info("Commands:\n"
    //                 "  next(n) - Goto next page.\n"
    //                 "  prev(p) - Goto previous page.\n"
    //                 "  goto(g)>[index | page] - Goto specified note or page
    //                 by " "either using numbers or format as 'p3'\n" "
    //                 firstpage(fp) - Goto the first page.\n" "  lastpage(lp) -
    //                 Goto the last page.\n" "  load>[file] - Load specified
    //                 file\n" "Any key to continue...");
    Screen screen(getsize());
    main_ui(&screen);
    // getch();
    // Editor edit;
    // edit.load(pj);
    // edit.revoker();

    // system("pause>nul");
    return 0;
}