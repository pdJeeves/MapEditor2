#ifndef FACE_H
#define FACE_H
#include "vertex.h"
#include "direction.h"
#include <list>
#include <QRect>

class Vertex;
class QPainter;
class QPoint;
class Selection;

/* TODO: search for doors does not correctly find new doors when face has been deleted */


class Face
{
public:
	typedef std::vector<Face*> List_t;

private:
friend class Vertex;
	QPoint interpolatePoint(int i, float ratio);

	QRect bounds[5];

	static QRect GetRect(QPoint begin, QPoint end);

	int index;
	List_t * m_faces;

public:
	void computeBounds();

	Face();
	Face(const Face &);
	Face(Face && face) = delete;
	Face(QPoint * point, uint8_t room_type);
	~Face();

	bool continueSplit(float & t1, Direction dir) const;

	void setFaceList(List_t * list);

	List_t * faceList() const { return m_faces; }
	int id() const { return index; }

	Face * splitFrom;

	Vertex verticies[4];
	Face*  adjacent[4];
	uint8_t permeability[4];

	uint8_t room_type;

	bool isEdgeSelected() const;
	bool isSelected() const;
	bool canMoveHorizontally() const;

	bool isExtrudable() const;

	bool doesIntersect(QRect rect) const;
	bool isContained(QRect rect) const;

	float getRatio(QPoint pos, bool direction) const;
	bool doesContain(QPoint pos) const;
	bool doesContain(QPoint pos, Direction &dir) const;

	void drawOutline(QPainter & painter) const;
	void drawDoors(QPainter & painter) const;

	static void drawSplit(Face *_this, QPainter & painter, float ratio, bool direction);
	static Vertex::List_t splitFaces(Face * _this, float ratio, bool direction);
	bool SnapToEdge(QPoint & pos, float corner_snap, float edge_snap) const;

	static Vertex::List_t addFace(List_t * list, QPoint *);

	bool isOppositeEdgeAdjacent(const Face & face, int i) const;
	bool isAdjacent(const Face & face) const;
	std::pair<QPoint, QPoint> getMidSegment(int i) const;
	bool SnapToCorner(QPoint & pos, float &snap) const;
	bool SnapToEdge(QPoint & pos, float & snap) const;

	void restoreVerticies();
	void validateShape();

	bool isVertSelected() const;
	bool isClockwise(int i) const;

	bool doesContainCorner(const Face & it) const;
	void extrude(Vertex::List_t & verts);

	void tryConnectEdge(Direction i);
	int isLocationValid() const;


	void validateTranslation();
	bool findIntersection(QPoint begin, QPoint end, float * length) const;
	bool isHandle(Selection & selection, QPoint point, float fuzz);

	bool isEdgeSelected(Direction d) const;
	bool canSetPermeability(Direction d) const;

	int getPermeability(Direction d) const;
	void setPermeability(Direction d, int perm);
	void swapPermeability(Direction d, uint8_t & permeability);

	void setAdjacent(Direction d, Face * face);
	void swapAdjacent(Direction d, Face *& face);

	void swapVertex(Corner c, QPoint &p);

	static Face::List_t getSplittingFaces(Face * _this, float ratio, bool direction);
	void searchForDoors(Direction d);
};



#endif // FACE_H
