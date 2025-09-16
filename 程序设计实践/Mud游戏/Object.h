#ifndef OBJECT_H
#define OBJECT_H

#include "Thing.h"
#include <string>

class Object : public Thing {
public:
    enum ObjType { HP_POTION, KEY, BLUEPRINT_FRAGMENT, DOOR_CARD, WEAPON };
private:
    ObjType objType;
    std::string name;
    int attackBoost;
public:
    Object(ObjType t, int x_, int y_)
        : Thing(x_, y_), objType(t), attackBoost(0) {
        switch (objType) {
        case HP_POTION: name = "ҩ��"; break;
        case KEY: name = "��ͭԿ��"; break;
        case BLUEPRINT_FRAGMENT: name = "ͼֽ��Ƭ"; break;
        case DOOR_CARD: name = "�ſ�"; break;
        case WEAPON:
            name = "��ͭ�˹�";
            attackBoost = 15;
            break;
        }
    }

    void update() override {}
    ObjType getObjectType() const { return objType; }
    std::string getName() const { return name; }
    int getAttackBoost() const { return attackBoost; }
    void setName(const std::string& newName) { name = newName; }

    // �浵��ط���
    void save(std::ofstream& file) const override;
    void load(std::ifstream& file) override;
    std::string getType() const override { return "Object"; }
};

#endif // OBJECT_H
