#ifndef GAME_H
#define GAME_H

#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Backpack.h"
#include "Display.h"
#include "MapState.h"
#include <stack>
#include <ctime>

class Game {
private:
    Player* player;
    Map* currentMap;
    Backpack* backpack;
    Enemy* archiveRobot;
    bool isRunning;
    int currentMapIdx;
    bool needRender;
    clock_t lastRobotMoveTime;
    std::stack<MapState*>* mapHistory;

    // 物品获取状态 - 全部设置为只能获取一次
    bool archiveLootCollected;  // 档案室图纸和药丸
    bool doorCardObtained;      // 门卡
    bool weaponObtained;        // 武器(黄铜撬棍)
    bool keyObtained;           // 青铜钥匙
    bool hpPotionObtained;      // 药丸
    bool cabinetMonsterDefeated; // 柜子怪物是否已被击败

    std::vector<Thing*> copyEntitiesExceptPlayerAndRobot(const std::vector<Thing*>& original, Player* player, Enemy* robot);

    // 辅助函数：确保存档目录存在
    void ensureSaveDirectoryExists() const;

public:
    Game();
    ~Game();
    void spawnArchiveRobot();
    void dropWeaponAtRobotPos();
    void switchMap(Map::MapType targetType);
    void returnToPreviousMap();
    bool fight(Enemy* enemy);
    void attackAction(Enemy* enemy);
    void dodgeAction(Enemy* enemy);
    bool escapeAction(Enemy* enemy);
    void interactWithDoor();
    void checkInteraction();
    void handleInput();
    void updateRobotMovement();
    bool movePlayer(int dx, int dy);
    void renderNow();
    void run();

    // 存档相关方法
    bool saveGame(int slot);
    bool loadGame(int slot);
};

#endif // GAME_H
