#include "Map.h"
#include "constants.h"
#include "Player.h"
#include "Enemy.h"
#include "Object.h"
#include <fstream>
#include <algorithm>

Map::Map(MapType type)
    : mapType(type), width(0), height(0), mapData(nullptr), entities() {
    setSizeByType();
    initMap();
    setSpecialTiles();
}

Map::~Map() {
    if (mapData != nullptr) {
        for (int i = 0; i < height; i++) delete[] mapData[i];
        delete[] mapData;
    }
}

void Map::removeEntity(Thing* entity) {
    auto it = find(entities.begin(), entities.end(), entity);
    if (it != entities.end()) {
        entities.erase(it);
    }
}

void Map::setSizeByType() {
    if (mapType == ARCHIVE_ROOM) {
        width = 80;
        height = 30;
    }
    else if (mapType == LAB) {
        width = 96;
        height = 30;
    }
}

void Map::initMap() {
    if (width <= 0 || height <= 0) return;
    mapData = new int* [height];
    for (int i = 0; i < height; i++) {
        mapData[i] = new int[width];
        for (int j = 0; j < width; j++) {
            mapData[i][j] = (i == 0 || i == height - 1 || j == 0 || j == width - 1) ? TILE_WALL : TILE_EMPTY;
        }
    }
}

void Map::setSpecialTiles() {
    if (mapData == nullptr) return;
    if (mapType == ARCHIVE_ROOM) {
        mapData[8][20] = TILE_PHOTO_WALL;
        mapData[15][60] = TILE_SHELF;
        mapData[20][40] = TILE_BOX;
        mapData[15][78] = TILE_DOOR;
    }
    else if (mapType == LAB) {
        mapData[5][20] = TILE_TRASH;
        mapData[10][30] = TILE_MICROSCOPE;
        mapData[10][60] = TILE_CABINET;  // 柜子位置
        mapData[20][50] = TILE_REACT;
        mapData[15][94] = TILE_SAFE_DOOR;
        mapData[1][1] = TILE_BACK_DOOR;
    }
}

bool Map::isPassable(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height || mapData == nullptr) return false;
    return mapData[y][x] != TILE_WALL;
}

void Map::setCell(int x, int y, int tileType) {
    if (x >= 0 && x < width && y >= 0 && y < height && mapData != nullptr) {
        mapData[y][x] = tileType;
    }
}

int Map::getCell(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height || mapData == nullptr) return -1;
    return mapData[y][x];
}

void Map::addEntity(Thing* entity) {
    entities.push_back(entity);
}

bool Map::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << static_cast<int>(mapType) << std::endl;
    file << width << " " << height << std::endl;

    // 保存地图数据
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            file << mapData[y][x] << " ";
        }
        file << std::endl;
    }

    // 保存实体数量和状态
    file << entities.size() << std::endl;
    for (const auto& entity : entities) {
        file << entity->getType() << std::endl;
        entity->save(file);
    }

    return true;
}

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    // 清理现有数据
    if (mapData != nullptr) {
        for (int i = 0; i < height; i++) delete[] mapData[i];
        delete[] mapData;
        mapData = nullptr;
    }

    for (auto entity : entities) {
        delete entity;
    }
    entities.clear();

    // 加载地图类型
    int type;
    file >> type;
    mapType = static_cast<MapType>(type);

    // 加载尺寸
    file >> width >> height;

    // 初始化地图
    mapData = new int* [height];
    for (int i = 0; i < height; ++i) {
        mapData[i] = new int[width];
        for (int j = 0; j < width; ++j) {
            file >> mapData[i][j];
        }
    }

    // 加载实体
    size_t entityCount;
    file >> entityCount;

    for (size_t i = 0; i < entityCount; ++i) {
        std::string type;
        file >> type;

        if (type == "Player") {
            int x, y, hp, attack, keyCount;
            std::string name;
            bool alive;

            file >> x >> y >> alive >> hp >> attack >> name >> keyCount;

            Player* player = new Player(x, y, name);
            player->setAlive(alive);
            // 设置HP
            while (player->getHp() > hp) {
                player->reduceHp(1);
            }
            // 调整攻击力
            int attackDiff = attack - 10; // 初始攻击力为10
            if (attackDiff > 0) {
                player->addAttack(attackDiff);
            }
            // 设置钥匙数量
            while (player->getKeyCount() < keyCount) {
                player->addKey();
            }
            entities.push_back(player);
        }
        else if (type == "Enemy") {
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
            entities.push_back(enemy);
        }
        else if (type == "Object") {
            int x, y, objType, attackBoost;
            std::string name;
            bool alive;

            file >> x >> y >> alive >> objType >> name >> attackBoost;

            Object* obj = new Object(static_cast<Object::ObjType>(objType), x, y);
            obj->setAlive(alive);
            obj->setName(name);
            entities.push_back(obj);
        }
    }

    return true;
}
