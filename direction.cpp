#include <cassert>
#include "direction.h"

EndPoint_t GetCorners(Direction d)
{
	switch(d)
	{
	case Left:
		return EndPoint_t(TopLeft, BottomLeft);
	case Right:
		return EndPoint_t(TopRight, BottomRight);
	case Up:
		return EndPoint_t(TopLeft, TopRight);
	case Down:
		return EndPoint_t(BottomLeft, BottomRight);
	default:
		assert(false);
	}
}

Corner GetOppositeCorner(Corner corner)
{
	switch(corner)
	{
	case TopLeft:     return BottomLeft;
	case BottomLeft:  return TopLeft;
	case TopRight:    return BottomRight;
	case BottomRight: return TopRight;
	default:
		assert(false);
	}
}
