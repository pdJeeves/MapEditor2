#ifndef SELECTION_H
#define SELECTION_H
#include "direction.h"
#include "face.h"
#include <vector>
#include <list>
#include <QPoint>
#include <QSize>

class Vertex;
class Face;
class QPainter;
class MapEditor;

typedef std::pair<Vertex*, QPoint> VertexPos;
typedef std::vector<VertexPos> VertexState;

class Selection
{
public:
typedef Vertex::List_t List_t;

private:
	MapEditor * window;

	void setSelection(List_t &&list);
	void andSelection(List_t && list);
	void orSelection(List_t && list);
	void xorSelection(List_t && list);

	static float distSquaredToLine(QPoint begin, QPoint end, QPoint p);
	static float distSquared(QPoint begin, QPoint end);
	static float dotProduct(QPoint u, QPoint v);

	bool getVerticies(List_t & r, QPoint pos) const;
	bool getEdges(List_t & r, QPoint pos) const;
	bool getFaces(List_t & r, QPoint pos) const;

public:
	Selection(MapEditor*);

	void setSelected(List_t list);

	Face::List_t allFaces;

	Face::List_t faces;
	List_t verts;

	bool isExtrudable() const;
	bool canMoveHorizontally() const;

	void selectVerticies(List_t &&list, Qt::KeyboardModifiers mod = Qt::NoModifier);

	void clear();

	List_t getVerticies(QPoint pos, QSize size) const;
	List_t getVerticies(QPoint pos) const;

	bool isSelected(QPoint pos) const;

	void draw(QPainter & painter);

	VertexState getVertexLocations() const;
	void setVertexLocations(const VertexState &vec );

	QPoint adjustDelta(QPoint delta, QSize size) const;
	void computeFaceList();

	bool isOnlyFaces() const;
	bool isOnlyEdges() const;


	bool isValidPosition(QPoint p) const;

};

#endif // SELECTION_H
