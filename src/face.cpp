#include <cassert>
#include <QPainter>
#include "face.h"
#include "vertex.h"
#include "direction.h"
#include <iostream>
#include <QVector2D>
#include "selection.h"
#include <cmath>

Face::Face() :
	index(0L),
	m_faces(0L),
	splitFrom(0L),
	verticies{
		Vertex(*this, (Corner) 0, QPoint()),
		Vertex(*this, (Corner) 1, QPoint()),
		Vertex(*this, (Corner) 2, QPoint()),
		Vertex(*this, (Corner) 3, QPoint())},
	adjacent{0, 0, 0, 0},
	permeability{0,0,0,0},
	room_type(0)
{
	computeBounds();

	for(int i = 0; i < 4; ++i)
	{
		verticies[i].stored = verticies[i];
		verticies[i].lastValid = verticies[i];
	}
}

Face::Face(const Face & it) :
	index(0L),
	m_faces(0L),
	splitFrom(0L),
	verticies{
		Vertex(*this, (Corner) 0, it.verticies[0]),
		Vertex(*this, (Corner) 1, it.verticies[1]),
		Vertex(*this, (Corner) 2, it.verticies[2]),
		Vertex(*this, (Corner) 3, it.verticies[3])},
	adjacent{0, 0, 0, 0},
	permeability{0,0,0,0},
	room_type(0)
{
	computeBounds();

	for(int i = 0; i < 4; ++i)
	{
		verticies[i].stored = verticies[i];
		verticies[i].lastValid = verticies[i];
	}
}

Face::Face(QPoint * point, uint8_t room_type) :
	index(0L),
	m_faces(0L),
	verticies{
		Vertex(*this, (Corner) 0, point[0]),
		Vertex(*this, (Corner) 1, point[1]),
		Vertex(*this, (Corner) 2, point[2]),
		Vertex(*this, (Corner) 3, point[3])},
	adjacent{0, 0, 0, 0},
	permeability{0,0,0,0},
	room_type(room_type)
{
	computeBounds();

	for(int i = 0; i < 4; ++i)
	{
		verticies[i].stored = verticies[i];
		verticies[i].lastValid = verticies[i];
	}
}

void Face::setFaceList(List_t * list)
{
	if(list == m_faces)
	{
		return;
	}

	if(m_faces)
	{
		int N = m_faces->size()-1;
		for(int i = index; i < N; ++i)
		{
			m_faces->at(i) = m_faces->at(i+1);
			m_faces->at(i)->index = i;
		}

		m_faces->resize(N);
	}

	m_faces = list;

	if(m_faces)
	{
		index  = m_faces->size();
		m_faces->push_back(this);
	}
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

	setFaceList(0L);
}

QRect Face::GetRect(QPoint begin, QPoint end)
{
	int x, y, w, h;

	if(begin.x() < end.x())
	{
		x = begin.x();
		w = end.x() - x;
	}
	else
	{
		x = end.x();
		w = begin.x() - x;
	}

	if(begin.y() < end.y())
	{
		y = begin.y();
		h = end.y() - y;
	}
	else
	{
		y = end.y();
		h = begin.y() - y;
	}

	return QRect(x, y, w, h);
}

void Face::computeBounds()
{
	int min_x = USHRT_MAX, max_x = 0;
	int min_y = USHRT_MAX, max_y = 0;

	for(int i = 0; i < 4; ++i)
	{
		bounds[i] = GetRect(verticies[i], verticies[ClockwiseCorner((Corner) i)]);

		min_x = std::min(min_x, verticies[i].x);
		max_x = std::max(max_x, verticies[i].x);
		min_y = std::min(min_y, verticies[i].y);
		max_y = std::max(max_y, verticies[i].y);
	}

	bounds[4] = QRect(min_x, min_y, max_x - min_x, max_y - min_y);
}


typedef std::pair<float, QPoint> Pair_t;
typedef std::pair<float, int> IndexPair_t;

static
int ComparePair(const void * a, const void * b)
{
	float r = ((const Pair_t *) a)->first - ((const Pair_t *) b)->first;
	return r == 0? 0 : r < 0? -1 : 1;
}

static
int CompareIndexPair(const void * a, const void * b)
{
	float r = ((const IndexPair_t *) a)->first - ((const IndexPair_t *) b)->first;
	return r == 0? 0 : r < 0? -1 : 1;
}

