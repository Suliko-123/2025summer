#ifndef BACKPACK_H
#define BACKPACK_H

#include "Object.h"
#include "Player.h"
#include "Map.h"
#include <vector>
#include <string>

class Backpack {
private:
    std::vector<Object*>* items;
    const int MAX_CAP;

public:
    Backpack() : MAX_CAP(5) {
        items = new std::vector<Object*>();
    }

    ~Backpack() {
        for (auto item : *items) delete item;
        delete items;
    }

    bool addItem(Object* item);
    bool useItem(int index, Player& player, const Map& map, bool& isOpenDoor, bool& isWin);
    bool hasKey() const;
    bool hasBlueprint() const;
    std::vector<std::string> getItemsName() const;
    int getItemCount() const { return static_cast<int>(items->size()); }
    int getMaxCap() const { return MAX_CAP; }

    // 存档相关方法
    void save(std::ofstream& file) const;
    void load(std::ifstream& file);
};

#endif // BACKPACK_H
