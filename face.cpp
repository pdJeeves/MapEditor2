#include <cassert>
#include <QPainter>
#include "face.h"
#include "vertex.h"
#include "direction.h"

Face::List_t & Face::allFaces()
{
	static List_t list;
	return list;
}


Face::Face(QPoint * point, uint8_t room_type) :
	extruded(false),
	destroyed(false),
	verticies{
		Vertex(*this, (Corner) 0, point[0]),
		Vertex(*this, (Corner) 1, point[1]),
		Vertex(*this, (Corner) 2, point[2]),
		Vertex(*this, (Corner) 3, point[3])},
	adjacent{0, 0, 0, 0},
	permeability{0,0,0,0},
	room_type(room_type)
{
	allFaces().push_back(this);
}


Face::~Face()
{
	for(int i = 0; i < 4; ++i)
	{
		if(adjacent[i])
		{
			assert(this == adjacent[i]->adjacent[i^1]);
			adjacent[i]->adjacent[i^1] = 0L;
			adjacent[i]->permeability[i^1] = 0;
		}
	}

	for(auto i = allFaces().begin(); i != allFaces().end(); ++i)
	{
		if(*i == this)
		{
			allFaces().erase(i);
			break;
		}
	}
}

typedef std::pair<float, QPoint> Pair_t;

static
int ComparePair(const void * a, const void * b)
{
	float r = ((const Pair_t *) a)->first - ((const Pair_t *) b)->first;
	return r == 0? 0 : r < 0? -1 : 1;
}

static
std::pair<QPoint, QPoint> getMidSegment(QPoint a1, QPoint a2, QPoint b1, QPoint b2)
{
//always are all in 1 quadrant.
	Pair_t a[4];
	a[0] = Pair_t(atan2(a1.y(), a1.x()), a1);
	a[1] = Pair_t(atan2(a2.y(), a2.x()), a2);
	a[2] = Pair_t(atan2(b1.y(), b1.x()), b1);
	a[3] = Pair_t(atan2(b2.y(), b2.x()), b2);

	qsort(a, 4, sizeof(Pair_t), ComparePair);
	return std::make_pair(a[1].second, a[2].second);
}

void Face::clearExtrudeFlag()
{
	for(auto i = allFaces().begin(); i != allFaces().end(); ++i)
	{
		(*i)->extruded = false;
	}
}

bool Face::trueConnection(Direction dir) const
{
	if(adjacent[dir] == 0L)
		return false;

	Direction opp = (Direction) (dir^1);
	EndPoint_t eBefore = GetCorners(dir);
	EndPoint_t eAfter= GetCorners(opp);

	return verticies[eBefore.first ] == adjacent[dir]->verticies[eAfter.first ]
		&& verticies[eBefore.second] == adjacent[dir]->verticies[eAfter.second];
}

bool Face::continueSplit(float & t1, Direction dir) const
{
	if(adjacent[dir] == 0L)
		return false;

	Direction opp = (Direction) (dir^1);
	EndPoint_t eBefore = GetCorners(dir);
	EndPoint_t eAfter= GetCorners(opp);

	if(verticies[eBefore.first ] == adjacent[dir]->verticies[eAfter.first ]
	&& verticies[eBefore.second] == adjacent[dir]->verticies[eAfter.second])
		return true;

	int a, b, c, d;

	if(dir == Left || dir == Right)
	{
		a = verticies[eBefore.first ].y;
		b = verticies[eBefore.second].y;
		c = verticies[eAfter .first ].y;
		d = verticies[eAfter .second].y;
	}
	else
	{
		a  = verticies[eBefore.first ].x;
		b  = verticies[eBefore.second].x;
		c  = verticies[eAfter .first ].x;
		d  = verticies[eAfter .second].x;
	}

// a * (1 - t1) + b * t1 = c * (1 - t2) + d * t2

	float t2 = (a*(1-t1) + b*t1 - c) / (float) (d - c);

	if(0 < t2 && t2 < 1)
	{
		t1 = t2;
		return true;
	}

	return false;
}

