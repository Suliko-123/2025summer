#ifndef DISPLAY_H
#define DISPLAY_H

#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Backpack.h"
#include <string>
#include <vector>
#include <sstream>
#include <deque>
#include <windows.h>

class Display {
private:
    static std::stringstream* mapUIBuffer;
    static std::deque<std::string>* fightLogs;

    static void setColor(WORD color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    }

    static void drawBorderedPage(const std::string& title, const std::vector<std::string>& content, bool needInput = false);

public:
    enum Color {
        WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        RED = FOREGROUND_RED | FOREGROUND_INTENSITY,
        GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        BLUE = FOREGROUND_BLUE | FOREGROUND_INTENSITY
    };

    static void initDisplay();
    static void cleanupDisplay();
    static void hideCursor();
    static void showCursor();
    static void setCursorToTopLeft();
    static void clearAllBuffers();
    static void flushBuffer();
    static void printMapToBuffer(const Map& map, const Player& player, Enemy* robot = nullptr);
    static void printUIToBuffer(const Map& map, const Player& player, const Backpack& backpack);
    static void renderAll(const Map& map, const Player& player, const Backpack& backpack, Enemy* robot = nullptr);
    static void printGameOver(bool isWin);

    static void showCluePage(const std::string& title, const std::vector<std::string>& clues);
    static std::string showInputPage(const std::string& title, const std::vector<std::string>& clues, const std::string& inputPrompt);
    static void showResultPage(const std::string& result, bool isSuccess);

    static void addFightLog(const std::string& log, Color color = WHITE);
    static void renderFightUI(const Player& player, const Enemy& enemy);
    static void clearFightLogs();

    static bool showEntryPage();
    static void drawGameTitle();

    // ´æµµÏà¹ØUI
    static int showSaveSlotSelection(bool forLoading);
    static void showSaveSuccess();
    static void showLoadSuccess();
    static void showSaveError(const std::string& message);
};

#endif // DISPLAY_H
