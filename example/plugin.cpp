#include "../command.h"
#include "../editor.h"
extern "C" std::pair<std::string, Parser> _export() {
    Parser p;
    p.set("help", [](const std::string&, Parser*, UI* ui, Editor*) -> bool {
        ui->render_log(
            ColorText("Example Plugin by FurryR, use :owo vcv2cv", "")
                .output());
        ui->update();
        return false;
    });
    p.set("vcv2cv",
          [](const std::string&, Parser*, UI* ui, Editor* editor) -> bool {
              size_t idx = 0;
              for (auto&& i : editor->project().notes) {
                  if (i.Lyric.length() >= 3) {
                      if (((i.Lyric[0] >= 'a' && i.Lyric[0] <= 'z') ||
                           i.Lyric[0] == '-') &&
                          i.Lyric.substr(2) != "R") {
                          Note after = i;
                          after.Lyric = after.Lyric.substr(2);
                          editor->action(
                              Action(ActionType::Modify, i, after, idx));
                      }
                  }
                  idx++;
              }
              editor->render(ui);
              ui->render_log(ColorText("Done!", "").output());
              ui->update();
              return false;
          });
    p.set_default([](const std::string&, Parser*, UI* ui, Editor*) -> bool {
        ui->render_log(
            ColorText("Usage: owo [help|vcv2cv]", "\x1b[31m").output());
        ui->update();
        return false;
    });
    return std::make_pair("owo", p);
}