#include "Display.h"
#include "constants.h"
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <deque>

// Display类静态成员初始化
std::stringstream* Display::mapUIBuffer = nullptr;
std::deque<std::string>* Display::fightLogs = nullptr;

// Display类实现
void Display::initDisplay() {
    mapUIBuffer = new std::stringstream();
    fightLogs = new std::deque<std::string>();
    hideCursor();
    clearAllBuffers();
}

void Display::cleanupDisplay() {
    delete mapUIBuffer;
    delete fightLogs;
    mapUIBuffer = nullptr;
    fightLogs = nullptr;
}

void Display::hideCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void Display::showCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void Display::setCursorToTopLeft() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(hConsole, coord);
}

void Display::clearAllBuffers() {
    if (mapUIBuffer) {
        mapUIBuffer->str("");
        mapUIBuffer->clear();
    }
    if (fightLogs) fightLogs->clear();
    setColor(WHITE);
}

void Display::flushBuffer() {
    setCursorToTopLeft();
    if (mapUIBuffer) std::cout << mapUIBuffer->str();
    clearAllBuffers();
}

void Display::printMapToBuffer(const Map& map, const Player& player, Enemy* robot) {
    if (!mapUIBuffer) return;

    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            if (x == player.getX() && y == player.getY()) {
                setColor(GREEN);
                *mapUIBuffer << "@";
                setColor(WHITE);
                continue;
            }
            if (robot && robot->isAlive() && x == robot->getX() && y == robot->getY()) {
                setColor(RED);
                *mapUIBuffer << "R";
                setColor(WHITE);
                continue;
            }
            switch (map.getCell(x, y)) {
            case TILE_EMPTY: *mapUIBuffer << "."; break;
            case TILE_WALL: *mapUIBuffer << "#"; break;
            case TILE_BOX: *mapUIBuffer << "B"; break;
            case TILE_PHOTO_WALL: *mapUIBuffer << "P"; break;
            case TILE_SHELF: *mapUIBuffer << "S"; break;
            case TILE_DOOR: *mapUIBuffer << "D"; break;
            case TILE_TRASH: *mapUIBuffer << "T"; break;
            case TILE_MICROSCOPE: *mapUIBuffer << "M"; break;
            case TILE_CABINET: *mapUIBuffer << "C"; break;  // 柜子显示为C
            case TILE_REACT: *mapUIBuffer << "R"; break;
            case TILE_SAFE_DOOR: *mapUIBuffer << "S"; break;
            case TILE_BACK_DOOR: *mapUIBuffer << "B"; break;
            default: *mapUIBuffer << "?"; break;
            }
        }
        *mapUIBuffer << "\n";
    }
}

