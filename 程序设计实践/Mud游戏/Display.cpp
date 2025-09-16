#include "Display.h"
#include "constants.h"
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <deque>

// Display�ྲ̬��Ա��ʼ��
std::stringstream* Display::mapUIBuffer = nullptr;
std::deque<std::string>* Display::fightLogs = nullptr;

// Display��ʵ��
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
            case TILE_CABINET: *mapUIBuffer << "C"; break;  // ������ʾΪC
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
    *mapUIBuffer << "��ң�" << player.getName();
    setColor(RED);
    *mapUIBuffer << " | HP��" << player.getHp();
    setColor(BLUE);
    *mapUIBuffer << " | ��������" << player.getAttack();
    setColor(YELLOW);
    *mapUIBuffer << " | Կ�ף�" << player.getKeyCount() << "\n";
    setColor(WHITE);

    *mapUIBuffer << "������" << backpack.getItemCount() << "/"
        << backpack.getMaxCap() << "����";
    std::vector<std::string> items = backpack.getItemsName();
    if (items.empty()) *mapUIBuffer << "��";
    else for (int i = 0; i < backpack.getItemCount(); i++)
        *mapUIBuffer << "[" << i + 1 << "]" << items[i] << " ";

    *mapUIBuffer << "\n������WASD�ƶ� | �ո�鿴���� | 1-5ʹ����Ʒ | Q�˳� | F5�浵 | L����" << "\n";

    if (map.getMapType() == Map::ARCHIVE_ROOM) {
        if (!backpack.hasBlueprint()) {
            *mapUIBuffer << "Ŀ�꣺1. ����Ƭǽ(P)������ �� 2. ���(S)��Կ�� �� 3. ��������(B) ע�⣺�л�����׷��" << "\n";
        }
        else {
            *mapUIBuffer << "Ŀ�꣺����ͭԿ�׿�(D)��ǰ������ʵ���ң�ͼֽ���룺19000715��" << "\n";
        }
    }
    else {
        if (!backpack.hasKey()) {
            *mapUIBuffer << "Ŀ�꣺1. ��ֽ¨(T)������ �� 2. ��΢��(M)��˳�� �� 3. ��Ӧ̨(R)�ϳ��ſ� | BΪ������" << "\n";
        }
        else {
            *mapUIBuffer << "Ŀ�꣺���ſ�+ͼֽ����򿪰�ȫ��(S)������ | BΪ������" << "\n";
        }
    }

    // ͼ��˵�� - ����ǰ��ͼ��̬��ʾ��ӦԪ��
    *mapUIBuffer << "��ͼ����";
    // ͨ��Ԫ�أ�������ͼ����ʾ��
    setColor(GREEN);
    *mapUIBuffer << "@=��� ";
    setColor(RED);
    *mapUIBuffer << "R=������/���� ";
    setColor(WHITE);
    *mapUIBuffer << "#=ǽ�� .=�յ� ?=δ֪Ԫ�� ";

    // ���ݵ�ͼ������ʾר��Ԫ��
    if (map.getMapType() == Map::ARCHIVE_ROOM) {
        *mapUIBuffer << "D=�������� S=��� ";
        *mapUIBuffer << "P=��Ƭǽ B=������ ";
    }
    else if (map.getMapType() == Map::LAB) {
        *mapUIBuffer << "S=��ȫ�� B=������ ";
        *mapUIBuffer << "T=��ֽ¨ M=��΢�� C=���� R=��Ӧ̨ ";  // ��ʾ����
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
    std::string msg = isWin ? "��ϲ���ѣ�" : "HP�ľ���";
    int msgPos = (consoleWidth - msg.size()) / 2;

    for (int i = 0; i < 10; i++) std::cout << std::endl;
    std::cout << std::string(consoleWidth, '=') << std::endl;
    std::cout << std::string(msgPos, ' ') << msg << std::endl;
    std::cout << std::string(consoleWidth, '=') << std::endl;

    setColor(WHITE);
    std::cout << "\n��ESC���˳�..." << std::endl;
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
        std::cout << "| ���������ݺ󰴻س�ȷ�ϣ�" << std::string(CONTENT_WIDTH - 18, ' ') << " |" << std::endl;
        setColor(WHITE);
        std::cout << "| > ";
    }
    else {
        std::cout << std::string(PAGE_WIDTH, '-') << std::endl;
        setColor(BLUE);
        std::cout << "| �������������Ϸ..." << std::string(CONTENT_WIDTH - 12, ' ') << " |" << std::endl;
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

    std::cout << "\n  2��󷵻ص�ͼ..." << std::endl;
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
    std::cout << "===================================== ս������ =====================================" << std::endl;
    setColor(WHITE);

    setColor(GREEN);
    std::cout << "��ң�" << player.getName() << " | HP��" << player.getHp() << " | ��������" << player.getAttack() << std::endl;
    setColor(RED);
    std::cout << "���ˣ�" << enemy.getName() << " | HP��" << enemy.getHp() << " | ��������" << enemy.getAttack() << std::endl;
    setColor(WHITE);
    std::cout << "-------------------------------------------------------------------------------------" << std::endl;

    for (const auto& log : *fightLogs) {
        std::cout << log << std::endl;
    }
    std::cout << "-------------------------------------------------------------------------------------" << std::endl;
    std::cout << "������1-����  2-����  Q-���ܣ�ʧ����50%��" << std::endl;
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
    std::cout << std::string(indent, ' ') << "����Ϸ��ʾ��" << std::endl;
    setColor(WHITE);
    std::cout << std::string(indent, ' ') << "* �����ȿ���ȫ��ģʽ�����������飡" << std::endl;
    std::cout << std::string(indent, ' ') << "* ȫ����ݼ���F11������ϵͳΪ Alt + Enter��" << std::endl;
    std::cout << std::string(indent, ' ') << "* ����˵����WASD�ƶ� | �ո�鿴���� | 1-5ʹ����Ʒ | Q�˳� | F5�浵 | L����" << std::endl << std::endl;

    setColor(BLUE);
    std::cout << std::string(indent, ' ') << "���������¡�" << std::endl;
    setColor(WHITE);
    std::cout << std::string(indent, ' ') << "����һ������Ա��Ϊ׷��1900��Ĳ���ʵ�����ള����������ҡ�" << std::endl;
    std::cout << std::string(indent, ' ') << "�ƽ����롢��ܻ����ˡ��ϳɽⶾ������������ƬΣ���ķ�������" << std::endl << std::endl;

    setColor(RED);
    std::cout << std::string(indent, ' ') << ">> ��ȷ���ѿ���ȫ���󣬰����ո������ʼ��Ϸ | ����Q���˳� <<" << std::endl;
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

// �浵���UIʵ��
int Display::showSaveSlotSelection(bool forLoading) {
    std::vector<std::string> content = {
        "��ѡ��" + std::string(forLoading ? "��ȡ" : "����") + "�Ĵ浵λ�ã�",
        "",
        "[1] �浵λ�� 1",
        "[2] �浵λ�� 2",
        "[3] �浵λ�� 3",
        "",
        "����Ӧ���ּ�ѡ�񣬰�ESCȡ��"
    };

    drawBorderedPage(forLoading ? "��ȡ�浵" : "����浵", content, false);

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
        "��Ϸ�ѳɹ����棡",
        "",
        "����ͨ����L����ȡ�˴浵������Ϸ"
    };
    showCluePage("����ɹ�", content);
}

void Display::showLoadSuccess() {
    std::vector<std::string> content = {
        "�浵�ѳɹ����أ�",
        "",
        "�������ð�հ�"
    };
    showCluePage("���سɹ�", content);
}

void Display::showSaveError(const std::string& message) {
    std::vector<std::string> content = {
        "����ʧ�ܣ�" + message,
        "",
        "�����Ի�ѡ�������浵λ��"
    };
    showCluePage("����ʧ��", content);
}
