#include<ftxui/dom/elements.hpp>
#include<ftxui/screen/screen.hpp>
#include<ftxui/component/screen_interactive.hpp>
#include<ftxui/component/component.hpp>
#include "utils/logger.hpp"

#include"tui/mainMenu.hpp"
#include"tui/course.hpp"
#include"tui/assignment.hpp"

int main() {
    logger::init_logger();
    logger::log_info("Application started.");
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();
    static int cur = 0;

    auto curTab = Container::Tab({
        mainMenu(screen, cur),
        course(screen, cur),
        assignment(screen, cur),
    }, &cur);

    auto mainComponent = CatchEvent(curTab, [&](Event e) {
        return false;
    });

    screen.Loop(mainComponent);
    logger::log_info("Application finished.");
    return 0;
}
