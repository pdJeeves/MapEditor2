#ifndef VERTEX_H
#define VERTEX_H
#include "direction.h"
#include <vector>
#include <cstdint>
#include <QPoint>
#include <QSize>

class Face;

class Vertex
{
friend class Face;
	Face & face;
	const Corner corner;

public:
	Vertex() = delete;
	Vertex(Face & face, Corner corner, QPoint p);

	int32_t x;
	int32_t y;

	bool isSelected;

	operator QPoint() const { return QPoint(x, y); }

	bool operator==(const Vertex & v) const;
	bool operator!=(const Vertex & v) const;

	bool isContained(QPoint, QSize) const;

	void destroy();

	bool removeFace(Face * it);
	bool swapFace(Face * original, Face * replace);
	bool canMoveHorizontally();
	bool isEdgeSelected();
	bool isExtrudable();
};

#endif // VERTEX_H
