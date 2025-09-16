#include "Game.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Object.h"
#include "Backpack.h"
#include "Display.h"
#include "MapState.h"
#include "constants.h"
#include <windows.h>
#include <conio.h>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <stdexcept>

std::vector<Thing*> Game::copyEntitiesExceptPlayerAndRobot(const std::vector<Thing*>& original, Player* player, Enemy* robot) {
    std::vector<Thing*> result;
    for (auto entity : original) {
        if (entity != player && entity != robot) {
            if (Object* obj = dynamic_cast<Object*>(entity)) {
                if (obj->isAlive()) {
                    result.push_back(new Object(obj->getObjectType(), obj->getX(), obj->getY()));
                }
            }
            else if (Enemy* e = dynamic_cast<Enemy*>(entity)) {
                if (e != robot && e->isAlive()) {
                    Enemy* newEnemy = new Enemy(e->getX(), e->getY(), e->getName(), e->isChaserType());
                    while (newEnemy->getHp() > e->getHp()) {
                        newEnemy->reduceHp(1);
                    }
                    result.push_back(newEnemy);
                }
            }
        }
    }
    return result;
}

void Game::ensureSaveDirectoryExists() const {
    CreateDirectoryA(SAVE_DIRECTORY.c_str(), NULL);
}

Game::Game()
    : player(nullptr), currentMap(nullptr), backpack(nullptr), archiveRobot(nullptr),
    isRunning(true), currentMapIdx(0), needRender(true), lastRobotMoveTime(clock()),
    // ��ʼ��������ƷΪδ��ȡ״̬
    archiveLootCollected(false), doorCardObtained(false),
    weaponObtained(false), keyObtained(false), hpPotionObtained(false),
    cabinetMonsterDefeated(false) {
    backpack = new Backpack();
    player = new Player(1, 1, "ð����");
    currentMap = new Map(Map::ARCHIVE_ROOM);
    currentMap->addEntity(player);
    // ��ʼ���Կ�ף���ֻ��δ��ȡ״̬��
    if (!keyObtained) {
        currentMap->addEntity(new Object(Object::KEY, 60, 15));
    }
    mapHistory = new std::stack<MapState*>();
    Display::initDisplay();
}

Game::~Game() {
    delete player;
    delete backpack;
    delete currentMap;
    if (archiveRobot) delete archiveRobot;

    while (!mapHistory->empty()) {
        MapState* state = mapHistory->top();
        mapHistory->pop();
        delete state;
    }
    delete mapHistory;

    Display::cleanupDisplay();
}

void Game::spawnArchiveRobot() {
    if (archiveRobot != nullptr || currentMap->getMapType() != Map::ARCHIVE_ROOM) return;

    int x = rand() % 2 == 0 ? 1 : currentMap->getWidth() - 2;
    int y = rand() % (currentMap->getHeight() - 2) + 1;

    archiveRobot = new Enemy(x, y, "�����һ�����", true);
    currentMap->addEntity(archiveRobot);
    std::vector<std::string> clues = { "[����] �����һ����˱����", "��ɫ����������㣬��ʼ׷��" };
    Display::showCluePage("Σ�վ���", clues);
    needRender = true;
}

void Game::dropWeaponAtRobotPos() {
    // ��������ѱ���ȡ����������
    if (weaponObtained || !archiveRobot || archiveRobot->isAlive()) return;

    Object* weapon = new Object(Object::WEAPON, archiveRobot->getX(), archiveRobot->getY());
    currentMap->addEntity(weapon);
    std::vector<std::string> clues = { "[ս��Ʒ] ���ܻ����ˣ�", "�����ˡ���ͭ�˹�������������ʰȡ��" };
    Display::showCluePage("ս������", clues);
    needRender = true;
}