QPoint Face::interpolatePoint(Direction after, EndPoint_t eBefore, EndPoint_t eAfter, float ratio)
{
	if(adjacent[after] == 0L)
	{
		QPoint a = verticies[eAfter.first];
		QPoint b = verticies[eAfter.second];

		return QPointF(
			a.x() * (1 - ratio) + b.x() * ratio,
			a.y() * (1 - ratio) + b.y() * ratio).toPoint();
	}
	else
	{
		auto p = getMidSegment(verticies[eBefore.first], verticies[eBefore.second], adjacent[after]->verticies[eAfter.first], adjacent[after]->verticies[eAfter.first]);

		return QPointF(
			p.first.x() * (1 - ratio) + p.second.x() * ratio,
			p.first.y() * (1 - ratio) + p.second.y() * ratio).toPoint();
	}
}

//cut the faces, and return the new verticies
std::list<Vertex *> Face::splitFaces(Face * _this, float ratio, bool direction)
{
	std::list<Vertex *> r;

	Direction before = direction? Left  : Down;
	Direction after  = direction? Right : Up;
	Direction adj0   = direction? Up    : Left;
	Direction adj1   = direction? Down  : Right;

	EndPoint_t eBefore = GetCorners(before);
	EndPoint_t eAfter= GetCorners(after);

	while(_this->continueSplit(ratio, before))
		_this = _this->adjacent[before];

	Face * splitPrev = 0L;
	Face * splitFace = 0L;

	QPoint prev_vertex = _this->interpolatePoint(before, eBefore, eAfter, ratio);

	for(;;)
	{
//define new verticies
		QPoint verticies[4];
		verticies[eBefore.first] = _this->verticies[eBefore.first];
		verticies[eAfter.first]  = _this->verticies[eBefore.first];
		verticies[eAfter.second]  = _this->interpolatePoint(after, eBefore, eAfter, ratio);
		verticies[eBefore.second] = prev_vertex;


		splitFace = new Face(verticies, _this->room_type);

//split face vertically
		splitFace->adjacent[adj1] = _this;
		splitFace->adjacent[adj0] = _this->adjacent[adj0];

		if(_this->adjacent[adj0])
		{
			_this->adjacent[adj0]->adjacent[adj1] = splitFace;
		}

//set permeability
		splitFace->permeability[before]  = _this->permeability[before];
		splitFace->permeability[after]	 = _this->permeability[after];
		splitFace->permeability[adj0]	 = _this->permeability[adj0];
		splitFace->permeability[adj1]	 = 100;

		_this->permeability[adj0]   = 100;

//set the prev/next things in the sequence
		if(splitPrev)
		{
			splitFace->adjacent[before] = splitPrev;
			splitPrev->adjacent[after] = splitFace;
		}
//if something was before this and didn't split, we need to pick which one it goes to
		else if(_this->adjacent[before])
		{
//it is in our territory, so change the pointer to us

//horizontal split, test Y component
			if(( direction && _this->adjacent[before]->verticies[eAfter.second].y <= prev_vertex.y())
//vertical split, text X component
			|| (!direction && _this->adjacent[before]->verticies[eAfter.second].x <= prev_vertex.x()))
			{
				splitFace->adjacent[before] = _this->adjacent[before];
				splitFace->adjacent[before]->adjacent[after] = splitFace;
				_this->adjacent[before] = 0L;
			}
//it is in the original's territory, do nothing.
		}


		prev_vertex = verticies[eAfter.second];
		splitPrev   = splitFace;

		if(_this->continueSplit(ratio, after))
			break;

		_this = _this->adjacent[after];
	}

	if(_this->adjacent[after])
	{
		if(( direction && _this->adjacent[after]->verticies[eBefore.second].y <= prev_vertex.y())
		|| (!direction && _this->adjacent[after]->verticies[eBefore.second].x <= prev_vertex.x()))
		{
			splitPrev->adjacent[after] = _this->adjacent[after];
			splitPrev->adjacent[after]->adjacent[before] = splitPrev;
			_this->adjacent[after] = 0L;
		}
	}

	return r;
}

bool Face::doesIntersect(QPoint pos, QSize size) const
{
	return verticies[0].isContained(pos, size)
		|| verticies[1].isContained(pos, size)
		|| verticies[2].isContained(pos, size)
		|| verticies[3].isContained(pos, size);
}

