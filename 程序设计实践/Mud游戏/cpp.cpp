#include "Game.h"
#include "Display.h"
#include <windows.h>
#include <conio.h>

int main() {
    Display::initDisplay();
    bool startGame = Display::showEntryPage();
    if (!startGame) {
        Display::cleanupDisplay();
        return 0;
    }

    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (GetConsoleMode(hConsole, &mode)) {
        mode &= ~ENABLE_QUICK_EDIT_MODE;
        SetConsoleMode(hConsole, mode);
    }

    Game* game = new Game();
    game->run();
    delete game;

    Display::cleanupDisplay();
    return 0;
}