void Game::switchMap(Map::MapType targetType) {
    if (currentMap == nullptr || player == nullptr) return;

    std::vector<Thing*> copiedEntities = copyEntitiesExceptPlayerAndRobot(currentMap->getEntities(), player, archiveRobot);

    // ���浱ǰ״̬ʱ����������Ʒ��ȡ״̬
    mapHistory->push(new MapState(
        currentMap->getMapType(),
        player,
        archiveRobot,
        copiedEntities,
        archiveLootCollected,
        doorCardObtained,
        weaponObtained,
        keyObtained,
        hpPotionObtained,
        cabinetMonsterDefeated
    ));

    currentMap->removeEntity(player);
    if (archiveRobot) {
        currentMap->removeEntity(archiveRobot);
    }

    delete currentMap;
    currentMap = new Map(targetType);
    currentMap->addEntity(player);

    if (targetType == Map::ARCHIVE_ROOM) {
        currentMapIdx = 0;
        if (archiveRobot != nullptr && archiveRobot->isAlive()) {
            currentMap->addEntity(archiveRobot);
        }
        // ���Կ��δ����ȡ�������Կ��ʵ��
        if (!keyObtained) {
            currentMap->addEntity(new Object(Object::KEY, 60, 15));
        }
    }
    else if (targetType == Map::LAB) {
        currentMapIdx = 1;
        // ֻ��û�л��ܹ��ӹ��������²����ʵ���ҵ���
        if (!cabinetMonsterDefeated) {
            Enemy* labEnemy = new Enemy(30, 10, "ʵ�����");
            currentMap->addEntity(labEnemy);
        }
        player->setPos(1, 2);
    }

    std::string mapName = (targetType == Map::ARCHIVE_ROOM) ? "������" : "����ʵ����";
    std::vector<std::string> clues = { "[�����л�] ����" + mapName + "��" };
    if (targetType == Map::LAB) {
        clues.push_back("��������������ɫ��������������й©��");
        clues.push_back("(B��Ǵ�Ϊ���ص����ҵ���)");
    }
    else {
        clues.push_back("�ص��˵����ң�С�Ļ����ˣ�");
    }
    Display::showCluePage("������ʾ", clues);
    needRender = true;
}

void Game::returnToPreviousMap() {
    if (mapHistory->empty()) {
        std::vector<std::string> clues = { "[��ʾ] �޷����أ����ǳ�ʼ��ͼ��" };
        Display::showCluePage("��ͼ��ʾ", clues);
        return;
    }

    MapState* prevState = mapHistory->top();
    mapHistory->pop();

    // �ָ�������Ʒ��ȡ״̬
    archiveLootCollected = prevState->archiveLootCollected;
    doorCardObtained = prevState->doorCardObtained;
    weaponObtained = prevState->weaponObtained;
    keyObtained = prevState->keyObtained;
    hpPotionObtained = prevState->hpPotionObtained;
    cabinetMonsterDefeated = prevState->cabinetMonsterDefeated;

    Enemy* currentRobot = (currentMap->getMapType() == Map::ARCHIVE_ROOM) ? archiveRobot : nullptr;

    // �����ǰ��ͼ��ʵ��
    std::vector<Thing*> entitiesToRemove = currentMap->getEntities();
    for (auto entity : entitiesToRemove) {
        currentMap->removeEntity(entity);
        if (entity != player && entity != currentRobot) {
            delete entity;
        }
    }

    // ���浱ǰ������ָ�루����У�
    Enemy* tempRobot = archiveRobot;
    if (currentMap->getMapType() != Map::ARCHIVE_ROOM) {
        archiveRobot = nullptr;
    }

    // ɾ����ǰ��ͼ
    delete currentMap;

    // �����µ�ͼ���ָ�״̬
    currentMap = new Map(prevState->mapType);
    currentMap->clearEntities();

    // �ָ����λ��
    player->setPos(prevState->playerX, prevState->playerY);
    currentMap->addEntity(player);

    // �ָ�ʵ��
    for (auto entity : *prevState->entities) {
        currentMap->addEntity(entity);
    }

    // �ָ�������
    if (prevState->archiveRobot) {
        archiveRobot = prevState->archiveRobot;
        if (archiveRobot->isAlive()) {
            currentMap->addEntity(archiveRobot);
        }
    }
    else {
        archiveRobot = nullptr;
    }

    // ����ʵ���ҵ�ͼ���������
    if (prevState->mapType == Map::LAB && !cabinetMonsterDefeated) {
        bool hasEnemy = false;
        for (auto entity : currentMap->getEntities()) {
            if (dynamic_cast<Enemy*>(entity) && !dynamic_cast<Enemy*>(entity)->isChaserType()) {
                hasEnemy = true;
                break;
            }
        }
        if (!hasEnemy) {
            Enemy* labEnemy = new Enemy(30, 10, "ʵ�����");
            currentMap->addEntity(labEnemy);
        }
    }

    currentMapIdx = (prevState->mapType == Map::ARCHIVE_ROOM) ? 0 : 1;

    std::string mapName = (prevState->mapType == Map::ARCHIVE_ROOM) ? "������" : "����ʵ����";
    std::vector<std::string> clues = { "[�����л�] ����" + mapName + "��" };
    Display::showCluePage("������ʾ", clues);
    needRender = true;

    delete prevState;
}

