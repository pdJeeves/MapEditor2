#include "selection.h"
#include "face.h"
#include "vertex.h"
#include <iostream>
#include <QVector2D>
#include "mapeditor.h"
#include "properties.h"

Selection::Selection(MapEditor * window) :
	window(window)
{
}

void Selection::clear()
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		(*i)->isSelected = false;
	}

	verts.clear();
}

//is there at least one vertex
//such that at least one of it's edges
//
bool Selection::canMoveHorizontally() const
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		if(!(*i)->canMoveHorizontally())
		{
			return false;
		}
	}

	return true;
}

bool Selection::isExtrudable() const
{
	if(verts.empty())
		return false;

	for(auto i = faces.begin(); i != faces.end();++i)
	{
		if(!(*i)->isEdgeSelected())
			return false;

		if(!(*i)->isExtrudable())
			return false;
	}

	return true;
}

void Selection::setSelected(List_t list)
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		(*i)->isSelected = false;
	}

	verts = list;

	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		(*i)->isSelected = true;
	}

	computeFaceList();
}

void Selection::setSelection(List_t &&list)
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		(*i)->isSelected = false;
	}

	for(auto i = list.begin(); i != list.end(); ++i)
	{
		(*i)->isSelected = true;
	}

	verts = std::move(list);
	computeFaceList();
}

Selection::List_t Selection::getVerticies(QPoint pos, QSize size) const
{
	List_t r;

	for(auto i = allFaces.begin(); i != allFaces.end(); ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			Vertex * v = &((*i)->verticies[j]);
			if(v->isContained(pos, size))
				r.push_back(v);
		}
	}

	return r;
}


bool Selection::getVerticies(List_t & r, QPoint pos) const
{
	for(auto i = allFaces.begin(); i != allFaces.end(); ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			QVector2D v(pos - ((*i)->verticies[j]));

			if(v.lengthSquared() <= 36)
			{
				r.push_back(&((*i)->verticies[j]));
			}
		}
	}

	return r.size();
}

bool Selection::getEdges(List_t & r, QPoint pos) const
{
	for(auto i = allFaces.begin(); i != allFaces.end(); ++i)
	{
//if near 2 edges, we're probably trying to select a face.
		bool near_edge = false;

		for(int j = 0; j < 4; ++j)
		{
			EndPoint_t e = GetCorners((Direction) j);

			if(distSquaredToLine((*i)->verticies[e.first], (*i)->verticies[e.second], pos) < 36)
			{
				if(near_edge)
				{
					r.clear();
					return false;
				}

				near_edge = true;
				r.push_back(&((*i)->verticies[e.first]));
				r.push_back(&((*i)->verticies[e.second]));
			}
		}
	}

	return r.size();
}

bool Selection::getFaces(List_t & r, QPoint pos) const
{
	for(auto i = allFaces.begin(); i != allFaces.end(); ++i)
	{
		if((*i)->doesContain(pos))
		{
			for(int j = 0; j < 4; ++j)
			{
				r.push_back(&((*i)->verticies[j]));
			}
		}
	}

	return r.size();
}

Selection::List_t Selection::getVerticies(QPoint pos) const
{
	List_t r;

	if(!getVerticies(r, pos))
		if(!getEdges(r, pos))
			getFaces(r, pos);

	return r;
}

float Selection::distSquaredToLine(QPoint begin, QPoint end, QPoint p)
{
	float l2 = distSquared(begin, end);
	if(!l2) return distSquared(begin, p);
	float t = std::max(0.f, std::min(1.f, dotProduct(p - begin, end - begin) / 12));
	return distSquared(p, QPoint(begin.x() + t * (end.x() - begin.x()), begin.y() + t * (end.y() - begin.y()) ));
}

float Selection::distSquared(QPoint begin, QPoint end)
{
	end -= begin;
	return end.x()*end.x() + end.y()*end.y();
}

float Selection::dotProduct(QPoint u, QPoint v)
{
	return u.x()*v.x() + u.y()*v.y();
}

