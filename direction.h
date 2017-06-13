#ifndef DIRECTION_H
#define DIRECTION_H
#include <utility>

typedef enum Direction
{
	InvalidDirection = -1,
	Left = 0,
	Right = 1,
	Up = 2,
	Down = 3,
} Direction;

typedef enum Corner
{
	TopLeft = 0,
	TopRight = 1,
	BottomLeft = 2,
	BottomRight = 3,
} Corner;

typedef std::pair<Corner, Corner>  EndPoint_t;

EndPoint_t GetCorners(Direction);
Corner GetOppositeCorner(Corner corner);

#endif // DIRECTION_H
