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

    // 添加空指针安全检查的获取器
    int getX() const {
        if (this == nullptr) {
            throw std::runtime_error("调用getX()时this指针为nullptr");
        }
        return x;
    }

    int getY() const {
        if (this == nullptr) {
            throw std::runtime_error("调用getY()时this指针为nullptr");
        }
        return y;
    }

    void setPos(int x_, int y_) {
        if (this == nullptr) {
            throw std::runtime_error("调用setPos()时this指针为nullptr");
        }
        x = x_;
        y = y_;
    }

    bool isAlive() const {
        if (this == nullptr) {
            throw std::runtime_error("调用isAlive()时this指针为nullptr");
        }
        return alive;
    }

    void setAlive(bool flag) {
        if (this == nullptr) {
            throw std::runtime_error("调用setAlive()时this指针为nullptr");
        }
        alive = flag;
    }

    virtual ~Thing() {}

    // 存档相关方法
    virtual void save(std::ofstream& file) const = 0;
    virtual void load(std::ifstream& file) = 0;
    virtual std::string getType() const = 0;

    // 添加一个安全检查方法，用于在使用指针前验证
    bool isValid() const {
        return this != nullptr;
    }
};

#endif // THING_H