static
std::pair<QPoint, QPoint> GetMidSegment(QPoint a1, QPoint a2, QPoint b1, QPoint b2)
{
	QPoint v[4] = { a1, b1, a2, b2 };

//always are all in 1 quadrant.
	IndexPair_t a[4];
	a[0] = IndexPair_t(std::atan2(a1.y(), a1.x()), 0);
	a[1] = IndexPair_t(std::atan2(a2.y(), a2.x()), 2);

	if(a1 == b1)
		a[2] = a[0];
	else
		a[2] = IndexPair_t(std::atan2(b1.y(), b1.x()), 1);

	if(a2 == b2)
		a[3] = a[1];
	else
		a[3] = IndexPair_t(std::atan2(b2.y(), b2.x()), 3);

	qsort(a, 4, sizeof(IndexPair_t), CompareIndexPair);

	if(a[1].second >  a[2].second)
		return std::make_pair(v[a[2].second], v[a[1].second]);

	return std::make_pair(v[a[1].second], v[a[2].second]);
}


#if 0
typedef std::pair<float, QPoint> Pair_t;

static
int ComparePair(const void * a, const void * b)
{
	float r = ((const Pair_t *) a)->first - ((const Pair_t *) b)->first;
	return r == 0? 0 : r < 0? -1 : 1;
}

static
std::pair<QPoint, QPoint> GetMidSegment(QPoint a1, QPoint a2, QPoint b1, QPoint b2)
{

//always are all in 1 quadrant.
	Pair_t a[4];
	a[0] = Pair_t(std::atan2(a1.y(), a1.x()), a1);
	a[1] = Pair_t(std::atan2(a2.y(), a2.x()), a2);

	if(a1 == b1)
		a[2] = a[0];
	else
		a[2] = Pair_t(std::atan2(b1.y(), b1.x()), b1);

	if(a2 == b2)
		a[3] = a[1];
	else
		a[3] = Pair_t(std::atan2(b2.y(), b2.x()), b2);

	qsort(a, 4, sizeof(Pair_t), ComparePair);
	return std::make_pair(a[1].second, a[2].second);
}
#endif

std::pair<QPoint, QPoint> Face::getMidSegment(int i) const
{
	EndPoint_t eBefore = GetCorners((Direction) i);
	EndPoint_t eAfter  = GetCorners((Direction) (i^1));

	if(adjacent[i] == 0L
	||(verticies[eAfter.first ] == adjacent[i]->verticies[eBefore.first ]
	&& verticies[eAfter.second] == adjacent[i]->verticies[eBefore.second]))
		return std::make_pair(verticies[eBefore.first], verticies[eBefore.second]);

	return GetMidSegment(verticies[eBefore.first], verticies[eBefore.second],
						   adjacent[i]->verticies[eAfter.first], adjacent[i]->verticies[eAfter.second]);
}