bool Game::fight(Enemy* enemy) {
    if (enemy == nullptr || player == nullptr) return false;
    Display::clearFightLogs();
    bool fightEnd = false;
    bool playerEscape = false;

    Display::addFightLog("����" + enemy->getName() + "��ս����ʼ��", Display::YELLOW);
    if (enemy->isChaserType()) {
        Display::addFightLog("�����˼粿չ�����ⷢ�������������̵ĳ������", Display::RED);
    }
    else {
        Display::addFightLog("ʵ�����������ã����ճ��Ĵ�������Ϯ����", Display::RED);
    }

    srand(static_cast<unsigned int>(time(0)));
    while (!fightEnd) {
        Display::renderFightUI(*player, *enemy);
        char key = _getch();
        switch (toupper(key)) {
        case '1': attackAction(enemy); break;
        case '2': dodgeAction(enemy); break;
        case 'Q': playerEscape = escapeAction(enemy); fightEnd = true; break;
        default: Display::addFightLog("��Ч�������밴1-������2-������Q-���ܣ�", Display::RED); continue;
        }

        if (player->getHp() <= 0) {
            Display::addFightLog("�㱻" + enemy->getName() + "�����ˣ�", Display::RED);
            fightEnd = true;
        }
        else if (enemy->isDead()) {
            Display::addFightLog("��ɹ�������" + enemy->getName() + "��", Display::GREEN);
            enemy->setAlive(false);
            fightEnd = true;
        }
    }

    Display::renderFightUI(*player, *enemy);
    Sleep(2000);
    Display::clearFightLogs();

    if (player->getHp() <= 0) {
        Display::printGameOver(false);
        isRunning = false;
        return false;
    }
    else if (enemy->isDead()) {
        if (enemy == archiveRobot) dropWeaponAtRobotPos();
        return true;
    }
    else if (playerEscape) {
        std::vector<std::string> clues = { "[ս�����] �ɹ�����" + enemy->getName() + "�Ĺ�����Χ��" };
        Display::showCluePage("ս�����", clues);
        return true;
    }
    return false;
}

void Game::attackAction(Enemy* enemy) {
    std::string weapon = player->getAttack() > 10 ? "��ͭ�˹�" : "ȭͷ";
    Display::addFightLog("�����" + weapon + "��" + enemy->getName() + "�ͻ���", Display::GREEN);

    if (rand() % 100 < enemy->getAggression()) {
        int playerDmg = enemy->getAttack();
        int enemyDmg = player->getAttack();
        player->reduceHp(playerDmg);
        enemy->reduceHp(enemyDmg);

        if (enemy->isChaserType()) {
            Display::addFightLog("�����˲����ܣ�ͬʱ���伤�����������㣡", Display::RED);
        }
        else {
            Display::addFightLog("ʵ������ô��ֲ�ס����ֱۣ��ݺ�˦����棡", Display::RED);
        }
        Display::addFightLog("�����" + std::to_string(enemyDmg) + "���˺����ܵ�" + std::to_string(playerDmg) + "���˺���", Display::YELLOW);
    }
    else {
        int dodgeRate = rand() % 3 + 1;
        int enemyDmg = player->getAttack() / dodgeRate;
        enemy->reduceHp(enemyDmg);

        if (enemy->isChaserType()) {
            Display::addFightLog("�����˿�����󻬲�����Ĺ���ֻ������������ǣ�", Display::RED);
        }
        else {
            Display::addFightLog("ʵ�����������һ�ţ���Ĺ�����������ۿۣ�", Display::RED);
        }
        Display::addFightLog("���˶����ɹ���������" + std::to_string(enemyDmg) + "���˺���", Display::YELLOW);
    }
}

void Game::dodgeAction(Enemy* enemy) {
    Display::addFightLog("��Ѹ����෭�������Զ���������", Display::GREEN);

    if (rand() % 100 < enemy->getAggression()) {
        int dodgeRate = rand() % 3 + 1;
        int playerDmg = enemy->getAttack() / dodgeRate;
        player->reduceHp(playerDmg);

        if (enemy->isChaserType()) {
            Display::addFightLog("������Ԥ������Ĺ켣��������������ļ��", Display::RED);
        }
        else {
            Display::addFightLog("ʵ�����Ĵ��ֺ�ɨ����������Ȼ�㿪��Ҫ�������Ǳ����ˣ�", Display::RED);
        }
        Display::addFightLog("��������" + std::to_string(100 - 100 / dodgeRate) + "%���ܵ�" + std::to_string(playerDmg) + "���˺���", Display::YELLOW);
    }
    else {
        if (enemy->isChaserType()) {
            Display::addFightLog("�����˵ļ�����е��棬����һ���𻨣�������������", Display::GREEN);
        }
        else {
            Display::addFightLog("ʵ��������˸��գ�һͷײ��ǽ�ϣ���ʱ����ѣ�Σ�", Display::GREEN);
        }
    }
}

