#ifndef _COMMAND_H_
#define _COMMAND_H_
#include <functional>
#include <map>
#include <memory>

#include "editor.h"

// refresh(argument, UI, editor...)
typedef std::function<bool(const std::string &, UI *, Editor *)> Command;
typedef class Parser {
    std::map<std::string, Command> cmd;
    Command default_command;

   public:
    Parser() {}
    bool execute(const std::string &command, UI *ui, Editor *editor) const {
        size_t idx = command.find_first_of(' ');
        std::string name, arg;
        if (idx == std::string::npos) {
            name = command;
            arg = "";
        } else {
            name = command.substr(0, idx);
            arg = command.substr(name.length() + 1);
        }
        if (cmd.find(name) != cmd.cend() && cmd.at(name)) {
            return cmd.at(name)(arg, ui, editor);
        }
        if (default_command)
            return default_command(command, ui, editor);
        else
            return true;
    }
    void set_default(const Command &fn) { default_command = fn; }
    void set(const std::string &name, const Command &fn) { cmd[name] = fn; }
} Parser;
#endif