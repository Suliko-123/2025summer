#ifndef MAP_H
#define MAP_H

#include "constants.h"
#include "Thing.h"
#include <vector>
#include <string>

class Map {
public:
    enum MapType { ARCHIVE_ROOM, LAB };

private:
    MapType mapType;
    int width;
    int height;
    int** mapData;
    std::vector<Thing*> entities;

public:
    Map(MapType type);
    ~Map();
    void setSizeByType();
    void initMap();
    void setSpecialTiles();
    bool isPassable(int x, int y) const;
    void setCell(int x, int y, int tileType);
    void addEntity(Thing* entity);
    void removeEntity(Thing* entity);
    MapType getMapType() const { return mapType; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getCell(int x, int y) const;
    std::vector<Thing*> getEntities() const { return entities; }
    void clearEntities() { entities.clear(); }

    // 存档相关方法
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
};

#endif // MAP_H