bool Game::escapeAction(Enemy* enemy) {
    Display::addFightLog("��ת������ڿ񱼣���ͼ���ܣ�", Display::GREEN);
    if (rand() % 100 < 50) {
        Display::addFightLog("��ɹ�˦����" + enemy->getName() + "������˸����Ľ��䣡", Display::GREEN);
        return true;
    }
    else {
        int playerDmg = enemy->getAttack() / 2;
        player->reduceHp(playerDmg);
        if (enemy->isChaserType()) {
            Display::addFightLog("�����˵ļ����������ĺ󱳣�����ʧ�ܣ�", Display::RED);
        }
        else {
            Display::addFightLog("ʵ������ס����Ľ��ף��������˻�����", Display::RED);
        }
        Display::addFightLog("�ܵ�" + std::to_string(playerDmg) + "���˺������ȼ���ս����", Display::YELLOW);
        return false;
    }
}

void Game::interactWithDoor() {
    int px = player->getX();
    int py = player->getY();
    int cell = currentMap->getCell(px, py);
    std::vector<std::string> clues;

    if (cell == TILE_DOOR) {
        if (backpack->hasKey()) {
            clues = {
                "[��] ��⵽��ͭԿ�ף�",
                "��������Կ�׵����ּ���1-5������",
                "",
                "��ʾ�����ź�����ǰ������ʵ���ң�ȷ��׼��������"
            };
        }
        else {
            clues = {
                "[��] �������ŵģ���Ҫ��ͭԿ�ײ��ܴ�",
                "",
                "��������ܣ�S�����ƺ���Կ�׵��ټ�"
            };
        }
    }
    else if (cell == TILE_SAFE_DOOR) {
        if (backpack->hasKey()) {
            clues = {
                "[��ȫ��] ��⵽�ſ���",
                "���������ſ������ּ���1-5��������֤",
                "ע�⣺��֤��Ҫ�������ſ�+ͼֽ����",
                "��������ͼֽ��Ƭ�У��ǵ���ǰ�鿴����"
            };
        }
        else {
            clues = {
                "[��ȫ��] ��Ҫ�ſ�+ͼֽ����˫����֤���ܿ�����",
                "",
                "����1����Ӧ̨��R���ϳɽⶾ���ɻ���ſ�",
                "����2��������������ɻ��ͼֽ��Ƭ�������룩"
            };
        }
    }
    else if (cell == TILE_BACK_DOOR) {
        clues = {
            "[������] �ɷ��ص�����",
            "",
            "���ո��ȷ�Ϸ���"
        };
        Display::showCluePage("������ʾ", clues);
        returnToPreviousMap();
        return;
    }

    Display::showCluePage("�Ž�����ʾ", clues);
    needRender = true;
}

