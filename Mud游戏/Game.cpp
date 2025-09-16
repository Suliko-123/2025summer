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
    // 初始化所有物品为未获取状态
    archiveLootCollected(false), doorCardObtained(false),
    weaponObtained(false), keyObtained(false), hpPotionObtained(false),
    cabinetMonsterDefeated(false) {
    backpack = new Backpack();
    player = new Player(1, 1, "冒险者");
    currentMap = new Map(Map::ARCHIVE_ROOM);
    currentMap->addEntity(player);
    // 初始添加钥匙，但只在未获取状态下
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

    archiveRobot = new Enemy(x, y, "档案室机器人", true);
    currentMap->addEntity(archiveRobot);
    std::vector<std::string> clues = { "[警告] 档案室机器人被激活！", "红色光点锁定了你，开始追逐！" };
    Display::showCluePage("危险警告", clues);
    needRender = true;
}

void Game::dropWeaponAtRobotPos() {
    // 如果武器已被获取，则不再生成
    if (weaponObtained || !archiveRobot || archiveRobot->isAlive()) return;

    Object* weapon = new Object(Object::WEAPON, archiveRobot->getX(), archiveRobot->getY());
    currentMap->addEntity(weapon);
    std::vector<std::string> clues = { "[战利品] 击败机器人！", "掉落了「黄铜撬棍」，靠近即可拾取！" };
    Display::showCluePage("战斗奖励", clues);
    needRender = true;
}

void Game::switchMap(Map::MapType targetType) {
    if (currentMap == nullptr || player == nullptr) return;

    std::vector<Thing*> copiedEntities = copyEntitiesExceptPlayerAndRobot(currentMap->getEntities(), player, archiveRobot);

    // 保存当前状态时包含所有物品获取状态
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
        // 如果钥匙未被获取，才添加钥匙实体
        if (!keyObtained) {
            currentMap->addEntity(new Object(Object::KEY, 60, 15));
        }
    }
    else if (targetType == Map::LAB) {
        currentMapIdx = 1;
        // 只在没有击败柜子怪物的情况下才添加实验室敌人
        if (!cabinetMonsterDefeated) {
            Enemy* labEnemy = new Enemy(30, 10, "实验怪物");
            currentMap->addEntity(labEnemy);
        }
        player->setPos(1, 2);
    }

    std::string mapName = (targetType == Map::ARCHIVE_ROOM) ? "档案室" : "生化实验室";
    std::vector<std::string> clues = { "[场景切换] 进入" + mapName + "！" };
    if (targetType == Map::LAB) {
        clues.push_back("空气中弥漫着绿色毒气，病毒正在泄漏！");
        clues.push_back("(B标记处为返回档案室的门)");
    }
    else {
        clues.push_back("回到了档案室，小心机器人！");
    }
    Display::showCluePage("场景提示", clues);
    needRender = true;
}

