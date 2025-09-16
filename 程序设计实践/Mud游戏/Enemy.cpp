#include "Enemy.h"
#include "Player.h"
#include "Map.h"

void Enemy::chasePlayer(const Player& player, const Map& map) {
    if (!isChaser || !alive) return;

    int dx = 0, dy = 0;
    if (x < player.getX()) dx = 1;
    else if (x > player.getX()) dx = -1;
    else if (y < player.getY()) dy = 1;
    else if (y > player.getY()) dy = -1;

    if (map.isPassable(x + dx, y + dy)) {
        setPos(x + dx, y + dy);
    }
}

void Enemy::save(std::ofstream& file) const {
    file << x << " " << y << " " << alive << " "
        << hp << " " << attack << " " << aggression << " "
        << name << " " << isChaser << std::endl;
}

void Enemy::load(std::ifstream& file) {
    file >> x >> y >> alive >> hp >> attack >> aggression >> name >> isChaser;
}