void Game::checkInteraction() {
    if (player == nullptr || currentMap == nullptr || backpack == nullptr) return;
    int px = player->getX();
    int py = player->getY();
    int cell = currentMap->getCell(px, py);
    bool interacted = false;

    // ʰȡ��Ʒ - ����Ƿ��ѻ�ȡ
    for (auto entity : currentMap->getEntities()) {
        if (Object* obj = dynamic_cast<Object*>(entity)) {
            if (obj->isAlive()) {
                // ����ֻ�ܻ�ȡһ��
                if (obj->getObjectType() == Object::WEAPON && !weaponObtained &&
                    obj->getX() == px && obj->getY() == py) {
                    backpack->addItem(obj);
                    weaponObtained = true; // ���Ϊ�ѻ�ȡ
                    std::vector<std::string> clues = { "[ʰȡ] ��á���ͭ�˹�����", "�����ּ�װ����������+15" };
                    Display::showCluePage("��Ʒʰȡ", clues);
                    interacted = true;
                    needRender = true;
                }
                // Կ��ֻ�ܻ�ȡһ��
                else if (obj->getObjectType() == Object::KEY && !keyObtained &&
                    obj->getX() == px && obj->getY() == py) {
                    backpack->addItem(obj);
                    keyObtained = true; // ���Ϊ�ѻ�ȡ
                    player->addKey();
                    std::vector<std::string> clues = { "[ʰȡ] ��á���ͭԿ�ס���", "�����ڴ򿪵�������" };
                    Display::showCluePage("��Ʒʰȡ", clues);
                    interacted = true;
                    needRender = true;
                }
            }
        }
    }

    // ��������
    if (archiveRobot && archiveRobot->isAlive() &&
        archiveRobot->getX() == px && archiveRobot->getY() == py) {
        fight(archiveRobot);
        interacted = true;
    }

    // �Ž���
    if (cell == TILE_DOOR || cell == TILE_SAFE_DOOR || cell == TILE_BACK_DOOR) {
        interactWithDoor();
        interacted = true;
    }

    // �����ҽ���
    if (currentMap->getMapType() == Map::ARCHIVE_ROOM) {
        if (cell == TILE_PHOTO_WALL) {
            std::vector<std::string> clues = {
                "����5�Ŵ��������ֵ�����Ƭ��",
                "1. VI����Ӧ����6��  2. IX����Ӧ����9��  3. XII����Ӧ����12��",
                "4. III����Ӧ����3��  5. VI����Ӧ����6��",
                "",
                "��ʾ����˳��������ֿ���������������루����691236��"
            };
            Display::showCluePage("��Ƭǽ����", clues);
            if (archiveRobot == nullptr) spawnArchiveRobot();
            interacted = true;
            needRender = true;
        }

        if (cell == TILE_SHELF && !keyObtained) { // Կ��δ��ȡʱ���ܽ���
            std::vector<std::string> clues = {
                "�㷭����1900��ľ��ļ��У��ҳ��������",
                "�ڵ�4��ġ�ʵ���¼���з�����һ����ͭԿ�ף�",
                "",
                "����ʰȡ..."
            };
            Display::showCluePage("���̽��", clues);
            Sleep(1000);

            // ʰȡԿ��
            for (auto entity : currentMap->getEntities()) {
                if (Object* obj = dynamic_cast<Object*>(entity)) {
                    if (obj->isAlive() && obj->getObjectType() == Object::KEY &&
                        obj->getX() == px && obj->getY() == py) {
                        backpack->addItem(obj);
                        player->addKey();
                        keyObtained = true; // ���Ϊ�ѻ�ȡ
                        std::vector<std::string> getClues = { "[���] ��ͭԿ��+1��", "�����ڴ򿪵�������" };
                        Display::showCluePage("��Ʒ��ȡ", getClues);
                    }
                }
            }
            if (archiveRobot == nullptr) spawnArchiveRobot();
            interacted = true;
            needRender = true;
        }
        else if (cell == TILE_SHELF && keyObtained) { // Կ���ѻ�ȡ
            std::vector<std::string> clues = {
                "���ٴμ����ܣ��������Ѿ�û���µķ�����",
                "��ͭԿ���Ѿ���ȡ�ߣ������ֻʣ��һЩ�޹صľ��ļ�"
            };
            Display::showCluePage("���̽��", clues);
            interacted = true;
            needRender = true;
        }

        if (cell == TILE_BOX) {
            std::vector<std::string> clues = {
                "�������������ּ��̣���ʾ��",
                "��������Ƭǽ�������ֶ�Ӧ��˳�����",
                "�����������Ƭ˳����VI��IX��������69��"
            };
            std::string input = Display::showInputPage("���������", clues, "����������");

            if (input == "691236") {
                // ����Ƿ��ѻ�ȡ����Ʒ
                if (!archiveLootCollected) {
                    Object* blueprint = new Object(Object::BLUEPRINT_FRAGMENT, px, py);
                    Object* potion = new Object(Object::HP_POTION, px, py);
                    currentMap->addEntity(blueprint);
                    currentMap->addEntity(potion);
                    backpack->addItem(blueprint);
                    backpack->addItem(potion);
                    archiveLootCollected = true;  // ���Ϊ�ѻ�ȡ
                    hpPotionObtained = true;      // ���ҩ��Ϊ�ѻ�ȡ
                    Display::showResultPage("������ȷ����á�ͼֽ��Ƭ���͡�ҩ�衹��", true);
                }
                else {
                    Display::showResultPage("������ȷ���������Ѿ�����...", true);
                }
            }
            else {
                Display::showResultPage("�����������ϸ������Ƭǽ��������", false);
            }
            if (archiveRobot == nullptr) spawnArchiveRobot();
            interacted = true;
            needRender = true;
        }
    }
    // ʵ���ҽ���
    else if (currentMap->getMapType() == Map::LAB) {
        if (cell == TILE_TRASH) {
            std::vector<std::string> clues = {
                "����һ�������ʵ���¼���ּ�ģ����",
                "����ɫ����������ӫ���Ƕ�Ӧ�Լ����˳��",
                "��R=��ɫ��y=��ɫ��b=��ɫ ���� ��ʿ����",
                "",
                "��ʾ���Ǻ���ɫ��Ӧ��ϵ�������ϳ���Ҫ"
            };
            Display::showCluePage("��ֽ¨����", clues);
            interacted = true;
            needRender = true;
        }

        if (cell == TILE_MICROSCOPE) {
            std::vector<std::string> clues = {
                "�۲첡������������ӫ�������ȼ���",
                "R=3����ɫ�Լ������ȼ�3��",
                "y=1����ɫ�Լ������ȼ�1��",
                "b=2����ɫ�Լ������ȼ�2��",
                "",
                "��ʾ�����˳��ӦΪ���ȼ�1��2��3���ơ������죩"
            };
            Display::showCluePage("��΢������", clues);
            interacted = true;
            needRender = true;
        }

        // ���ӽ��� - ������ع���
        if (cell == TILE_CABINET) {
            // �������Ƿ��ѱ�����
            if (!cabinetMonsterDefeated) {
                std::vector<std::string> clues = {
                    "����˱��С������������Ľ�����...",
                    "һ���ȳ�ζ���������������ƺ��ж������䶯��",
                    "",
                    "���棺������δ֪����������𹥻���"
                };
                Display::showCluePage("����̽��", clues);

                // �������ӹ��ﲢ����ս��
                Enemy* cabinetMonster = new Enemy(px, py, "����������");
                currentMap->addEntity(cabinetMonster);
                fight(cabinetMonster);

                // ս����������Ϊ�ѻ���
                cabinetMonsterDefeated = true;
                // �Ƴ�����ʵ��
                currentMap->removeEntity(cabinetMonster);
                delete cabinetMonster;
            }
            else {
                // �����ѱ����ܣ���ʾ��ͬ����
                std::vector<std::string> clues = {
                    "������տ���Ҳ��ֻʣ��һЩ����Ĳ�������",
                    "���������屻����������Ѿ�û���κ���в��"
                };
                Display::showCluePage("����̽��", clues);
            }
            interacted = true;
            needRender = true;
        }

        if (cell == TILE_REACT) {
            std::vector<std::string> clues = {
                "��Ӧ̨��Ҫ����ȷ˳���������Լ���",
                "����1����ֽ¨����R=�죬y=�ƣ�b=��",
                "����2����΢������˳��Ϊy��b��R��1��2��3��",
                "",
                "�������ʽ���ƺ��� / ybr / 123"
            };
            std::string seq = Display::showInputPage("��Ӧ̨�ϳ�", clues, "�������Լ����˳��");

            // �ſ�ֻ�ܻ�ȡһ��
            if ((seq == "�ƺ���" || seq == "ybr" || seq == "123" || seq == "������") && !doorCardObtained) {
                Object* doorCard = new Object(Object::DOOR_CARD, px, py);
                currentMap->addEntity(doorCard);
                backpack->addItem(doorCard);
                doorCardObtained = true;  // ����ſ��ѻ�ȡ
                Display::showResultPage("�ⶾ���ϳ���ϣ���á��ſ������ɴ򿪰�ȫ��", true);
            }
            else if (doorCardObtained) {
                Display::showResultPage("���Ѿ��ϳɹ��ſ��ˣ�����Ҫ���ظ��ϳɡ�", true);
            }
            else {
                player->reduceHp(15);
                Display::showResultPage("˳����󣡶���й©�Ӿ磬HP-15����ǰHP��" + std::to_string(player->getHp()), false);
            }
            interacted = true;
            needRender = true;
        }
    }

    if (interacted) needRender = true;
}

