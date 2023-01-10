#ifndef _SELECTOR_H_
#define _SELECTOR_H_
#include <conio.h>

#include <fstream>
#include <utility>

#include "ui.h"

/**
 * @brief 选项(子 UI) 逻辑。
 */
typedef struct Selector {
    std::vector<std::string> choice;
    std::string text;
    std::string log;
    size_t current;
    /**
     * @brief 输入的处理事件。
     *
     * @param ui UI 实例。
     * @param input 输入的字符。
     * @return bool 是否更新屏幕。
     */
    bool process(int input) {
        switch (input) {
            case 224: {
                switch (_getch()) {
                    case 72: {
                        // 上一个选项
                        if (current > 0)
                            current--;
                        else
                            current = choice.size() - 1;
                        break;
                    }
                    case 80: {
                        // 下一个选项
                        if (current < choice.size() - 1)
                            current++;
                        else
                            current = 0;
                        break;
                    }
                }
                break;
            }
        }
        return true;
    }
    /**
     * @brief 渲染界面。
     *
     * @param ui UI 实例
     */
    void render(SelectUI *ui) const {
        ui->clear();
        ui->render_bar(ColorText(text, "").output());
        if (choice.empty()) {
            ui->render_choice({"(Nothing to show)"}, 0);
        } else {
            ui->render_choice(choice, current);
        }
        ui->render_log(ColorText(log, "").output());
        return;
    }
    Selector(const std::string &text, const std::vector<std::string> &choice,
             const std::string &log, size_t current = 0)
        : choice(choice), text(text), log(log), current(current) {}
} Selector;
#endif