void Game::returnToPreviousMap() {
    if (mapHistory->empty()) {
        std::vector<std::string> clues = { "[提示] 无法返回，这是初始地图！" };
        Display::showCluePage("地图提示", clues);
        return;
    }

    MapState* prevState = mapHistory->top();
    mapHistory->pop();

    // 恢复所有物品获取状态
    archiveLootCollected = prevState->archiveLootCollected;
    doorCardObtained = prevState->doorCardObtained;
    weaponObtained = prevState->weaponObtained;
    keyObtained = prevState->keyObtained;
    hpPotionObtained = prevState->hpPotionObtained;
    cabinetMonsterDefeated = prevState->cabinetMonsterDefeated;

    Enemy* currentRobot = (currentMap->getMapType() == Map::ARCHIVE_ROOM) ? archiveRobot : nullptr;

    // 清除当前地图的实体
    std::vector<Thing*> entitiesToRemove = currentMap->getEntities();
    for (auto entity : entitiesToRemove) {
        currentMap->removeEntity(entity);
        if (entity != player && entity != currentRobot) {
            delete entity;
        }
    }

    // 保存当前机器人指针（如果有）
    Enemy* tempRobot = archiveRobot;
    if (currentMap->getMapType() != Map::ARCHIVE_ROOM) {
        archiveRobot = nullptr;
    }

    // 删除当前地图
    delete currentMap;

    // 创建新地图并恢复状态
    currentMap = new Map(prevState->mapType);
    currentMap->clearEntities();

    // 恢复玩家位置
    player->setPos(prevState->playerX, prevState->playerY);
    currentMap->addEntity(player);

    // 恢复实体
    for (auto entity : *prevState->entities) {
        currentMap->addEntity(entity);
    }

    // 恢复机器人
    if (prevState->archiveRobot) {
        archiveRobot = prevState->archiveRobot;
        if (archiveRobot->isAlive()) {
            currentMap->addEntity(archiveRobot);
        }
    }
    else {
        archiveRobot = nullptr;
    }

    // 处理实验室地图的特殊情况
    if (prevState->mapType == Map::LAB && !cabinetMonsterDefeated) {
        bool hasEnemy = false;
        for (auto entity : currentMap->getEntities()) {
            if (dynamic_cast<Enemy*>(entity) && !dynamic_cast<Enemy*>(entity)->isChaserType()) {
                hasEnemy = true;
                break;
            }
        }
        if (!hasEnemy) {
            Enemy* labEnemy = new Enemy(30, 10, "实验怪物");
            currentMap->addEntity(labEnemy);
        }
    }

    currentMapIdx = (prevState->mapType == Map::ARCHIVE_ROOM) ? 0 : 1;

    std::string mapName = (prevState->mapType == Map::ARCHIVE_ROOM) ? "档案室" : "生化实验室";
    std::vector<std::string> clues = { "[场景切换] 返回" + mapName + "！" };
    Display::showCluePage("场景提示", clues);
    needRender = true;

    delete prevState;
}

