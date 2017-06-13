#include <cassert>
#include "vertex.h"
#include "face.h"

Vertex::Vertex(Face & face, Corner corner, QPoint p) :
	face(face),
	corner(corner),
	x(p.x()),
	y(p.y()),
	isSelected(false)
{
}

bool Vertex::operator==(const Vertex & v) const
{
	return x == v.x && y == v.y;
}

bool Vertex::operator!=(const Vertex & v) const
{
	return x != v.x || y != v.y;
}

bool Vertex::isContained(QPoint pos, QSize size) const
{
	return pos.x() < x && x < pos.x() + size.width()
		&& pos.y() < y && y < pos.y() + size.height();
}


void Vertex::destroy()
{
	face.destroyed = true;
}

bool Vertex::canMoveHorizontally()
{
	return isSelected && face.verticies[GetOppositeCorner(corner)].isSelected;
}

bool Vertex::isEdgeSelected()
{
	return face.isEdgeSelected();
}

bool Vertex::isExtrudable()
{
	return face.isExtrudable();
}