void Game::handleInput() {
    if (!_kbhit() || player == nullptr || currentMap == nullptr || backpack == nullptr) return;
    char key = _getch();
    bool moved = false;
    bool isOpenDoor = false;
    bool isWin = false;

    switch (toupper(key)) {
    case 'W': moved = movePlayer(0, -1); break;
    case 'S': moved = movePlayer(0, 1); break; // S�������������ƶ�
    case 'A': moved = movePlayer(-1, 0); break;
    case 'D': moved = movePlayer(1, 0); break;
    case ' ': checkInteraction(); break;
    case '1': if (backpack->useItem(0, *player, *currentMap, isOpenDoor, isWin)) needRender = true; break;
    case '2': if (backpack->useItem(1, *player, *currentMap, isOpenDoor, isWin)) needRender = true; break;
    case '3': if (backpack->useItem(2, *player, *currentMap, isOpenDoor, isWin)) needRender = true; break;
    case '4': if (backpack->useItem(3, *player, *currentMap, isOpenDoor, isWin)) needRender = true; break;
    case '5': if (backpack->useItem(4, *player, *currentMap, isOpenDoor, isWin)) needRender = true; break;
    case 'L': {
        // ��������
        int slot = Display::showSaveSlotSelection(true);
        if (slot > 0) {
            if (loadGame(slot)) {
                Display::showLoadSuccess();
            }
            else {
                Display::showSaveError("�޷����ش浵��浵������");
            }
        }
        break;
    }
    case 0x3F: { // F5����ɨ����
        // �浵����
        int slot = Display::showSaveSlotSelection(false);
        if (slot > 0) {
            if (saveGame(slot)) {
                Display::showSaveSuccess();
            }
            else {
                Display::showSaveError("�޷�������Ϸ");
            }
        }
        break;
    }
    case 'Q': isRunning = false; break;
    }

    if (isOpenDoor) {
        currentMap->setCell(player->getX(), player->getY(), TILE_EMPTY);
        switchMap(Map::LAB);
    }

    if (isWin) {
        Display::printGameOver(true);
        isRunning = false;
    }

    if (moved) needRender = true;
}

