#include "Player.h"
#include "Map.h"

bool Player::canMove(int dx, int dy, const Map& map) const {
    int newX = x + dx;
    int newY = y + dy;
    return map.isPassable(newX, newY);
}

void Player::save(std::ofstream& file) const {
    file << x << " " << y << " " << alive << " "
        << hp << " " << attack << " " << name << " " << keyCount << std::endl;
}

void Player::load(std::ifstream& file) {
    file >> x >> y >> alive >> hp >> attack >> name >> keyCount;
}
