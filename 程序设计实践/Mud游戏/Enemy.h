#ifndef ENEMY_H
#define ENEMY_H

#include "Thing.h"
#include "Player.h"
#include "Map.h"
#include <string>

class Enemy : public Thing {
private:
    int hp;
    int attack;
    int aggression;
    std::string name;
    bool isChaser;
public:
    Enemy(int x_, int y_, const std::string& name_, bool chaser = false)
        : Thing(x_, y_), name(name_), isChaser(chaser) {
        if (!isChaser) {
            hp = 30;
            attack = 25;
            aggression = 60;
        }
        else {
            hp = 50;
            attack = 20;
            aggression = 80;
        }
    }

    void update() override {}
    void chasePlayer(const Player& player, const Map& map);
    void reduceHp(int val) { hp = (hp - val) < 0 ? 0 : hp - val; }
    bool isDead() const { return hp <= 0; }
    int getAttack() const { return attack; }
    std::string getName() const { return name; }
    int getHp() const { return hp; }
    int getAggression() { return aggression; }
    bool isChaserType() const { return isChaser; }

    // 存档相关方法
    void save(std::ofstream& file) const override;
    void load(std::ifstream& file) override;
    std::string getType() const override { return "Enemy"; }
};

#endif // ENEMY_H
