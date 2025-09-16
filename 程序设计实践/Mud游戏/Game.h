#ifndef GAME_H
#define GAME_H

#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Backpack.h"
#include "Display.h"
#include "MapState.h"
#include <stack>
#include <ctime>

class Game {
private:
    Player* player;
    Map* currentMap;
    Backpack* backpack;
    Enemy* archiveRobot;
    bool isRunning;
    int currentMapIdx;
    bool needRender;
    clock_t lastRobotMoveTime;
    std::stack<MapState*>* mapHistory;

    // ��Ʒ��ȡ״̬ - ȫ������Ϊֻ�ܻ�ȡһ��
    bool archiveLootCollected;  // ������ͼֽ��ҩ��
    bool doorCardObtained;      // �ſ�
    bool weaponObtained;        // ����(��ͭ�˹�)
    bool keyObtained;           // ��ͭԿ��
    bool hpPotionObtained;      // ҩ��
    bool cabinetMonsterDefeated; // ���ӹ����Ƿ��ѱ�����

    std::vector<Thing*> copyEntitiesExceptPlayerAndRobot(const std::vector<Thing*>& original, Player* player, Enemy* robot);

    // ����������ȷ���浵Ŀ¼����
    void ensureSaveDirectoryExists() const;

public:
    Game();
    ~Game();
    void spawnArchiveRobot();
    void dropWeaponAtRobotPos();
    void switchMap(Map::MapType targetType);
    void returnToPreviousMap();
    bool fight(Enemy* enemy);
    void attackAction(Enemy* enemy);
    void dodgeAction(Enemy* enemy);
    bool escapeAction(Enemy* enemy);
    void interactWithDoor();
    void checkInteraction();
    void handleInput();
    void updateRobotMovement();
    bool movePlayer(int dx, int dy);
    void renderNow();
    void run();

    // �浵��ط���
    bool saveGame(int slot);
    bool loadGame(int slot);
};

#endif // GAME_H