void Game::updateRobotMovement() {
    if (!archiveRobot || !archiveRobot->isAlive() || currentMap->getMapType() != Map::ARCHIVE_ROOM) return;

    clock_t currentTime = clock();
    if ((currentTime - lastRobotMoveTime) > 500) {
        archiveRobot->chasePlayer(*player, *currentMap);
        lastRobotMoveTime = currentTime;
        needRender = true;
    }
}

bool Game::movePlayer(int dx, int dy) {
    if (player->canMove(dx, dy, *currentMap)) {
        player->setPos(player->getX() + dx, player->getY() + dy);
        checkInteraction();
        return true;
    }
    return false;
}

void Game::renderNow() {
    if (player != nullptr && currentMap != nullptr && backpack != nullptr) {
        Display::renderAll(*currentMap, *player, *backpack, archiveRobot);
        needRender = false;
    }
}

void Game::run() {
    while (isRunning) {
        handleInput();
        updateRobotMovement();
        if (needRender) {
            renderNow();
        }
        if (player != nullptr && player->getHp() <= 0) {
            Display::printGameOver(false);
            isRunning = false;
        }
        Sleep(50);
    }
}

// �浵��ط���ʵ��
bool Game::saveGame(int slot) {
    try {
        ensureSaveDirectoryExists();
        std::string baseFilename = SAVE_DIRECTORY + "save" + std::to_string(slot);

        // ���浱ǰ��ͼ
        if (!currentMap->saveToFile(baseFilename + "_map" + SAVE_FILE_EXTENSION)) {
            return false;
        }

        // �������״̬
        std::ofstream playerFile(baseFilename + "_player" + SAVE_FILE_EXTENSION);
        if (!playerFile.is_open()) return false;
        player->save(playerFile);
        playerFile.close();

        // ���汳��
        std::ofstream backpackFile(baseFilename + "_backpack" + SAVE_FILE_EXTENSION);
        if (!backpackFile.is_open()) return false;
        backpack->save(backpackFile);
        backpackFile.close();

        // �����ͼ��ʷ����Ʒ��ȡ״̬
        std::ofstream historyFile(baseFilename + "_history" + SAVE_FILE_EXTENSION);
        if (!historyFile.is_open()) return false;

        // ������ʷ��¼����
        historyFile << mapHistory->size() << std::endl;

        // Ϊ�˱���ջ��������Ҫ�Ƚ���ת��Ϊ��ʱ����
        std::vector<MapState*> tempHistory;
        while (!mapHistory->empty()) {
            tempHistory.push_back(mapHistory->top());
            mapHistory->pop();
        }

        // ������ʷ��¼
        for (auto it = tempHistory.rbegin(); it != tempHistory.rend(); ++it) {
            (*it)->save(historyFile);
        }

        // �ָ�ջ
        for (auto it = tempHistory.rbegin(); it != tempHistory.rend(); ++it) {
            mapHistory->push(*it);
        }

        // ���Ᵽ�浱ǰ��Ϸ״̬�е���Ʒ��ȡ���
        historyFile << archiveLootCollected << std::endl;
        historyFile << doorCardObtained << std::endl;
        historyFile << weaponObtained << std::endl;
        historyFile << keyObtained << std::endl;
        historyFile << hpPotionObtained << std::endl;
        historyFile << cabinetMonsterDefeated << std::endl;

        historyFile.close();
        return true;
    }
    catch (...) {
        return false;
    }
}