void Display::printUIToBuffer(const Map& map, const Player& player, const Backpack& backpack) {
    if (!mapUIBuffer) return;

    *mapUIBuffer << "========================" << "\n";
    setColor(GREEN);
    *mapUIBuffer << "玩家：" << player.getName();
    setColor(RED);
    *mapUIBuffer << " | HP：" << player.getHp();
    setColor(BLUE);
    *mapUIBuffer << " | 攻击力：" << player.getAttack();
    setColor(YELLOW);
    *mapUIBuffer << " | 钥匙：" << player.getKeyCount() << "\n";
    setColor(WHITE);

    *mapUIBuffer << "背包（" << backpack.getItemCount() << "/"
        << backpack.getMaxCap() << "）：";
    std::vector<std::string> items = backpack.getItemsName();
    if (items.empty()) *mapUIBuffer << "空";
    else for (int i = 0; i < backpack.getItemCount(); i++)
        *mapUIBuffer << "[" << i + 1 << "]" << items[i] << " ";

    *mapUIBuffer << "\n操作：WASD移动 | 空格查看交互 | 1-5使用物品 | Q退出 | F5存档 | L读档" << "\n";

    if (map.getMapType() == Map::ARCHIVE_ROOM) {
        if (!backpack.hasBlueprint()) {
            *mapUIBuffer << "目标：1. 查照片墙(P)拿密码 → 2. 书架(S)找钥匙 → 3. 开密码箱(B) 注意：有机器人追逐！" << "\n";
        }
        else {
            *mapUIBuffer << "目标：用青铜钥匙开(D)→前往生化实验室（图纸密码：19000715）" << "\n";
        }
    }
    else {
        if (!backpack.hasKey()) {
            *mapUIBuffer << "目标：1. 废纸篓(T)找线索 → 2. 显微镜(M)看顺序 → 3. 反应台(R)合成门卡 | B为返回门" << "\n";
        }
        else {
            *mapUIBuffer << "目标：用门卡+图纸密码打开安全门(S)逃生！ | B为返回门" << "\n";
        }
    }

    // 图例说明 - 按当前地图动态显示对应元素
    *mapUIBuffer << "【图例】";
    // 通用元素（两个地图都显示）
    setColor(GREEN);
    *mapUIBuffer << "@=玩家 ";
    setColor(RED);
    *mapUIBuffer << "R=机器人/怪物 ";
    setColor(WHITE);
    *mapUIBuffer << "#=墙壁 .=空地 ?=未知元素 ";

    // 根据地图类型显示专属元素
    if (map.getMapType() == Map::ARCHIVE_ROOM) {
        *mapUIBuffer << "D=档案室门 S=书架 ";
        *mapUIBuffer << "P=照片墙 B=密码箱 ";
    }
    else if (map.getMapType() == Map::LAB) {
        *mapUIBuffer << "S=安全门 B=返回门 ";
        *mapUIBuffer << "T=废纸篓 M=显微镜 C=柜子 R=反应台 ";  // 显示柜子
    }
    *mapUIBuffer << "\n";

    *mapUIBuffer << "========================" << "\n";
}

void Display::renderAll(const Map& map, const Player& player, const Backpack& backpack, Enemy* robot) {
    clearAllBuffers();
    printMapToBuffer(map, player, robot);
    printUIToBuffer(map, player, backpack);
    flushBuffer();
}

void Display::printGameOver(bool isWin) {
    system("cls");
    setColor(isWin ? GREEN : RED);
    int consoleWidth = 80;
    std::string msg = isWin ? "恭喜逃脱！" : "HP耗尽！";
    int msgPos = (consoleWidth - msg.size()) / 2;

    for (int i = 0; i < 10; i++) std::cout << std::endl;
    std::cout << std::string(consoleWidth, '=') << std::endl;
    std::cout << std::string(msgPos, ' ') << msg << std::endl;
    std::cout << std::string(consoleWidth, '=') << std::endl;

    setColor(WHITE);
    std::cout << "\n按ESC键退出..." << std::endl;
    while (_getch() != 27);
}

void Display::drawBorderedPage(const std::string& title, const std::vector<std::string>& content, bool needInput) {
    system("cls");
    const int PAGE_WIDTH = 60;
    const int CONTENT_WIDTH = PAGE_WIDTH - 4;

    setColor(YELLOW);
    std::cout << std::string(PAGE_WIDTH, '=') << std::endl;
    int titlePad = (PAGE_WIDTH - title.size()) / 2;
    std::cout << std::string(titlePad, ' ') << title << std::string(PAGE_WIDTH - titlePad - title.size(), ' ') << std::endl;
    std::cout << std::string(PAGE_WIDTH, '=') << std::endl;
    setColor(WHITE);

    for (const auto& line : content) {
        if (line.empty()) {
            std::cout << "|" << std::string(CONTENT_WIDTH, ' ') << "|" << std::endl;
            continue;
        }
        std::string remaining = line;
        while (!remaining.empty()) {
            std::string sub = remaining.substr(0, CONTENT_WIDTH);
            std::cout << "| " << sub << std::string(CONTENT_WIDTH - sub.size(), ' ') << " |" << std::endl;
            if (remaining.size() > CONTENT_WIDTH) remaining = remaining.substr(CONTENT_WIDTH);
            else remaining.clear();
        }
    }

    if (needInput) {
        std::cout << std::string(PAGE_WIDTH, '-') << std::endl;
        setColor(GREEN);
        std::cout << "| 请输入内容后按回车确认：" << std::string(CONTENT_WIDTH - 18, ' ') << " |" << std::endl;
        setColor(WHITE);
        std::cout << "| > ";
    }
    else {
        std::cout << std::string(PAGE_WIDTH, '-') << std::endl;
        setColor(BLUE);
        std::cout << "| 按任意键返回游戏..." << std::string(CONTENT_WIDTH - 12, ' ') << " |" << std::endl;
        setColor(WHITE);
    }

    setColor(YELLOW);
    std::cout << std::string(PAGE_WIDTH, '=') << std::endl;
    setColor(WHITE);
}

