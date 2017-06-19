#ifndef VERTEX_H
#define VERTEX_H
#include "direction.h"
#include <vector>
#include <list>
#include <cstdint>
#include <QPoint>
#include <QSize>

class Face;
class QPainter;

class Vertex
{
public:
	typedef std::list<Vertex*> List_t;

	Vertex() = delete;
	Vertex(Face & face, Corner corner, QPoint p);

	Face & face;
	const Corner corner;

	int32_t x;
	int32_t y;

	QPoint stored;
	QPoint lastValid;

	bool isSelected;

	operator QPoint() const { return QPoint(x, y); }

	QPointF toPointF() const { return QPointF(x, y); }

	const Vertex & operator=(const QPoint & p);
	const Vertex & operator=(const Vertex & p);

	bool operator==(const Vertex & v) const;
	bool operator!=(const Vertex & v) const;

	bool operator==(const QPoint & v) const;
	bool operator!=(const QPoint & v) const;

	bool isContained(QPoint, QSize) const;

	bool removeFace(Face * it);
	bool swapFace(Face * original, Face * replace);
	bool canMoveHorizontally();
	bool isEdgeSelected();
	bool isExtrudable();

	void draw(QPainter & painter);

};

#endif // VERTEX_H