void Selection::orSelection(List_t && list)
{
	for(auto i = list.begin(); i != list.end(); ++i)
	{
		if((*i)->isSelected == false)
		{
			(*i)->isSelected = true;
			verts.push_back(*i);
		}
	}

	computeFaceList();
}


void Selection::andSelection(List_t && list)
{
	List_t anded;

	for(auto i = list.begin(); i != list.end(); ++i)
	{
		if((*i)->isSelected == true)
		{
			anded.push_back(*i);
		}
	}

	setSelection(std::move(anded));
}


void Selection::xorSelection(List_t && list)
{
	for(auto i = list.begin(); i != list.end(); ++i)
	{
		if((*i)->isSelected == false)
		{
			(*i)->isSelected = true;
			verts.push_back(*i);
		}
		else
		{
			(*i)->isSelected = false;
			verts.remove(*i);
		}
	}

	computeFaceList();
}


void Selection::draw(QPainter & painter)
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		if(!(*i)->face.isSelected())
		{
			(*i)->draw(painter);
		}
	}
}

VertexState Selection::getVertexLocations() const
{
	VertexState vec;
	vec.reserve(verts.size());

	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		vec.push_back(VertexPos(*i, **i));
	}

	return vec;
}

void Selection::setVertexLocations(const VertexState & vec )
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		(*i)->isSelected = false;
	}

	verts.clear();

	for(auto i = vec.begin(); i != vec.end(); ++i)
	{
		verts.push_back(i->first);
		i->first->isSelected = true;

		*(i->first) = i->second;
	}
}

void Selection::selectVerticies(Selection::List_t && v, Qt::KeyboardModifiers mod)
{
	if(mod == Qt::ControlModifier)
		xorSelection(std::move(v));
	else if(mod == Qt::ShiftModifier)
		orSelection(std::move(v));
	else if(mod == (Qt::ShiftModifier|Qt::ControlModifier))
		andSelection(std::move(v));
	else
		setSelection(std::move(v));
}

bool Selection::isSelected(QPoint pos) const
{
	List_t v = getVerticies(pos);

	for(auto i = v.begin(); i != v.end(); ++i)
	{
		if(!(*i)->isSelected)
			return false;
	}

	return v.size();
}


void Selection::computeFaceList()
{
	faces.clear();

	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		faces.push_back(&((*i)->face));
	}

	std::sort(faces.begin(), faces.end());
	auto itr = std::unique (faces.begin(), faces.end());
    faces.resize( std::distance(faces.begin(),itr) );

	window->propertiesWindow.onSelectionChanged();
}


QPoint Clamp(QPoint min, QPoint mid, QPoint max)
{
	return QPoint(
		std::max(min.x(), std::min(mid.x(), max.x())),
		std::max(min.y(), std::min(mid.y(), max.y())));
}

QPoint Selection::adjustDelta(QPoint delta, QSize size) const
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		QPoint p = (*i)->stored;
		delta = Clamp(-p, delta, QPoint(size.width() - p.x(), size.height() - p.y()));
	}

	return delta;
}

bool Selection::isOnlyFaces() const
{
	for(auto i = faces.begin(); i != faces.end(); ++i)
	{
		if(!(*i)->isSelected())
		{
			for(int j = 0; j < 4; ++j)
			{
				if(!(*i)->verticies[j].isSelected)
					continue;

				if((*i)->verticies[ClockwiseCorner((Corner) j)].isSelected)
				{
					if(!(*i)->adjacent[ClockwiseEdge((Corner) j)]
					|| !(*i)->adjacent[ClockwiseEdge((Corner) j)]->isSelected())
						return false;
				}
			}
		}
	}

	return true;
}


bool Selection::isValidPosition(QPoint p) const
{
	for(auto i = allFaces.begin(); i != allFaces.end(); ++i)
	{
		if((*i)->doesContain(p))
			return false;
	}

	return true;
}