void Display::showCluePage(const std::string& title, const std::vector<std::string>& clues) {
    drawBorderedPage(title, clues, false);
    _getch();
}

std::string Display::showInputPage(const std::string& title, const std::vector<std::string>& clues, const std::string& inputPrompt) {
    drawBorderedPage(title, clues, true);
    std::string input;
    getline(std::cin, input);
    hideCursor();
    return input;
}

void Display::showResultPage(const std::string& result, bool isSuccess) {
    system("cls");
    setColor(isSuccess ? GREEN : RED);
    int consoleWidth = 80;
    std::string border = std::string(consoleWidth, '=');

    std::cout << border << std::endl;
    std::cout << std::string((consoleWidth - result.size()) / 2, ' ') << result << std::endl;
    std::cout << border << std::endl;
    setColor(WHITE);

    std::cout << "\n  2秒后返回地图..." << std::endl;
    Sleep(2000);
}

void Display::addFightLog(const std::string& log, Color color) {
    if (!fightLogs) return;

    std::string coloredLog;
    switch (color) {
    case RED: coloredLog = "\033[1;31m" + log + "\033[0m"; break;
    case GREEN: coloredLog = "\033[1;32m" + log + "\033[0m"; break;
    default: coloredLog = log; break;
    }
    fightLogs->push_back(coloredLog);
    if (fightLogs->size() > 10) fightLogs->pop_front();
}

void Display::renderFightUI(const Player& player, const Enemy& enemy) {
    if (!fightLogs) return;

    system("cls");
    setColor(YELLOW);
    std::cout << "===================================== 战斗界面 =====================================" << std::endl;
    setColor(WHITE);

    setColor(GREEN);
    std::cout << "玩家：" << player.getName() << " | HP：" << player.getHp() << " | 攻击力：" << player.getAttack() << std::endl;
    setColor(RED);
    std::cout << "敌人：" << enemy.getName() << " | HP：" << enemy.getHp() << " | 攻击力：" << enemy.getAttack() << std::endl;
    setColor(WHITE);
    std::cout << "-------------------------------------------------------------------------------------" << std::endl;

    for (const auto& log : *fightLogs) {
        std::cout << log << std::endl;
    }
    std::cout << "-------------------------------------------------------------------------------------" << std::endl;
    std::cout << "操作：1-攻击  2-躲闪  Q-逃跑（失败率50%）" << std::endl;
    setColor(YELLOW);
    std::cout << "====================================================================================" << std::endl;
    setColor(WHITE);
}

void Display::clearFightLogs() {
    if (fightLogs) fightLogs->clear();
}

