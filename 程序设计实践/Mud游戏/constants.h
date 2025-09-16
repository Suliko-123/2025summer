#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

// 前向声明
class Thing;
class Player;
class Enemy;
class Map;

// 常量定义
const int TILE_EMPTY = 0;
const int TILE_WALL = 1;
const int TILE_BOX = 2;
const int TILE_PHOTO_WALL = 3;
const int TILE_SHELF = 4;
const int TILE_DOOR = 5;
const int TILE_TRASH = 6;
const int TILE_MICROSCOPE = 7;
const int TILE_CABINET = 8;
const int TILE_REACT = 9;
const int TILE_SAFE_DOOR = 10;
const int TILE_BACK_DOOR = 11;

// 存档相关常量
const std::string SAVE_FILE_EXTENSION = ".sav";
const std::string SAVE_DIRECTORY = "saves/";

#endif // CONSTANTS_H