bool Game::fight(Enemy* enemy) {
    if (enemy == nullptr || player == nullptr) return false;
    Display::clearFightLogs();
    bool fightEnd = false;
    bool playerEscape = false;

    Display::addFightLog("遭遇" + enemy->getName() + "！战斗开始！", Display::YELLOW);
    if (enemy->isChaserType()) {
        Display::addFightLog("机器人肩部展开激光发射器，发出滋滋的充电声！", Display::RED);
    }
    else {
        Display::addFightLog("实验怪物浑身溃烂，伸出粘稠的触手向你袭来！", Display::RED);
    }

    srand(static_cast<unsigned int>(time(0)));
    while (!fightEnd) {
        Display::renderFightUI(*player, *enemy);
        char key = _getch();
        switch (toupper(key)) {
        case '1': attackAction(enemy); break;
        case '2': dodgeAction(enemy); break;
        case 'Q': playerEscape = escapeAction(enemy); fightEnd = true; break;
        default: Display::addFightLog("无效操作！请按1-攻击、2-躲闪、Q-逃跑！", Display::RED); continue;
        }

        if (player->getHp() <= 0) {
            Display::addFightLog("你被" + enemy->getName() + "击败了！", Display::RED);
            fightEnd = true;
        }
        else if (enemy->isDead()) {
            Display::addFightLog("你成功击败了" + enemy->getName() + "！", Display::GREEN);
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
        std::vector<std::string> clues = { "[战斗结果] 成功逃离" + enemy->getName() + "的攻击范围！" };
        Display::showCluePage("战斗结果", clues);
        return true;
    }
    return false;
}

void Game::attackAction(Enemy* enemy) {
    std::string weapon = player->getAttack() > 10 ? "黄铜撬棍" : "拳头";
    Display::addFightLog("你挥舞" + weapon + "向" + enemy->getName() + "猛击！", Display::GREEN);

    if (rand() % 100 < enemy->getAggression()) {
        int playerDmg = enemy->getAttack();
        int enemyDmg = player->getAttack();
        player->reduceHp(playerDmg);
        enemy->reduceHp(enemyDmg);

        if (enemy->isChaserType()) {
            Display::addFightLog("机器人侧身躲避，同时发射激光束击中了你！", Display::RED);
        }
        else {
            Display::addFightLog("实验怪物用触手缠住你的手臂，狠狠甩向地面！", Display::RED);
        }
        Display::addFightLog("你造成" + std::to_string(enemyDmg) + "点伤害，受到" + std::to_string(playerDmg) + "点伤害！", Display::YELLOW);
    }
    else {
        int dodgeRate = rand() % 3 + 1;
        int enemyDmg = player->getAttack() / dodgeRate;
        enemy->reduceHp(enemyDmg);

        if (enemy->isChaserType()) {
            Display::addFightLog("机器人快速向后滑步，你的攻击只擦到了它的外壳！", Display::RED);
        }
        else {
            Display::addFightLog("实验怪物蜷缩成一团，你的攻击威力大打折扣！", Display::RED);
        }
        Display::addFightLog("敌人躲闪成功，你仅造成" + std::to_string(enemyDmg) + "点伤害！", Display::YELLOW);
    }
}

void Game::dodgeAction(Enemy* enemy) {
    Display::addFightLog("你迅速向侧翻滚，尝试躲闪攻击！", Display::GREEN);

    if (rand() % 100 < enemy->getAggression()) {
        int dodgeRate = rand() % 3 + 1;
        int playerDmg = enemy->getAttack() / dodgeRate;
        player->reduceHp(playerDmg);

        if (enemy->isChaserType()) {
            Display::addFightLog("机器人预判了你的轨迹，激光束擦过你的肩膀！", Display::RED);
        }
        else {
            Display::addFightLog("实验怪物的触手横扫过来，你虽然躲开了要害但还是被划伤！", Display::RED);
        }
        Display::addFightLog("躲闪减伤" + std::to_string(100 - 100 / dodgeRate) + "%，受到" + std::to_string(playerDmg) + "点伤害！", Display::YELLOW);
    }
    else {
        if (enemy->isChaserType()) {
            Display::addFightLog("机器人的激光击中地面，溅起一串火花！你完美躲闪！", Display::GREEN);
        }
        else {
            Display::addFightLog("实验怪物扑了个空，一头撞在墙上，暂时陷入眩晕！", Display::GREEN);
        }
    }
}

bool Game::escapeAction(Enemy* enemy) {
    Display::addFightLog("你转身向出口狂奔，试图逃跑！", Display::GREEN);
    if (rand() % 100 < 50) {
        Display::addFightLog("你成功甩开了" + enemy->getName() + "，躲进了附近的角落！", Display::GREEN);
        return true;
    }
    else {
        int playerDmg = enemy->getAttack() / 2;
        player->reduceHp(playerDmg);
        if (enemy->isChaserType()) {
            Display::addFightLog("机器人的激光击中了你的后背，逃跑失败！", Display::RED);
        }
        else {
            Display::addFightLog("实验怪物缠住了你的脚踝，把你拉了回来！", Display::RED);
        }
        Display::addFightLog("受到" + std::to_string(playerDmg) + "点伤害，被迫继续战斗！", Display::YELLOW);
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
                "[门] 检测到青铜钥匙！",
                "按背包中钥匙的数字键（1-5）开门",
                "",
                "提示：开门后将立即前往生化实验室，确保准备就绪！"
            };
        }
        else {
            clues = {
                "[门] 门是锁着的！需要青铜钥匙才能打开",
                "",
                "线索：书架（S）上似乎有钥匙的踪迹"
            };
        }
    }
    else if (cell == TILE_SAFE_DOOR) {
        if (backpack->hasKey()) {
            clues = {
                "[安全门] 检测到门卡！",
                "按背包中门卡的数字键（1-5）启动验证",
                "注意：验证需要两步：门卡+图纸密码",
                "（密码在图纸碎片中，记得提前查看！）"
            };
        }
        else {
            clues = {
                "[安全门] 需要门卡+图纸密码双重验证才能开启！",
                "",
                "线索1：反应台（R）合成解毒剂可获得门卡",
                "线索2：档案室密码箱可获得图纸碎片（含密码）"
            };
        }
    }
    else if (cell == TILE_BACK_DOOR) {
        clues = {
            "[返回门] 可返回档案室",
            "",
            "按空格键确认返回"
        };
        Display::showCluePage("返回提示", clues);
        returnToPreviousMap();
        return;
    }

    Display::showCluePage("门交互提示", clues);
    needRender = true;
}

