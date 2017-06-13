#ifndef FACE_H
#define FACE_H
#include "vertex.h"
#include "direction.h"
#include <list>

class Vertex;
class QPainter;
class QPoint;

class Face
{
friend class Vertex;
	QPoint interpolatePoint(Direction after, EndPoint_t eBefore, EndPoint_t eAfter, float ratio);

	bool extruded;
	bool destroyed;


public:
typedef std::vector<Face*> List_t;
	static List_t & allFaces();

	Face() = delete;
	Face(const Face &) = delete;
	Face(Face && face) = delete;
	Face(QPoint * point, uint8_t room_type);
	~Face();

	static void clearExtrudeFlag();
	bool trueConnection(Direction dir) const;
	bool continueSplit(float & t1, Direction dir) const;

	Vertex verticies[4];
	Face*  adjacent[4];
	uint8_t permeability[4];

	uint8_t room_type;

	bool isEdgeSelected() const;
	bool isSelected() const;
	bool canMoveHorizontally() const;

	bool isExtrudable() const;

	bool doesIntersect(QPoint pos, QSize size) const;
	bool isContained(QPoint pos, QSize size) const;

	bool doesContain(QPoint pos) const;
	bool doesContain(QPoint pos, float &min, Direction &dir) const;

	void drawOutline(QPainter & painter) const;
	void drawDoors(QPainter & painter) const;

	static void drawSplit(Face *_this, QPainter & painter, float ratio, bool direction);
	static std::list<Vertex*> splitFaces(Face * _this, float ratio, bool direction);
	static bool isValidPosition(QPoint *);
	static bool isValidPosition(QPoint p);

	static std::list<Vertex*> addFace(QPoint *);

	bool isOppositeEdgeAdjacent(const Face & face, int i) const;
	bool isAdjacent(const Face & face) const;
};



#endif // FACE_H