void Display::drawGameTitle() {
    setColor(YELLOW);
    std::cout << "==========================================================================" << std::endl;
    std::cout << "  _____                  _   _               _       _                   " << std::endl;
    std::cout << " | ____|__ _ ___ _   _  | | | | __ _ _ __   | | __ _| |__  ___ _ __      " << std::endl;
    std::cout << " |  _| / _` / __| | | | | |_| |/ _` | '_ \\  | |/ _` | '_ \\/ __| '_ \\     " << std::endl;
    std::cout << " | |__| (_| \\__ \\ |_| | |  _  | (_| | | | | | | (_| | |_) \\__ \\ | | |    " << std::endl;
    std::cout << " |_____\\__,_|___/\\__, | |_| |_|\\__,_|_| |_| |_|\\__,_|_.__/|___/_| |_|    " << std::endl;
    std::cout << "                 |___/                                                   " << std::endl;
    std::cout << "                                                                          " << std::endl;
    std::cout << "               ____            _    _ _   _ _                             " << std::endl;
    std::cout << "              / ___|___  _ __ | | _| | |_(_) |___                         " << std::endl;
    std::cout << "             | |   / _ \\| '_ \\| |/ / | __| | / __|                        " << std::endl;
    std::cout << "             | |__| (_) | | | |   <| | |_| | \\__ \\                        " << std::endl;
    std::cout << "              \\____\\___/|_| |_|_|\\_\\_|\\__|_|_|___/                        " << std::endl;
    std::cout << "==========================================================================" << std::endl;
    setColor(WHITE);
}

bool Display::showEntryPage() {
    system("cls");
    hideCursor();
    clearAllBuffers();

    drawGameTitle();
    std::cout << std::endl << std::endl;

    int indent = 4;
    setColor(GREEN);
    std::cout << std::string(indent, ' ') << "【游戏提示】" << std::endl;
    setColor(WHITE);
    std::cout << std::string(indent, ' ') << "* 建议先开启全屏模式，获得最佳体验！" << std::endl;
    std::cout << std::string(indent, ' ') << "* 全屏快捷键：F11（部分系统为 Alt + Enter）" << std::endl;
    std::cout << std::string(indent, ' ') << "* 操作说明：WASD移动 | 空格查看交互 | 1-5使用物品 | Q退出 | F5存档 | L读档" << std::endl << std::endl;

    setColor(BLUE);
    std::cout << std::string(indent, ' ') << "【背景故事】" << std::endl;
    setColor(WHITE);
    std::cout << std::string(indent, ' ') << "你是一名调查员，为追查1900年的病毒实验真相闯入废弃档案室。" << std::endl;
    std::cout << std::string(indent, ' ') << "破解密码、躲避机器人、合成解毒剂——逃离这片危机四伏的区域！" << std::endl << std::endl;

    setColor(RED);
    std::cout << std::string(indent, ' ') << ">> 请确认已开启全屏后，按【空格键】开始游戏 | 按【Q】退出 <<" << std::endl;
    setColor(WHITE);

    while (true) {
        if (_kbhit()) {
            char key = _getch();
            if (toupper(key) == ' ') {
                system("cls");
                return true;
            }
            else if (toupper(key) == 'Q') {
                return false;
            }
        }
        Sleep(50);
    }
}

// 存档相关UI实现
int Display::showSaveSlotSelection(bool forLoading) {
    std::vector<std::string> content = {
        "请选择" + std::string(forLoading ? "读取" : "保存") + "的存档位置：",
        "",
        "[1] 存档位置 1",
        "[2] 存档位置 2",
        "[3] 存档位置 3",
        "",
        "按对应数字键选择，按ESC取消"
    };

    drawBorderedPage(forLoading ? "读取存档" : "保存存档", content, false);

    while (true) {
        if (_kbhit()) {
            char key = _getch();
            if (key >= '1' && key <= '3') {
                return key - '0';
            }
            else if (key == 27) {
                return 0;
            }
        }
        Sleep(50);
    }
}

void Display::showSaveSuccess() {
    std::vector<std::string> content = {
        "游戏已成功保存！",
        "",
        "可以通过按L键读取此存档继续游戏"
    };
    showCluePage("保存成功", content);
}

void Display::showLoadSuccess() {
    std::vector<std::string> content = {
        "存档已成功加载！",
        "",
        "继续你的冒险吧"
    };
    showCluePage("加载成功", content);
}

void Display::showSaveError(const std::string& message) {
    std::vector<std::string> content = {
        "操作失败：" + message,
        "",
        "请重试或选择其他存档位置"
    };
    showCluePage("操作失败", content);
}
