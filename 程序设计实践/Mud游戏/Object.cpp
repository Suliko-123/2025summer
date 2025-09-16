#include "Object.h"

void Object::save(std::ofstream& file) const {
    file << x << " " << y << " " << alive << " "
        << static_cast<int>(objType) << " " << name << " " << attackBoost << std::endl;
}

void Object::load(std::ifstream& file) {
    int typeInt;
    file >> x >> y >> alive >> typeInt >> name >> attackBoost;
    objType = static_cast<ObjType>(typeInt);
}