bool Face::isContained(QPoint pos, QSize size) const
{
	return verticies[0].isContained(pos, size)
		&& verticies[1].isContained(pos, size)
		&& verticies[2].isContained(pos, size)
		&& verticies[3].isContained(pos, size);
}

void Face::drawOutline(QPainter & painter) const
{
	if(isSelected())
	{
		QPoint points[4] = { verticies[0], verticies[1], verticies[3], verticies[2] };

		painter.setBrush(QBrush(Qt::white, Qt::DiagCrossPattern));
		painter.setPen(Qt::NoPen);
		painter.drawPolygon(points, 4);
		painter.setBrush(Qt::NoBrush);
	}

	painter.setPen(QPen(Qt::cyan, 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

	for(int i = 0; i < 4; ++i)
	{
		EndPoint_t edge = GetCorners((Direction) (i));
		painter.drawLine(verticies[edge.first], verticies[edge.second]);
	}
}

void Face::drawDoors(QPainter & painter) const
{
	for(int i = 0; i < 4; ++i)
	{
		if(!adjacent[i])
			continue;

		int red = (100 - permeability[i]) * 255 / 100;
		int green = permeability[i] * 255 / 100;

		painter.setPen(QPen(QColor(red, green, 0), 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

		EndPoint_t e0 = GetCorners((Direction) (i));
		EndPoint_t e1 = GetCorners((Direction) (i^1));

		auto p = getMidSegment(verticies[e0.first], verticies[e0.second], adjacent[i]->verticies[e1.first], adjacent[i]->verticies[e1.second]);
		painter.drawLine(p.first, p.second);
	}
}

void Face::drawSplit(Face * _this, QPainter & painter, float ratio, bool direction)
{
	Direction before = direction? Left  : Down;
	Direction after  = direction? Right : Up;

	EndPoint_t eBefore = GetCorners(before);
	EndPoint_t eAfter= GetCorners(after);

	while(_this->continueSplit(ratio, before))
		_this = _this->adjacent[before];

	QPen pen(Qt::magenta, 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setPen(pen);

	QPoint prev_vertex =  _this->interpolatePoint(before, eAfter, eBefore, ratio);

	for(;;)
	{
		QPoint p1 = prev_vertex;
		QPoint p2 = _this->interpolatePoint(after, eBefore, eAfter, ratio);

		painter.drawLine(p1, p2);

		if(!_this->continueSplit(ratio, after))
			break;

		_this = _this->adjacent[after];
		prev_vertex = p2;
	}

}

bool Face::isValidPosition(QPoint *)
{
	return true;
}

bool Face::isExtrudable() const
{
	if(verticies[TopLeft].isSelected)
	{
		if(verticies[TopRight].isSelected && adjacent[Up])
			return false;

		if(verticies[BottomLeft].isSelected && adjacent[Left])
			return false;
	}

	if(verticies[BottomRight].isSelected)
	{
		if(verticies[TopRight].isSelected && adjacent[Right])
			return false;

		if(verticies[BottomLeft].isSelected && adjacent[Down])
			return false;
	}

	return true;
}


bool Face::isEdgeSelected() const
{
	bool r = false;

	if(verticies[TopLeft].isSelected)
		r = verticies[TopRight].isSelected ^ verticies[BottomLeft].isSelected;

	if(verticies[BottomRight].isSelected)
		r ^= verticies[TopRight].isSelected ^ verticies[BottomLeft].isSelected;

	return r;
}


bool Face::isSelected() const
{
	return verticies[0].isSelected
		&& verticies[1].isSelected
		&& verticies[2].isSelected
		&& verticies[3].isSelected;
}

bool Face::canMoveHorizontally() const
{
	return verticies[TopLeft].isSelected == verticies[BottomLeft].isSelected
		&& verticies[TopRight].isSelected == verticies[BottomRight].isSelected;
}

static
float crossProduct(QPoint u, QPoint v)
{
	return u.x() * v.y() - u.y() * v.x();
}

static
float getSignedTriangleArea(QPoint a_left, QPoint a_right, QPoint p)
{
	return crossProduct(p - a_left, a_right - a_left) ;
}

static
bool isPointColinear(QPoint a_left, QPoint a_right, QPoint p, float epsilon)
{
	return std::fabs(getSignedTriangleArea(a_left, a_right, p)) <= epsilon;
}

static
bool areSegmentsColinear(QPoint a_left, QPoint a_right, QPoint b_left, QPoint b_right, float epsilon)
{
	return isPointColinear(a_left, a_right, b_left, epsilon) && isPointColinear(a_left, a_right, b_right, epsilon);
}

static
bool doArcsOverlap(QPoint a1, QPoint a2, QPoint b1, QPoint b2)
{
//always are all in 1 quadrant.
	float a_1 = atan2(a1.y(), a1.x());
	float a_2 = atan2(a2.y(), a2.x());
	float b_1 = atan2(b1.y(), b1.x());
	float b_2 = atan2(b2.y(), b2.x());

	if(a_1 > a_2) std::swap(a_1, a_2);
	if(b_1 > b_2) std::swap(b_1, b_2);

	return a_1 < b_2 && b_1 < a_2;
}


static
bool doSegmentsOverlap(QPoint a1, QPoint a2, QPoint b1, QPoint b2)
{
	return areSegmentsColinear(a1, a2, b1, b2, 0.001)
		&& doArcsOverlap(a1, a2, b1, b2);
}

bool Face::isOppositeEdgeAdjacent(const Face & face, int i) const
{
	if(adjacent[i] || face.adjacent[i^1])
		return false;

	EndPoint_t e0 = GetCorners((Direction) i);
	EndPoint_t e1 = GetCorners((Direction) (i^1));

	return doSegmentsOverlap(
		verticies[e0.first], verticies[e0.second],
		face.verticies[e1.first], face.verticies[e1.second]);
}

std::list<Vertex*> Face::addFace(QPoint * points)
{
	Face * face = new Face(points, 0);

	for(auto i = allFaces().begin(); *i != face; ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			if(face->isOppositeEdgeAdjacent(**i, j))
			{
				face->adjacent[j] = *i;
				(*i)->adjacent[j^1] = face;
				break;
			}
		}
	}

	std::list<Vertex*> r;

	r.push_back(&(face->verticies[0]));
	r.push_back(&(face->verticies[1]));
	r.push_back(&(face->verticies[2]));
	r.push_back(&(face->verticies[3]));

	return r;
}

bool Face::isValidPosition(QPoint p)
{
	for(auto i = allFaces().begin(); i != allFaces().end(); ++i)
	{
		if((*i)->doesContain(p))
			return false;
	}

	return true;
}

bool Face::doesContain(QPoint pos) const
{
	float min;
	Direction dir;
	return doesContain(pos, min, dir);
}

bool Face::doesContain(QPoint pos, float & min, Direction & dir) const
{
//get the center point of the polygon
	const float cx = (verticies[0].x + verticies[1].x + verticies[2].x + verticies[3].x)/4;
	const float cy = (verticies[0].y + verticies[1].y + verticies[2].y + verticies[3].y)/4;

	min = 65535;
	dir = InvalidDirection;

	for(int i = 0; i < 4; ++i)
	{
		EndPoint_t e = GetCorners((Direction) i);

		if(verticies[e.first] == verticies[e.second])
			continue;

		float dx = verticies[e.second].x - verticies[e.first].x;
		float dy = verticies[e.second].y - verticies[e.first].y;

//bigger thing is the denominator, to minimize rounding errors
		if(std::fabs(dx) > std::fabs(dy))
		{
			float m = dy / dx;
			float b = verticies[e.first].x - verticies[e.first].y * m;

			float x1 = (pos.y() * m + b) - pos.x();
			float x2 = (cy * m + b) - cx;

//if the point is going in the same direction as the center then it must be inside the polygon.
			if(x1*x2 < 0)
				return false;

			if(x1 < min)
			{
				min = x1;
				dir = (Direction) i;
			}
		}
		else
		{
			float m = dx / dy;
			float b = verticies[e.first].y - verticies[e.first].x * m;

			float y1 = (pos.x() * m + b) - pos.y();
			float y2 = (cx * m + b) - cy;

			if(y1*y2 < 0)
				return false;

			if(y1 < min)
			{
				min = y1;
				dir = (Direction) i;
			}
		}
	}

	return true;
}
