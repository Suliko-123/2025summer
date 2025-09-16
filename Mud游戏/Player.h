#ifndef PLAYER_H
#define PLAYER_H

#include "Thing.h"
#include "Map.h"
#include <string>

class Player : public Thing {
private:
    int hp;
    int attack;
    std::string name;
    int keyCount;
public:
    Player(int x_, int y_, const std::string& name_)
        : Thing(x_, y_), hp(100), attack(10), name(name_), keyCount(0) {
    }
    void update() override {}
    bool canMove(int dx, int dy, const Map& map) const;
    void addHp(int val) { hp = (hp + val) > 100 ? 100 : hp + val; }
    void addAttack(int val) { attack += val; }
    void reduceHp(int val) { hp = (hp - val) < 0 ? 0 : hp - val; }
    void addKey() { keyCount++; }
    void removeKey() { keyCount--; }
    int getHp() const { return hp; }
    int getAttack() const { return attack; }
    std::string getName() const { return name; }
    int getKeyCount() const { return keyCount; }

    // 存档相关方法
    void save(std::ofstream& file) const override;
    void load(std::ifstream& file) override;
    std::string getType() const override { return "Player"; }
};

#endif // PLAYER_H