void Game::checkInteraction() {
    if (player == nullptr || currentMap == nullptr || backpack == nullptr) return;
    int px = player->getX();
    int py = player->getY();
    int cell = currentMap->getCell(px, py);
    bool interacted = false;

    // 拾取物品 - 检查是否已获取
    for (auto entity : currentMap->getEntities()) {
        if (Object* obj = dynamic_cast<Object*>(entity)) {
            if (obj->isAlive()) {
                // 武器只能获取一次
                if (obj->getObjectType() == Object::WEAPON && !weaponObtained &&
                    obj->getX() == px && obj->getY() == py) {
                    backpack->addItem(obj);
                    weaponObtained = true; // 标记为已获取
                    std::vector<std::string> clues = { "[拾取] 获得「黄铜撬棍」！", "按数字键装备，攻击力+15" };
                    Display::showCluePage("物品拾取", clues);
                    interacted = true;
                    needRender = true;
                }
                // 钥匙只能获取一次
                else if (obj->getObjectType() == Object::KEY && !keyObtained &&
                    obj->getX() == px && obj->getY() == py) {
                    backpack->addItem(obj);
                    keyObtained = true; // 标记为已获取
                    player->addKey();
                    std::vector<std::string> clues = { "[拾取] 获得「青铜钥匙」！", "可用于打开档案室门" };
                    Display::showCluePage("物品拾取", clues);
                    interacted = true;
                    needRender = true;
                }
            }
        }
    }

    // 遭遇敌人
    if (archiveRobot && archiveRobot->isAlive() &&
        archiveRobot->getX() == px && archiveRobot->getY() == py) {
        fight(archiveRobot);
        interacted = true;
    }

    // 门交互
    if (cell == TILE_DOOR || cell == TILE_SAFE_DOOR || cell == TILE_BACK_DOOR) {
        interactWithDoor();
        interacted = true;
    }

    // 档案室交互
    if (currentMap->getMapType() == Map::ARCHIVE_ROOM) {
        if (cell == TILE_PHOTO_WALL) {
            std::vector<std::string> clues = {
                "发现5张带罗马数字的老照片：",
                "1. VI（对应数字6）  2. IX（对应数字9）  3. XII（对应数字12）",
                "4. III（对应数字3）  5. VI（对应数字6）",
                "",
                "提示：按顺序组合数字可能是密码箱的密码（例：691236）"
            };
            Display::showCluePage("照片墙线索", clues);
            if (archiveRobot == nullptr) spawnArchiveRobot();
            interacted = true;
            needRender = true;
        }

        if (cell == TILE_SHELF && !keyObtained) { // 钥匙未获取时才能交互
            std::vector<std::string> clues = {
                "你翻阅着1900年的旧文件夹，灰尘扑面而来",
                "在第4层的《实验记录》中发现了一把青铜钥匙！",
                "",
                "正在拾取..."
            };
            Display::showCluePage("书架探索", clues);
            Sleep(1000);

            // 拾取钥匙
            for (auto entity : currentMap->getEntities()) {
                if (Object* obj = dynamic_cast<Object*>(entity)) {
                    if (obj->isAlive() && obj->getObjectType() == Object::KEY &&
                        obj->getX() == px && obj->getY() == py) {
                        backpack->addItem(obj);
                        player->addKey();
                        keyObtained = true; // 标记为已获取
                        std::vector<std::string> getClues = { "[获得] 青铜钥匙+1！", "可用于打开档案室门" };
                        Display::showCluePage("物品获取", getClues);
                    }
                }
            }
            if (archiveRobot == nullptr) spawnArchiveRobot();
            interacted = true;
            needRender = true;
        }
        else if (cell == TILE_SHELF && keyObtained) { // 钥匙已获取
            std::vector<std::string> clues = {
                "你再次检查书架，但这里已经没有新的发现了",
                "青铜钥匙已经被取走，书架上只剩下一些无关的旧文件"
            };
            Display::showCluePage("书架探索", clues);
            interacted = true;
            needRender = true;
        }

        if (cell == TILE_BOX) {
            std::vector<std::string> clues = {
                "密码箱上有数字键盘，提示：",
                "请输入照片墙罗马数字对应的顺序组合",
                "（例：如果照片顺序是VI、IX，则输入69）"
            };
            std::string input = Display::showInputPage("密码箱解锁", clues, "请输入密码");

            if (input == "691236") {
                // 检查是否已获取过物品
                if (!archiveLootCollected) {
                    Object* blueprint = new Object(Object::BLUEPRINT_FRAGMENT, px, py);
                    Object* potion = new Object(Object::HP_POTION, px, py);
                    currentMap->addEntity(blueprint);
                    currentMap->addEntity(potion);
                    backpack->addItem(blueprint);
                    backpack->addItem(potion);
                    archiveLootCollected = true;  // 标记为已获取
                    hpPotionObtained = true;      // 标记药丸为已获取
                    Display::showResultPage("密码正确！获得「图纸碎片」和「药丸」！", true);
                }
                else {
                    Display::showResultPage("密码正确，但箱子已经空了...", true);
                }
            }
            else {
                Display::showResultPage("密码错误！再仔细看看照片墙的线索？", false);
            }
            if (archiveRobot == nullptr) spawnArchiveRobot();
            interacted = true;
            needRender = true;
        }
    }
    // 实验室交互
    else if (currentMap->getMapType() == Map::LAB) {
        if (cell == TILE_TRASH) {
            std::vector<std::string> clues = {
                "发现一张揉皱的实验记录，字迹模糊：",
                "「颜色决定生死，荧光标记对应试剂混合顺序」",
                "「R=红色，y=黄色，b=蓝色 —— 博士留」",
                "",
                "提示：记好颜色对应关系，后续合成需要"
            };
            Display::showCluePage("废纸篓线索", clues);
            interacted = true;
            needRender = true;
        }

        if (cell == TILE_MICROSCOPE) {
            std::vector<std::string> clues = {
                "观察病毒样本，发现荧光标记优先级：",
                "R=3（红色试剂，优先级3）",
                "y=1（黄色试剂，优先级1）",
                "b=2（蓝色试剂，优先级2）",
                "",
                "提示：混合顺序应为优先级1→2→3（黄→蓝→红）"
            };
            Display::showCluePage("显微镜分析", clues);
            interacted = true;
            needRender = true;
        }

        // 柜子交互 - 添加隐藏怪物
        if (cell == TILE_CABINET) {
            // 检查怪物是否已被击败
            if (!cabinetMonsterDefeated) {
                std::vector<std::string> clues = {
                    "你打开了标有「病毒样本」的金属柜...",
                    "一股腥臭味扑面而来，柜子深处似乎有东西在蠕动！",
                    "",
                    "警告：里面有未知生物！即将发起攻击！"
                };
                Display::showCluePage("柜子探索", clues);

                // 创建柜子怪物并触发战斗
                Enemy* cabinetMonster = new Enemy(px, py, "病毒变异体");
                currentMap->addEntity(cabinetMonster);
                fight(cabinetMonster);

                // 战斗结束后标记为已击败
                cabinetMonsterDefeated = true;
                // 移除怪物实体
                currentMap->removeEntity(cabinetMonster);
                delete cabinetMonster;
            }
            else {
                // 怪物已被击败，显示不同内容
                std::vector<std::string> clues = {
                    "柜子里空空如也，只剩下一些破碎的玻璃器皿",
                    "病毒变异体被消灭后，这里已经没有任何威胁了"
                };
                Display::showCluePage("柜子探索", clues);
            }
            interacted = true;
            needRender = true;
        }

        if (cell == TILE_REACT) {
            std::vector<std::string> clues = {
                "反应台需要按正确顺序混合三种试剂：",
                "线索1（废纸篓）：R=红，y=黄，b=蓝",
                "线索2（显微镜）：顺序为y→b→R（1→2→3）",
                "",
                "可输入格式：黄红蓝 / ybr / 123"
            };
            std::string seq = Display::showInputPage("反应台合成", clues, "请输入试剂混合顺序");

            // 门卡只能获取一次
            if ((seq == "黄红蓝" || seq == "ybr" || seq == "123" || seq == "黄蓝红") && !doorCardObtained) {
                Object* doorCard = new Object(Object::DOOR_CARD, px, py);
                currentMap->addEntity(doorCard);
                backpack->addItem(doorCard);
                doorCardObtained = true;  // 标记门卡已获取
                Display::showResultPage("解毒剂合成完毕！获得「门卡」，可打开安全门", true);
            }
            else if (doorCardObtained) {
                Display::showResultPage("你已经合成过门卡了，不需要再重复合成。", true);
            }
            else {
                player->reduceHp(15);
                Display::showResultPage("顺序错误！毒气泄漏加剧，HP-15！当前HP：" + std::to_string(player->getHp()), false);
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
    case 'S': moved = movePlayer(0, 1); break; // S键仅用于向下移动
    case 'A': moved = movePlayer(-1, 0); break;
    case 'D': moved = movePlayer(1, 0); break;
    case ' ': checkInteraction(); break;
    case '1': if (backpack->useItem(0, *player, *currentMap, isOpenDoor, isWin)) needRender = true; break;
    case '2': if (backpack->useItem(1, *player, *currentMap, isOpenDoor, isWin)) needRender = true; break;
    case '3': if (backpack->useItem(2, *player, *currentMap, isOpenDoor, isWin)) needRender = true; break;
    case '4': if (backpack->useItem(3, *player, *currentMap, isOpenDoor, isWin)) needRender = true; break;
    case '5': if (backpack->useItem(4, *player, *currentMap, isOpenDoor, isWin)) needRender = true; break;
    case 'L': {
        // 读档操作
        int slot = Display::showSaveSlotSelection(true);
        if (slot > 0) {
            if (loadGame(slot)) {
                Display::showLoadSuccess();
            }
            else {
                Display::showSaveError("无法加载存档或存档不存在");
            }
        }
        break;
    }
    case 0x3F: { // F5键的扫描码
        // 存档操作
        int slot = Display::showSaveSlotSelection(false);
        if (slot > 0) {
            if (saveGame(slot)) {
                Display::showSaveSuccess();
            }
            else {
                Display::showSaveError("无法保存游戏");
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

// 存档相关方法实现
bool Game::saveGame(int slot) {
    try {
        ensureSaveDirectoryExists();
        std::string baseFilename = SAVE_DIRECTORY + "save" + std::to_string(slot);

        // 保存当前地图
        if (!currentMap->saveToFile(baseFilename + "_map" + SAVE_FILE_EXTENSION)) {
            return false;
        }

        // 保存玩家状态
        std::ofstream playerFile(baseFilename + "_player" + SAVE_FILE_EXTENSION);
        if (!playerFile.is_open()) return false;
        player->save(playerFile);
        playerFile.close();

        // 保存背包
        std::ofstream backpackFile(baseFilename + "_backpack" + SAVE_FILE_EXTENSION);
        if (!backpackFile.is_open()) return false;
        backpack->save(backpackFile);
        backpackFile.close();

        // 保存地图历史和物品获取状态
        std::ofstream historyFile(baseFilename + "_history" + SAVE_FILE_EXTENSION);
        if (!historyFile.is_open()) return false;

        // 保存历史记录数量
        historyFile << mapHistory->size() << std::endl;

        // 为了保存栈，我们需要先将其转换为临时向量
        std::vector<MapState*> tempHistory;
        while (!mapHistory->empty()) {
            tempHistory.push_back(mapHistory->top());
            mapHistory->pop();
        }

        // 保存历史记录
        for (auto it = tempHistory.rbegin(); it != tempHistory.rend(); ++it) {
            (*it)->save(historyFile);
        }

        // 恢复栈
        for (auto it = tempHistory.rbegin(); it != tempHistory.rend(); ++it) {
            mapHistory->push(*it);
        }

        // 额外保存当前游戏状态中的物品获取标记
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

        // 检查存档文件是否存在
        std::ifstream testFile(baseFilename + "_map" + SAVE_FILE_EXTENSION);
        if (!testFile.is_open()) return false;
        testFile.close();

        // 清理当前游戏状态
        delete currentMap;
        delete player;
        delete backpack;
        if (archiveRobot) delete archiveRobot;

        // 清空地图历史
        while (!mapHistory->empty()) {
            MapState* state = mapHistory->top();
            mapHistory->pop();
            delete state;
        }

        // 加载地图
        currentMap = new Map(Map::ARCHIVE_ROOM);
        if (!currentMap->loadFromFile(baseFilename + "_map" + SAVE_FILE_EXTENSION)) {
            return false;
        }

        // 查找并提取玩家
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

        // 查找并提取机器人和实验室敌人
        archiveRobot = nullptr;
        std::vector<Enemy*> labEnemies; // 存储实验室敌人

        for (auto entity : currentMap->getEntities()) {
            if (Enemy* e = dynamic_cast<Enemy*>(entity)) {
                if (e->isChaserType()) {
                    archiveRobot = e; // 档案室机器人
                }
                else {
                    labEnemies.push_back(e); // 实验室普通敌人
                }
            }
        }

        // 实验室地图特殊处理：如果没有普通敌人则创建一个
        if (currentMap->getMapType() == Map::LAB && labEnemies.empty()) {
            Enemy* labEnemy = new Enemy(30, 10, "实验怪物");
            currentMap->addEntity(labEnemy);
        }

        // 加载背包
        backpack = new Backpack();
        std::ifstream backpackFile(baseFilename + "_backpack" + SAVE_FILE_EXTENSION);
        if (!backpackFile.is_open()) return false;
        backpack->load(backpackFile);
        backpackFile.close();

        // 加载地图历史和物品获取状态
        std::ifstream historyFile(baseFilename + "_history" + SAVE_FILE_EXTENSION);
        if (!historyFile.is_open()) return false;

        size_t historySize;
        historyFile >> historySize;

        for (size_t i = 0; i < historySize; ++i) {
            MapState* state = new MapState(Map::ARCHIVE_ROOM, nullptr, nullptr, {}, false, false, false, false, false);
            state->load(historyFile);
            mapHistory->push(state);
        }

        // 加载物品获取状态
        historyFile >> archiveLootCollected;
        historyFile >> doorCardObtained;
        historyFile >> weaponObtained;
        historyFile >> keyObtained;
        historyFile >> hpPotionObtained;
        historyFile >> cabinetMonsterDefeated;

        historyFile.close();

        // 更新当前地图索引
        currentMapIdx = (currentMap->getMapType() == Map::ARCHIVE_ROOM) ? 0 : 1;

        // 移除已获取的物品实体
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

        // 从地图中移除已获取的物品
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