bool Face::continueSplit(float & t1, Direction dir) const
{
	if(adjacent[dir] == 0L)
		return false;

	return true;

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

static
float InterpolateValue(int a, int b, float ratio)
{
	return (b - a) * ratio + a;
}

QPoint Face::interpolatePoint(int i, float ratio)
{
	auto p = getMidSegment(i);

	return QPointF(
		InterpolateValue(p.first.x(), p.second.x(), ratio),
		InterpolateValue(p.first.y(), p.second.y(), ratio)).toPoint();
}

void Face::extrude(Vertex::List_t & verts)
{
	if(!isExtrudable())
	{
		return;
	}

	for(int i = 0; i < 4; ++i)
	{
		if(adjacent[i])
			continue;

		EndPoint_t e0 = GetCorners((Direction) i);
		EndPoint_t e1 = GetCorners((Direction) (i^1));

		if(!verticies[e0.first].isSelected
		|| !verticies[e0.second].isSelected)
			continue;

		QPoint p[4];
		p[e1.first ] = verticies[e0.first ];
		p[e1.second] = verticies[e0.second];
		p[e0.first ] = p[e1.first ];
		p[e0.second] = p[e1.second];

		Face * face = new Face(p, room_type);
		face->setFaceList(m_faces);

		adjacent[i] = face;
		face->adjacent[i^1] = this;

		permeability[i] = 100;
		face->permeability[i^1] = 100;

		verts.push_back(&(face->verticies[e0.first]));
		verts.push_back(&(face->verticies[e0.second]));

		if(adjacent[i^2]
		&& adjacent[i^2]->adjacent[i]
		&& adjacent[i^2]->verticies[e0.first].isSelected
		&& adjacent[i^2]->verticies[e0.second].isSelected)
		{
			Face * _it_ = adjacent[i^2]->adjacent[i];

			face->adjacent[i^2] = _it_;
			_it_->adjacent[i^3] = face;

			face->permeability[i^2] = permeability[i^2];
			_it_->permeability[i^3] = permeability[i^2];
		}
		if(adjacent[i^3]
		&& adjacent[i^3]->adjacent[i]
		&& adjacent[i^3]->verticies[e0.first].isSelected
		&& adjacent[i^3]->verticies[e0.second].isSelected)
		{
			Face * _it_ = adjacent[i^3]->adjacent[i];

			face->adjacent[i^3] = _it_;
			_it_->adjacent[i^2] = face;

			face->permeability[i^3] = permeability[i^3];
			_it_->permeability[i^2] = permeability[i^3];
		}

	}
}

//cut the faces, and return the new verticies
Vertex::List_t Face::splitFaces(Face * _this, float ratio, bool direction)
{
	Vertex::List_t r;

	Direction before = direction? Left  : Up;
	Direction after  = direction? Right : Down;
	Direction adj0   = direction? Up    : Left;
	Direction adj1   = direction? Down  : Right;

	EndPoint_t eBefore = GetCorners(before);
	EndPoint_t eAfter= GetCorners(after);

	while(_this->continueSplit(ratio, before))
		_this = _this->adjacent[before];

	Face * splitPrev = 0L;
	Face * splitFace = 0L;

	QPoint prev_vertex = _this->interpolatePoint(before, ratio);

	for(;;)
	{
//define new verticies
		QPoint verticies[4];
		verticies[eBefore.first] = _this->verticies[eBefore.first];
		verticies[eAfter.first]  = _this->verticies[eAfter.first];
		verticies[eAfter.second]  = _this->interpolatePoint(after, ratio);
		verticies[eBefore.second] = prev_vertex;

		_this->verticies[eBefore.first] = verticies[eBefore.second];
		_this->verticies[eAfter .first] = verticies[eAfter .second];

		_this->computeBounds();

//create new face
		splitFace = new Face(verticies, _this->room_type);
		splitFace->setFaceList(_this->m_faces);
		splitFace->splitFrom = _this;

//add to new vertex selection
		r.push_back(&(splitFace->verticies[eBefore.second]));
		r.push_back(&(splitFace->verticies[eAfter .second]));

		r.push_back(&(_this->verticies[eBefore.first]));
		r.push_back(&(_this->verticies[eAfter .first]));

//split face vertically
		splitFace->adjacent[adj1] = _this;
		splitFace->adjacent[adj0] = _this->adjacent[adj0];
		_this->adjacent[adj0]     = splitFace;

		if(splitFace->adjacent[adj0])
		{
			splitFace->adjacent[adj0]->adjacent[adj1] = splitFace;
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
				_this->tryConnectEdge(before);
			}
//it is in the original's territory, see if there's anything we can connect to.
			else
			{
				splitFace->tryConnectEdge(before);
			}
		}


		prev_vertex = verticies[eAfter.second];
		splitPrev   = splitFace;

		if(!_this->continueSplit(ratio, after))
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
			_this->tryConnectEdge(after);
		}
		else
		{
			splitPrev->tryConnectEdge(after);
		}
	}

	return r;
}

void Face::tryConnectEdge(Direction e)
{
	for(auto i = m_faces->begin(); i != m_faces->end(); ++i)
	{
		if(*i == this
		|| (*i)->adjacent[e^1])
			continue;

		if(isOppositeEdgeAdjacent(**i, e))
		{
			adjacent[e] = *i;
			permeability[e] = 100;
			(*i)->adjacent[e^1] = this;
			(*i)->permeability[e^1] = 100;
			break;
		}
	}
}

bool Face::doesIntersect(QRect rect) const
{
	return rect.intersects(bounds[4]);
}

bool Face::isContained(QRect rect) const
{
	return rect.contains(bounds[4]);
}

