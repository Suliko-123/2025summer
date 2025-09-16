#ifndef MAPSTATE_H
#define MAPSTATE_H

#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include <vector>
#include <fstream>

// 地图状态结构体
struct MapState {
    Map::MapType mapType;
    int playerX;
    int playerY;
    Enemy* archiveRobot;
    std::vector<Thing*>* entities;

    // 物品获取状态 - 全部物品都只能获取一次
    bool archiveLootCollected;  // 记录是否已获取档案室物品
    bool doorCardObtained;      // 记录是否已获取门卡
    bool weaponObtained;        // 记录是否已获取武器
    bool keyObtained;           // 记录是否已获取青铜钥匙
    bool hpPotionObtained;      // 记录是否已获取药丸
    bool cabinetMonsterDefeated; // 柜子怪物是否已被击败

    MapState(Map::MapType type, Player* player, Enemy* robot, const std::vector<Thing*>& ents,
        bool lootCollected, bool cardObtained, bool weapon, bool key, bool potion, bool monsterDefeated = false);
    ~MapState();

    // 存档相关方法
    void save(std::ofstream& file) const;
    void load(std::ifstream& file);
};

#endif // MAPSTATE_H
