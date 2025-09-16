#include "Backpack.h"
#include "Display.h"
#include <string>

bool Backpack::addItem(Object* item) {
    if (items->size() >= static_cast<size_t>(MAX_CAP)) return false;
    items->push_back(item);
    item->setAlive(false);
    return true;
}

bool Backpack::useItem(int index, Player& player, const Map& map, bool& isOpenDoor, bool& isWin) {
    isOpenDoor = false;
    isWin = false;
    int playerX = player.getX();
    int playerY = player.getY();
    int cell = map.getCell(playerX, playerY);

    // 检查索引有效性
    if (index < 0 || index >= static_cast<int>(items->size())) {
        std::vector<std::string> clues = { "[提示] 背包此格子为空！" };
        Display::showCluePage("物品使用提示", clues);
        return false;
    }

    Object* item = (*items)[index];
    switch (item->getObjectType()) {
    case Object::HP_POTION:
        player.addHp(20);
        Display::showResultPage("使用药丸，恢复20HP！当前HP：" + std::to_string(player.getHp()), true);
        items->erase(items->begin() + index);
        return true;

    case Object::KEY:
        if (cell != TILE_DOOR) {
            std::vector<std::string> clues = { "[提示] 请先移动到档案室门（D）的位置！" };
            Display::showCluePage("钥匙使用提示", clues);
            return false;
        }
        item->setName("青铜钥匙（已使用）");
        Display::showResultPage("青铜钥匙转动，门开了！前往生化实验室！", true);
        isOpenDoor = true;
        return true;

    case Object::BLUEPRINT_FRAGMENT: {
        std::vector<std::string> clues = {
            "【图纸碎片】拼合后显示逃生密码：19000715",
            "（推测为1900年科学家生日，*逃生必需*！）",
            "",
            "提示：安全门验证时需要输入此密码，务必牢记！"
        };
        Display::showCluePage("图纸线索", clues);
        return true;
    }

    case Object::DOOR_CARD: {
        if (cell != TILE_SAFE_DOOR) {
            std::vector<std::string> clues = { "[提示] 请先移动到安全门（S）的位置！" };
            Display::showCluePage("门卡使用提示", clues);
            return false;
        }

        std::vector<std::string> passwordClues = {
            "安全门需要双重验证：门卡+图纸密码",
            "提示：图纸碎片上记录的密码是科学家生日",
            "（输入6-8位数字，按回车确认）"
        };
        std::string inputPassword = Display::showInputPage("密码验证", passwordClues, "请输入图纸密码");

        if (inputPassword != "19000715") {
            Display::showResultPage("密码错误！安全门未开启，请重新输入。", false);
            return false;
        }
        Display::showResultPage("门卡验证通过+密码正确！安全门开启，成功逃脱！", true);
        items->erase(items->begin() + index);
        isWin = true;
        return true;
    }

    case Object::WEAPON:
        player.addAttack(item->getAttackBoost());
        Display::showResultPage("装备" + item->getName() + "，攻击力+" + std::to_string(item->getAttackBoost()) + "！当前攻击力：" + std::to_string(player.getAttack()), true);
        items->erase(items->begin() + index);
        return true;
    }
    return false;
}

bool Backpack::hasKey() const {
    for (auto item : *items) {
        if (item && (item->getObjectType() == Object::KEY || item->getObjectType() == Object::DOOR_CARD)) {
            return true;
        }
    }
    return false;
}

bool Backpack::hasBlueprint() const {
    for (auto item : *items) {
        if (item && item->getObjectType() == Object::BLUEPRINT_FRAGMENT) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> Backpack::getItemsName() const {
    std::vector<std::string> names;
    for (size_t i = 0; i < items->size(); i++) {
        names.push_back((*items)[i]->getName());
    }
    return names;
}

void Backpack::save(std::ofstream& file) const {
    file << items->size() << std::endl;
    for (const auto& item : *items) {
        item->save(file);
    }
}

void Backpack::load(std::ifstream& file) {
    // 清空现有物品
    for (auto item : *items) {
        delete item;
    }
    items->clear();

    size_t itemCount;
    file >> itemCount;

    for (size_t i = 0; i < itemCount; ++i) {
        int x, y, typeInt, attackBoost;
        std::string name;
        bool alive;

        file >> x >> y >> alive >> typeInt >> name >> attackBoost;

        Object* obj = new Object(static_cast<Object::ObjType>(typeInt), x, y);
        obj->setAlive(alive);
        obj->setName(name);
        items->push_back(obj);
    }
}
