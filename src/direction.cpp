#include <cassert>
#include "direction.h"


Corner EndCorner(Direction d)
{
	switch(d)
	{
	case Left:  return BottomLeft;
	case Right: return BottomRight;
	case Up:    return TopRight;
	case Down:  return BottomRight;
	default:    assert(false);
	}
}

Corner StartCorner(Direction d)
{
	switch(d)
	{
	case Left:  return TopLeft;
	case Right: return TopRight;
	case Up:    return TopLeft;
	case Down:  return BottomLeft;
	default:    assert(false);
	}
}

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

DirPair_t GetOppositeEdges(Corner corner)
{
	switch(corner)
	{
	case TopLeft:
		return DirPair_t(Down, Right);
	case BottomLeft:
		return DirPair_t(Up, Right);
	case TopRight:
		return DirPair_t(Down, Left);
	case BottomRight:
		return DirPair_t(Up, Left);
	default:
		assert(false);
	}
}


Direction ClockwiseEdge(Corner corner)
{
	switch(corner)
	{
	case TopLeft:      return Up;
	case TopRight:	   return Right;
	case BottomRight:  return Down;
	case BottomLeft:   return Left;
	default:
		assert(false);
	}
}

Direction CounterClockwiseEdge(Corner corner)
{
	switch(corner)
	{
	case TopLeft:      return Left;
	case TopRight:	   return Up;
	case BottomRight:  return Right;
	case BottomLeft:   return Down;
	default:
		assert(false);
	}
}



Corner ClockwiseCorner(Corner corner)
{
	switch(corner)
	{
	case TopLeft:      return TopRight;
	case TopRight:	   return BottomRight;
	case BottomRight:  return BottomLeft;
	case BottomLeft:   return TopLeft;
	default:
		assert(false);
	}
}

Corner CounterClockwiseCorner(Corner corner)
{
	switch(corner)
	{
	case TopLeft:      return BottomLeft;
	case TopRight:	   return TopLeft;
	case BottomRight:  return TopRight;
	case BottomLeft:   return BottomRight;
	default:
		assert(false);
	}
}

Direction GetEdge(Corner a, Corner b)
{
	if((a == TopLeft     && b == TopRight)
	|| (b == TopLeft     && a == TopRight))
		return Up;

	if((a == BottomLeft  && b == BottomRight)
	|| (b == BottomLeft  && a == BottomRight))
		return Down;

	if((a == TopLeft     && b == BottomLeft)
	|| (b == TopLeft     && a == BottomLeft))
		return Left;

	if((a == TopRight    && b == BottomRight)
	|| (b == TopRight    && a == BottomRight))
		return Right;

	return InvalidDirection;
}
