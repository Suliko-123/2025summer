#include "MapState.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Object.h"
#include <fstream>

// MapState实现
MapState::MapState(Map::MapType type, Player* player, Enemy* robot, const std::vector<Thing*>& ents,
    bool lootCollected, bool cardObtained, bool weapon, bool key, bool potion, bool monsterDefeated)
    : mapType(type), playerX(player ? player->getX() : 0), playerY(player ? player->getY() : 0),
    archiveRobot(robot), archiveLootCollected(lootCollected), doorCardObtained(cardObtained),
    weaponObtained(weapon), keyObtained(key), hpPotionObtained(potion),
    cabinetMonsterDefeated(monsterDefeated) {
    entities = new std::vector<Thing*>();
    for (auto entity : ents) {
        if (entity != player && entity != robot) {
            entities->push_back(entity);
        }
    }
}

MapState::~MapState() {
    for (auto entity : *entities) {
        delete entity;
    }
    delete entities;
}

void MapState::save(std::ofstream& file) const {
    file << static_cast<int>(mapType) << std::endl;
    file << playerX << " " << playerY << std::endl;

    // 保存机器人状态
    bool hasRobot = (archiveRobot != nullptr && archiveRobot->isAlive());
    file << hasRobot << std::endl;
    if (hasRobot) {
        archiveRobot->save(file);
    }

    // 保存所有物品获取状态
    file << archiveLootCollected << std::endl;
    file << doorCardObtained << std::endl;
    file << weaponObtained << std::endl;
    file << keyObtained << std::endl;
    file << hpPotionObtained << std::endl;
    file << cabinetMonsterDefeated << std::endl;

    // 保存实体数量和状态
    file << entities->size() << std::endl;
    for (const auto& entity : *entities) {
        file << entity->getType() << std::endl;
        entity->save(file);
    }
}

void MapState::load(std::ifstream& file) {
    int type;
    file >> type;
    mapType = static_cast<Map::MapType>(type);

    file >> playerX >> playerY;

    // 加载机器人状态
    bool hasRobot;
    file >> hasRobot;
    if (hasRobot) {
        // 读取机器人属性
        int x, y, hp, attack, aggression;
        std::string name;
        bool isChaser;

        file >> x >> y >> hp >> attack >> aggression >> name >> isChaser;
        archiveRobot = new Enemy(x, y, name, isChaser);
        // 设置HP
        while (archiveRobot->getHp() > hp) {
            archiveRobot->reduceHp(1);
        }
    }
    else {
        archiveRobot = nullptr;
    }

    // 加载所有物品获取状态
    file >> archiveLootCollected;
    file >> doorCardObtained;
    file >> weaponObtained;
    file >> keyObtained;
    file >> hpPotionObtained;
    file >> cabinetMonsterDefeated;

    // 加载实体
    size_t entityCount;
    file >> entityCount;

    if (entities) {
        for (auto entity : *entities) {
            delete entity;
        }
        entities->clear();
    }
    else {
        entities = new std::vector<Thing*>();
    }

    for (size_t i = 0; i < entityCount; ++i) {
        std::string type;
        file >> type;

        if (type == "Object") {
            int x, y, objType, attackBoost;
            std::string name;
            bool alive;

            file >> x >> y >> alive >> objType >> name >> attackBoost;

            Object* obj = new Object(static_cast<Object::ObjType>(objType), x, y);
            obj->setAlive(alive);
            obj->setName(name);
            entities->push_back(obj);
        }
        // 加载其他实体类型
        else if (type == "Enemy" && mapType == Map::LAB) {
            int x, y, hp, attack, aggression;
            std::string name;
            bool alive, isChaser;

            file >> x >> y >> alive >> hp >> attack >> aggression >> name >> isChaser;

            Enemy* enemy = new Enemy(x, y, name, isChaser);
            enemy->setAlive(alive);
            // 设置HP
            while (enemy->getHp() > hp) {
                enemy->reduceHp(1);
            }
            entities->push_back(enemy);
        }
    }
}