bool Game::loadGame(int slot) {
    try {
        std::string baseFilename = SAVE_DIRECTORY + "save" + std::to_string(slot);

        // ���浵�ļ��Ƿ����
        std::ifstream testFile(baseFilename + "_map" + SAVE_FILE_EXTENSION);
        if (!testFile.is_open()) return false;
        testFile.close();

        // ����ǰ��Ϸ״̬
        delete currentMap;
        delete player;
        delete backpack;
        if (archiveRobot) delete archiveRobot;

        // ��յ�ͼ��ʷ
        while (!mapHistory->empty()) {
            MapState* state = mapHistory->top();
            mapHistory->pop();
            delete state;
        }

        // ���ص�ͼ
        currentMap = new Map(Map::ARCHIVE_ROOM);
        if (!currentMap->loadFromFile(baseFilename + "_map" + SAVE_FILE_EXTENSION)) {
            return false;
        }

        // ���Ҳ���ȡ���
        player = nullptr;
        for (auto entity : currentMap->getEntities()) {
            if (Player* p = dynamic_cast<Player*>(entity)) {
                player = p;
                break;
            }
        }

        if (!player) {
            return false;
        }

        // ���Ҳ���ȡ�����˺�ʵ���ҵ���
        archiveRobot = nullptr;
        std::vector<Enemy*> labEnemies; // �洢ʵ���ҵ���

        for (auto entity : currentMap->getEntities()) {
            if (Enemy* e = dynamic_cast<Enemy*>(entity)) {
                if (e->isChaserType()) {
                    archiveRobot = e; // �����һ�����
                }
                else {
                    labEnemies.push_back(e); // ʵ������ͨ����
                }
            }
        }

        // ʵ���ҵ�ͼ���⴦�����û����ͨ�����򴴽�һ��
        if (currentMap->getMapType() == Map::LAB && labEnemies.empty()) {
            Enemy* labEnemy = new Enemy(30, 10, "ʵ�����");
            currentMap->addEntity(labEnemy);
        }

        // ���ر���
        backpack = new Backpack();
        std::ifstream backpackFile(baseFilename + "_backpack" + SAVE_FILE_EXTENSION);
        if (!backpackFile.is_open()) return false;
        backpack->load(backpackFile);
        backpackFile.close();

        // ���ص�ͼ��ʷ����Ʒ��ȡ״̬
        std::ifstream historyFile(baseFilename + "_history" + SAVE_FILE_EXTENSION);
        if (!historyFile.is_open()) return false;

        size_t historySize;
        historyFile >> historySize;

        for (size_t i = 0; i < historySize; ++i) {
            MapState* state = new MapState(Map::ARCHIVE_ROOM, nullptr, nullptr, {}, false, false, false, false, false);
            state->load(historyFile);
            mapHistory->push(state);
        }

        // ������Ʒ��ȡ״̬
        historyFile >> archiveLootCollected;
        historyFile >> doorCardObtained;
        historyFile >> weaponObtained;
        historyFile >> keyObtained;
        historyFile >> hpPotionObtained;
        historyFile >> cabinetMonsterDefeated;

        historyFile.close();

        // ���µ�ǰ��ͼ����
        currentMapIdx = (currentMap->getMapType() == Map::ARCHIVE_ROOM) ? 0 : 1;

        // �Ƴ��ѻ�ȡ����Ʒʵ��
        std::vector<Thing*> entitiesToRemove;
        for (auto entity : currentMap->getEntities()) {
            if (Object* obj = dynamic_cast<Object*>(entity)) {
                if (obj->isAlive()) {
                    if ((obj->getObjectType() == Object::KEY && keyObtained) ||
                        (obj->getObjectType() == Object::WEAPON && weaponObtained) ||
                        (obj->getObjectType() == Object::HP_POTION && hpPotionObtained) ||
                        (obj->getObjectType() == Object::BLUEPRINT_FRAGMENT && archiveLootCollected) ||
                        (obj->getObjectType() == Object::DOOR_CARD && doorCardObtained)) {
                        entitiesToRemove.push_back(entity);
                    }
                }
            }
        }

        // �ӵ�ͼ���Ƴ��ѻ�ȡ����Ʒ
        for (auto entity : entitiesToRemove) {
            currentMap->removeEntity(entity);
            delete entity;
        }

        needRender = true;
        return true;
    }
    catch (...) {
        return false;
    }
}
