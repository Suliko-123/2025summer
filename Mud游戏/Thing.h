#ifndef THING_H
#define THING_H

#include <fstream>
#include <stdexcept>
#include <string>

class Thing {
protected:
    int x;
    int y;
    bool alive;
public:
    Thing(int x_, int y_) : x(x_), y(y_), alive(true) {}
    virtual void update() = 0;

    // ��ӿ�ָ�밲ȫ���Ļ�ȡ��
    int getX() const {
        if (this == nullptr) {
            throw std::runtime_error("����getX()ʱthisָ��Ϊnullptr");
        }
        return x;
    }

    int getY() const {
        if (this == nullptr) {
            throw std::runtime_error("����getY()ʱthisָ��Ϊnullptr");
        }
        return y;
    }

    void setPos(int x_, int y_) {
        if (this == nullptr) {
            throw std::runtime_error("����setPos()ʱthisָ��Ϊnullptr");
        }
        x = x_;
        y = y_;
    }

    bool isAlive() const {
        if (this == nullptr) {
            throw std::runtime_error("����isAlive()ʱthisָ��Ϊnullptr");
        }
        return alive;
    }

    void setAlive(bool flag) {
        if (this == nullptr) {
            throw std::runtime_error("����setAlive()ʱthisָ��Ϊnullptr");
        }
        alive = flag;
    }

    virtual ~Thing() {}

    // �浵��ط���
    virtual void save(std::ofstream& file) const = 0;
    virtual void load(std::ifstream& file) = 0;
    virtual std::string getType() const = 0;

    // ���һ����ȫ��鷽����������ʹ��ָ��ǰ��֤
    bool isValid() const {
        return this != nullptr;
    }
};

#endif // THING_H
