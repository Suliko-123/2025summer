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

    // ���������Ч��
    if (index < 0 || index >= static_cast<int>(items->size())) {
        std::vector<std::string> clues = { "[��ʾ] �����˸���Ϊ�գ�" };
        Display::showCluePage("��Ʒʹ����ʾ", clues);
        return false;
    }

    Object* item = (*items)[index];
    switch (item->getObjectType()) {
    case Object::HP_POTION:
        player.addHp(20);
        Display::showResultPage("ʹ��ҩ�裬�ָ�20HP����ǰHP��" + std::to_string(player.getHp()), true);
        items->erase(items->begin() + index);
        return true;

    case Object::KEY:
        if (cell != TILE_DOOR) {
            std::vector<std::string> clues = { "[��ʾ] �����ƶ����������ţ�D����λ�ã�" };
            Display::showCluePage("Կ��ʹ����ʾ", clues);
            return false;
        }
        item->setName("��ͭԿ�ף���ʹ�ã�");
        Display::showResultPage("��ͭԿ��ת�����ſ��ˣ�ǰ������ʵ���ң�", true);
        isOpenDoor = true;
        return true;

    case Object::BLUEPRINT_FRAGMENT: {
        std::vector<std::string> clues = {
            "��ͼֽ��Ƭ��ƴ�Ϻ���ʾ�������룺19000715",
            "���Ʋ�Ϊ1900���ѧ�����գ�*��������*����",
            "",
            "��ʾ����ȫ����֤ʱ��Ҫ��������룬����μǣ�"
        };
        Display::showCluePage("ͼֽ����", clues);
        return true;
    }

    case Object::DOOR_CARD: {
        if (cell != TILE_SAFE_DOOR) {
            std::vector<std::string> clues = { "[��ʾ] �����ƶ�����ȫ�ţ�S����λ�ã�" };
            Display::showCluePage("�ſ�ʹ����ʾ", clues);
            return false;
        }

        std::vector<std::string> passwordClues = {
            "��ȫ����Ҫ˫����֤���ſ�+ͼֽ����",
            "��ʾ��ͼֽ��Ƭ�ϼ�¼�������ǿ�ѧ������",
            "������6-8λ���֣����س�ȷ�ϣ�"
        };
        std::string inputPassword = Display::showInputPage("������֤", passwordClues, "������ͼֽ����");

        if (inputPassword != "19000715") {
            Display::showResultPage("������󣡰�ȫ��δ���������������롣", false);
            return false;
        }
        Display::showResultPage("�ſ���֤ͨ��+������ȷ����ȫ�ſ������ɹ����ѣ�", true);
        items->erase(items->begin() + index);
        isWin = true;
        return true;
    }

    case Object::WEAPON:
        player.addAttack(item->getAttackBoost());
        Display::showResultPage("װ��" + item->getName() + "��������+" + std::to_string(item->getAttackBoost()) + "����ǰ��������" + std::to_string(player.getAttack()), true);
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
    // ���������Ʒ
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
