#ifndef MAPSTATE_H
#define MAPSTATE_H

#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include <vector>
#include <fstream>

// ��ͼ״̬�ṹ��
struct MapState {
    Map::MapType mapType;
    int playerX;
    int playerY;
    Enemy* archiveRobot;
    std::vector<Thing*>* entities;

    // ��Ʒ��ȡ״̬ - ȫ����Ʒ��ֻ�ܻ�ȡһ��
    bool archiveLootCollected;  // ��¼�Ƿ��ѻ�ȡ��������Ʒ
    bool doorCardObtained;      // ��¼�Ƿ��ѻ�ȡ�ſ�
    bool weaponObtained;        // ��¼�Ƿ��ѻ�ȡ����
    bool keyObtained;           // ��¼�Ƿ��ѻ�ȡ��ͭԿ��
    bool hpPotionObtained;      // ��¼�Ƿ��ѻ�ȡҩ��
    bool cabinetMonsterDefeated; // ���ӹ����Ƿ��ѱ�����

    MapState(Map::MapType type, Player* player, Enemy* robot, const std::vector<Thing*>& ents,
        bool lootCollected, bool cardObtained, bool weapon, bool key, bool potion, bool monsterDefeated = false);
    ~MapState();

    // �浵��ط���
    void save(std::ofstream& file) const;
    void load(std::ifstream& file);
};

#endif // MAPSTATE_H