void Face::drawOutline(QPainter & painter) const
{
	QPoint points[4] = { verticies[0], verticies[1], verticies[3], verticies[2] };

	if(isSelected())
	{
		painter.setBrush(QBrush(Qt::white, Qt::DiagCrossPattern));
		painter.setPen(QPen(Qt::cyan, 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
		painter.drawPolygon(points, 4);
		painter.setBrush(Qt::NoBrush);
		return;
	}

	for(int i = 0; i < 4; ++i)
	{
		EndPoint_t edge = GetCorners((Direction) (i));

		if(!verticies[edge.first].isSelected && !verticies[edge.second].isSelected)
			continue;

		QPen pen;

		if(verticies[edge.first].isSelected && verticies[edge.second].isSelected)
			pen.setBrush(QColor(0xFF, 0xD7, 0x00, 0xFF));
		else
		{
			QLinearGradient gradient(verticies[edge.first].toPointF(), verticies[edge.second].toPointF());
			gradient.setColorAt(0, QColor(0xCF, 0xB5, 0x3B, verticies[edge.first ].isSelected? 0xFF : 0x00));
			gradient.setColorAt(1, QColor(0xCF, 0xB5, 0x3B, verticies[edge.second].isSelected? 0xFF : 0x00));
			pen.setBrush(gradient);
		}

		pen.setWidth(3);
		pen.setCosmetic(true);
		pen.setStyle(Qt::SolidLine);
		pen.setCapStyle(Qt::FlatCap);
		pen.setJoinStyle(Qt::MiterJoin);

		painter.setPen(pen);
		painter.drawLine(verticies[edge.first], verticies[edge.second]);
	}

	painter.setPen(QPen(Qt::cyan, 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
	painter.drawPolygon(points, 4);
}

void Face::drawDoors(QPainter & painter) const
{
	for(int i = 0; i < 4; ++i)
	{
		if(!adjacent[i])
			continue;

		int red  = std::min<int>(255, ((100 - permeability[i]) * 255 / 29.9));
		int green= std::min<int>(255, (permeability[i] * 255 / 58.7));

		painter.setPen(QPen(QColor(red, green, 0), 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

		auto p = getMidSegment(i);
		painter.drawLine(p.first, p.second);
	}
}

Face::List_t Face::getSplittingFaces(Face * _this, float ratio, bool direction)
{
	Face::List_t r;

	Direction before = direction? Left  : Up;
	Direction after  = direction? Right : Down;

	while(_this->continueSplit(ratio, before))
		_this = _this->adjacent[before];

	for(;;)
	{
		r.push_back(_this);

		if(!_this->continueSplit(ratio, after))
			break;

		_this = _this->adjacent[after];
	}

	return r;
}

void Face::drawSplit(Face * _this, QPainter & painter, float ratio, bool direction)
{
	Direction before = direction? Left  : Up;
	Direction after  = direction? Right : Down;

	while(_this->continueSplit(ratio, before))
		_this = _this->adjacent[before];

	QPen pen(Qt::magenta, 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
	painter.setPen(pen);

	QPoint prev_vertex =  _this->interpolatePoint(before, ratio);

	for(;;)
	{
		QPoint p1 = prev_vertex;
		QPoint p2 = _this->interpolatePoint(after, ratio);

		painter.drawLine(p1, p2);

		if(!_this->continueSplit(ratio, after))
			break;

		_this = _this->adjacent[after];
		prev_vertex = p2;
	}

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
	bool r = verticies[TopRight].isSelected && verticies[BottomLeft].isSelected;
	return !(r && (verticies[TopLeft].isSelected || verticies[BottomRight].isSelected));
}


bool Face::isSelected() const
{
	return verticies[0].isSelected
		&& verticies[1].isSelected
		&& verticies[2].isSelected
		&& verticies[3].isSelected;
}

bool  Face::isVertSelected() const
{
	return verticies[0].isSelected
		|| verticies[1].isSelected
		|| verticies[2].isSelected
		|| verticies[3].isSelected;
}

bool Face::canMoveHorizontally() const
{
	return verticies[TopLeft].isSelected == verticies[BottomLeft].isSelected
		&& verticies[TopRight].isSelected == verticies[BottomRight].isSelected;
}
/*
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
	float area = getSignedTriangleArea(a_left, a_right, p);
	return std::fabs(area) < epsilon;
}*/

static
bool areSegmentsColinear(QPoint a_left, QPoint a_right, QPoint b_left, QPoint b_right, float )
{
	if(a_left.x() == a_right.x())
		return a_left.x() == b_left.x() && a_left.x() == b_right.x();
	else if(a_left.y() == a_right.y())
		return a_left.y() == b_left.y() && a_left.y() == b_right.y();

	const float m = (a_right.y() - a_left.y()) / (float) (a_right.x() - a_left.x());
	const float b = a_left.y() - a_left.x() * m;

	const int c1 = (b_left.x() *m + b) - b_left.y();
	const int c2 = (b_right.x()*m + b) - b_right.y();

	return !c1 && !c2;

//	return isPointColinear(a_left, a_right, b_left, epsilon) && isPointColinear(a_left, a_right, b_right, epsilon);
}

static
bool doSegmentsIntersect(QPoint a_left, QPoint a_right, QPoint b_left, QPoint b_right)
{
	const float m = (a_right.y() - a_left.y()) / (float) (a_right.x() - a_left.x());
	const float b = a_left.y() - a_left.x() * m;

	const int c1 = (b_left.x() *m + b) - b_left.y();
	const int c2 = (b_right.x()*m + b) - b_right.y();

	return c1*c2 < 0;
//	return isPointColinear(a_left, a_right, b_left, epsilon) && isPointColinear(a_left, a_right, b_right, epsilon);
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
	return areSegmentsColinear(a1, a2, b1, b2, 1)
		&& doArcsOverlap(a1, a2, b1, b2);
}

bool Face::isOppositeEdgeAdjacent(const Face & face, int i) const
{
	EndPoint_t e0 = GetCorners((Direction) i);
	EndPoint_t e1 = GetCorners((Direction) (i^1));

	return doSegmentsOverlap(
		verticies[e0.first], verticies[e0.second],
		face.verticies[e1.first], face.verticies[e1.second]);
}

void Face::searchForDoors(Direction d)
{
	if(!m_faces)
		return;

	if(adjacent[d]
	&& adjacent[d]->m_faces == m_faces
	&& isOppositeEdgeAdjacent(*adjacent[d], d))
		return;

	Face * face = 0L;

	for(auto i = m_faces->begin(); i != m_faces->end(); ++i)
	{
		if(*i == this
		|| (*i)->adjacent[d^1])
			continue;

		if(isOppositeEdgeAdjacent(**i, d))
		{
			face = *i;
			break;
		}
	}

	setAdjacent(d, face);
	setPermeability(d,100);
}

Vertex::List_t Face::addFace(List_t * list, QPoint * points)
{
	Face * face = new Face(points, 0);
	face->setFaceList(list);

	for(int i = 0; i != 4; ++i)
	{
		face->searchForDoors((Direction) i);
	}

	Vertex::List_t r;

	r.push_back(&(face->verticies[0]));
	r.push_back(&(face->verticies[1]));
	r.push_back(&(face->verticies[2]));
	r.push_back(&(face->verticies[3]));

	return r;
}

bool Face::doesContain(QPoint pos) const
{
	Direction dir;
	return doesContain(pos, dir);
}

static
bool edgeTest(QPoint begin, QPoint end, QPoint c, QPoint p, float & f)
{
	float m, b, y;

	QPoint d = end - begin;

	if(std::fabs(d.y()) > std::fabs(d.x()))
	{
		begin = QPoint(begin.y(), begin.x());
		c     = QPoint(c    .y(), c    .x());
		d     = QPoint(d    .y(), d    .x());
		p     = QPoint(p    .y(), p    .x());
	}

	m = d.y() / (float) d.x();
	b = begin.y() - begin.x() * m;

	f = (p.x() * m + b) - p.y();
	y = (c.x() * m + b) - c.y();

	bool r = f*y >= 0;

	if(y)
		f /= y;
	else
		f = 0;

	return r;
}


bool Face::doesContain(QPoint pos, Direction & dir) const
{
//get the center point of the polygon
	const QPoint c = (verticies[0] + verticies[1] + verticies[2] + verticies[3])/4;

	float min = 65535;
	float runner_up = 65535;

	for(int i = 0; i < 4; ++i)
	{
		EndPoint_t e = GetCorners((Direction) i);

		if(verticies[e.first] == verticies[e.second])
			continue;

		float f;
		if(!edgeTest(verticies[e.first], verticies[e.second], c, pos, f))
			return false;

		if(f < min)
		{
			runner_up = min;
			min = f;
			dir = (Direction) i;
		}
	}

	if(std::fabs(runner_up - min) < .10)
	{
		dir = InvalidDirection;
	}

	return true;
}

static
float GetDistance(QPoint begin, QPoint end)
{
	end -= begin;
	return sqrt(end.x()*end.x() + end.y()*end.y());
}

static
float GetCompletion(QPoint begin, QPoint end, QPoint c)
{
	if(begin == end)
	{
		return .5;
	}
	else if(begin.x() == end.x())
	{
		return (c.y() - begin.y()) / (float) (end.y() - begin.y());
	}
	else if(begin.y() == end.y())
	{
		return (c.x() - begin.x()) / (float) (end.x() - begin.x());
	}

	QVector2D d(end - begin);
	QVector2D r(c - begin);

	float length = d.length();
	d.normalize();

	return (d.x()*r.x() + d.y()*r.y()) / length;
}

static
float GetBoundCompletion(QPoint begin, QPoint end, QPoint c)
{
	return std::max(0.f, std::min(1.f, GetCompletion(begin, end, c)));
}

float Face::getRatio(QPoint pos, bool direction) const
{
	EndPoint_t e0 = GetCorners(direction? Left  : Up );
	EndPoint_t e1 = GetCorners(direction? Right : Down);

	return (GetBoundCompletion(verticies[e0.first], verticies[e0.second], pos)
	    +   GetBoundCompletion(verticies[e1.first], verticies[e1.second], pos) ) / 2;
}

static
float GetDistanceSquared(QPoint begin, QPoint end)
{
	end -= begin;
	return end.x()*end.x() + end.y()*end.y();
}

bool Face::SnapToCorner(QPoint & pos, float & snap) const
{
	int corner = -1;

	for(int i = 0; i < 4; ++i)
	{
		float dist = GetDistanceSquared(verticies[i], pos);
		if(dist <= snap)
		{
			snap = dist;
			corner = i;
		}
	}

	if(corner != -1)
	{
		pos = verticies[corner];
		return true;
	}

	return false;
}

static
bool SnapTest(QPoint begin, QPoint end, QPoint c, QPoint & p, float & snap)
{
	float m, b, f, y;

	QPoint d = end - begin;
	bool swapped = false;

	if(std::fabs(d.y()) > std::fabs(d.x()))
	{
		swapped = true;

		begin = QPoint(begin.y(), begin.x());
		c     = QPoint(c    .y(), c    .x());
		d     = QPoint(d    .y(), d    .x());
		p     = QPoint(p    .y(), p    .x());
	}

	m = d.y() / (float) d.x();
	b = begin.y() - begin.x() * m;

	f = (p.x() * m + b) - p.y();
	y = (c.x() * m + b) - c.y();

	if(f*y >= 0
	|| f < snap)
	{
		snap = f;

		if(swapped)
			p = QPoint(p.x() * m + b, p.x());
		else
			p = QPoint(p.x(), p.x() * m + b);

		return true;
	}

	return false;
}

static
void SnapToEndPoint(QPoint begin, QPoint end, QPoint cursor, QPoint & point, float & snap)
{
	float c = GetCompletion(begin, end, point);

	if(0 < c && c <= .5)
	{
		point = begin;
		snap = GetDistance(cursor, point);
	}
	else if(c < 1.f)
	{
		point = end;
		snap = GetDistance(cursor, point);
	}
}


bool Face::SnapToEdge(QPoint & pos, float & snap) const
{
	//get the center point of the polygon
	const QPoint c = (verticies[0] + verticies[1] + verticies[2] + verticies[3])/4;

	Pair_t r[5] =
	{
		Pair_t(snap, pos),
		Pair_t(snap, pos),
		Pair_t(snap, pos),
		Pair_t(snap, pos)
	};

	for(int i = 0; i < 4; ++i)
	{
		EndPoint_t e = GetCorners((Direction) i);

		if(verticies[e.first] == verticies[e.second])
			continue;

		if(!SnapTest(verticies[e.first], verticies[e.second], c, r[i].second, r[i].first))
			return false;
	}

	float max = snap;

	for(int i = 0; i < 4; ++i)
	{
		if(adjacent[i])
		{
			auto p = getMidSegment(i);
			SnapToEndPoint(p.first, p.second, pos, r[i].second, r[i].first);

		}
	}

	SnapToCorner(r[4].second, r[4].first);

	for(int i = 0; i < 5; ++i)
	{
		if(r[i].first < snap)
		{
			snap = r[i].first;
			pos  = r[i].second;
		}
	}

	return snap < max;
}

static
int GetCrossProduct(QPoint a, QPoint b, QPoint c)
{
	c -= b;
	b -= a;
	return b.x()*c.y() - b.y()*c.x();
}

bool Face::isClockwise(int i) const
{
	return GetCrossProduct(
		verticies[CounterClockwiseCorner((Corner) i)],
		verticies[i],
		verticies[ClockwiseCorner((Corner) i)]) >= 0;
}


bool Face::doesContainCorner(const Face & it) const
{
	return doesContain(it.verticies[0])
		|| doesContain(it.verticies[1])
		|| doesContain(it.verticies[2])
		|| doesContain(it.verticies[3])

		|| it.doesContain(verticies[0])
		|| it.doesContain(verticies[1])
		|| it.doesContain(verticies[2])
		|| it.doesContain(verticies[3]);
}

void Face::validateTranslation()
{
	for(int j = 0; j < 4; ++j)
	{
		if(!verticies[j].isSelected)
			continue;

		if(j == TopLeft || j == BottomLeft)
			verticies[j].x = std::min(verticies[j].x, right()-1);
		else
			verticies[j].x = std::max(verticies[j].x, left()+1);

		if(j == TopLeft || j == TopRight)
			verticies[j].y = std::min(verticies[j].y, bottom()-1);
		else
			verticies[j].y = std::max(verticies[j].y, top()+1);
	}

	for(auto i = m_faces->begin(); i != m_faces->end(); ++i)
	{
		if(*i == this) continue;

		if(!bounds[5].intersects((*i)->bounds[5]))
			continue;

//for each vertex in the face
		for(int j = 0; j < 4; ++j)
		{
			if(!verticies[j].isSelected)
				continue;


			float length;
			if(!verticies[j^2].isSelected && (*i)->findIntersection(verticies[j^2], verticies[j], &length))
			{
				QVector2D r_d(verticies[j] - verticies[j^2]);
				r_d.normalize();
				r_d *= length;
				verticies[j] = r_d.toPoint() + verticies[j^2];
			}
			if(!verticies[j^1].isSelected && (*i)->findIntersection(verticies[j^1], verticies[j], &length))
			{
				QVector2D r_d(verticies[j] - verticies[j^1]);
				r_d.normalize();
				r_d *= length;
				verticies[j] = r_d.toPoint() + verticies[j^1];
			}
		}
	}
}

int Face::left() const
{
	return std::max(verticies[TopLeft].x, verticies[BottomLeft].x);
}

int Face::right() const
{
	return std::min(verticies[TopRight].x, verticies[BottomRight].x);
}

int Face::top() const
{
	return std::max(verticies[TopLeft].y, verticies[TopRight].y);
}

int Face::bottom() const
{
	return std::min(verticies[BottomLeft].y, verticies[BottomRight].y);
}

int Face::isLocationValid() const
{
	if(left() >= right()) return -3;

	if(top() >= bottom()) return -4;

	for(int i = 0; i < 4; ++i)
	{
		if(!isClockwise(i))
			return -1;
	}

//checking for intersections should be quicker than
//checking for each point being contained.
//So don't bother with that pre-check
	for(auto i = m_faces->begin(); i != m_faces->end(); ++i)
	{
		if(*i == this) continue;

		if(!bounds[5].intersects((*i)->bounds[5]))
			continue;

		for(int j = 0; j < 4; ++j)
		{
			EndPoint_t e = GetCorners((Direction) j);

			if(verticies[e.first] == verticies[e.second])
				continue;

			if((*i)->findIntersection(verticies[e.first], verticies[e.second], 0L))
				return -2;
		}
	}

	return 0;
}


bool Face::findIntersection(QPoint begin, QPoint end, float * length, float epsilon) const
{
	if(begin == end)
		return false;

	QVector2D r_p(begin);
	QVector2D r_d(end - begin);
	float distance = r_d.length();

	if(length) *length = distance;

	r_d.normalize();

	bool swapped = (r_d.x() == 0);

	if(swapped)
	{
		r_p = QVector2D(r_p.y(), r_p.x());
		r_d = QVector2D(r_d.y(), r_d.x());
	}

	for(int i = 0; i < 4; ++i)
	{
		EndPoint_t e = GetCorners((Direction) i);

		if(verticies[e.first] == verticies[e.second])
			continue;

		QVector2D s_p(verticies[e.first]);
		QVector2D s_d(verticies[e.second] - verticies[e.first]);

		if(swapped)
		{
			s_p = QVector2D(s_p.y(), s_p.x());
			s_d = QVector2D(s_d.y(), s_d.x());
		}

		float t2 = s_d.x()*r_d.y() - s_d.y()*r_d.x();

		if(!t2)
			continue;

		t2 = (r_d.x()*(s_p.y() - r_p.y()) + r_d.y()*(r_p.x()-s_p.x()))/t2;

		if(0 >= t2 || t2 >= 1)
			continue;

		t2 = (s_p.x() + s_d.x()*t2 - r_p.x()) / r_d.x();

		if(0 < t2 && t2 < distance)
		{
			if(length)
			{
				if(t2 < *length)
				{
					*length = t2;
				}
			}
			else
			{
				if(!areSegmentsColinear(begin, end, verticies[e.first], verticies[e.second], 1)
				&& (distance - t2) >= 1)
					return true;
			}
		}
	}

	if(length) return *length - distance;

	return false;
}

bool Face::isHandle(Selection &selection, QPoint point, float fuzz)
{
	return false;
}

bool Face::isEdgeSelected(Direction d) const
{
	return verticies[StartCorner(d)].isSelected && verticies[EndCorner(d)].isSelected;
}

bool Face::canSetPermeability(Direction d) const
{
	return adjacent[d];
}

int Face::getPermeability(Direction d) const
{
	if(adjacent[d])
		return permeability[d];

	return -1;
}

void Face::swapPermeability(Direction d, uint8_t & perm)
{
	uint8_t temp = permeability[d];
	setPermeability(d, perm);
	perm = temp;
}

void Face::setPermeability(Direction d, int perm)
{
	permeability[d] = perm;

	if(adjacent[d])
	{
		adjacent[d]->permeability[d^1] = perm;
	}
}

void Face::setAdjacent(Direction d, Face * face)
{
	if(adjacent[d])
	{
		adjacent[d]->adjacent[d^1] = 0L;
	}

	adjacent[d] = face;

	if(face)
	{
		if(face->adjacent[d^1])
		{
			face->adjacent[d^1]->adjacent[d] = 0L;
		}

		face->adjacent[d^1] = this;
	}
}

void Face::swapAdjacent(Direction d, Face *& face)
{
	Face * temp = adjacent[d];
	setAdjacent(d, face);
	face = temp;
}

void Face::swapVertex(Corner c, QPoint & v)
{
	QPoint temp = verticies[c];
	verticies[c] = v;
	v = temp;
}

float operator^(const QPoint & a, const QPoint & b)
{
	return a.x()*b.y() - a.y()*b.x();
}

bool Face::GetTValues(float & x, float & y, QPoint c)
{
	QPoint v0 = verticies[TopLeft];
	QPoint v1 = verticies[BottomLeft];
	QPoint v2 = verticies[TopRight];
	QPoint v3 = verticies[BottomRight];

	QPoint d10 = v1 - v0;
	QPoint d13 = v1 - v3;
	QPoint d20 = v2 - v0;
	QPoint d23 = v2 - v3;

	x = (d20.x() - d13.y()) * (d23.y() - d10.x()) - (d20.y() - (d13.x())) * (d23.x() - (d10.y()));
	y = d23.y() - d10.x();

	if(!x || ! y)
		return false;


	x = ((v0.x() - v3.y() - c.x() + c.y()) * (d23.y() - d10.x()) - (v0.y() - v3.x() + c.x() - c.y()) * (d23.x() - d10.y())) / x;
	y = (v0.x() - v3.y() + (d20.x() - d13.y())*x - c.x() + c.y()) / y;

	return true;
}

QPoint Face::FromTValues(float x, float y)
{
	QPoint v0 = verticies[0];
	QPoint v1 = verticies[1];
	QPoint v2 = verticies[2];
	QPoint v3 = verticies[3];

	QPoint mouse (v0.x() + (v1.x() - v0.x())*x*(1-y) + (v2.x() + (v3.x() - v2.x())*x)*y,
			      v0.y() + (v1.y() - v0.y())*y*(1-x) + (v2.y() + (v3.y() - v2.y())*y)*x);

	return mouse;
}

