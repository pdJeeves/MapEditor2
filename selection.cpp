#include "selection.h"
#include "face.h"
#include "vertex.h"

void Selection::clear()
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		(*i)->isSelected = false;
	}

	verts.clear();
}

bool Selection::toggleVertex(Vertex * it)
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		if(*i == it)
		{
			it->isSelected = false;
			verts.erase(i);
			return false;
		}
	}

	it->isSelected = true;
	verts.push_back(it);
	return true;
}


bool Selection::selectVertex(Vertex * it)
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		if(*i == it)
			return false;
	}

	it->isSelected = true;
	verts.push_back(it);
	return true;
}

bool Selection::deselectVertex(Vertex * it)
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		if(*i == it)
		{
			it->isSelected = false;
			verts.erase(i);
			return true;
		}
	}

	return false;
}

bool Selection::isSelected(Vertex * v) const
{
	for(auto i = verts.begin(); i != verts.end(); ++i)
	{
		if(*i == v)
			return true;
	}

	return false;
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
	for(auto i = verts.begin(); i != verts.end();++i)
	{
		if((*i)->isEdgeSelected())
			return false;

		if(!(*i)->isExtrudable())
			return false;
	}

	return true;
}

void Selection::setSelection(std::list<Vertex *> &&list)
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
}

std::list<Vertex*> Selection::getVerticies(QPoint pos, QSize size)
{
	std::list<Vertex*> r;

	for(auto i = Face::allFaces().begin(); i != Face::allFaces().end(); ++i)
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

std::list<Vertex*> Selection::getVerticies(QPoint pos)
{
	std::list<Vertex*> r;

	for(auto i = Face::allFaces().begin(); i != Face::allFaces().end(); ++i)
	{
		if((*i)->doesContain(pos))
		{
			for(int j = 0; j < 4; ++j)
			{
				r.push_back(&((*i)->verticies[j]));
			}
		}
	}

	return r;
}

void Selection::orSelection(std::list<Vertex*> & list)
{
	for(auto i = list.begin(); i != list.end(); ++i)
	{
		if((*i)->isSelected == false)
		{
			(*i)->isSelected = true;
			verts.push_back(*i);
		}
	}
}


void Selection::andSelection(std::list<Vertex*> & list)
{
	std::list<Vertex*> anded;

	for(auto i = list.begin(); i != list.end(); ++i)
	{
		if((*i)->isSelected == true)
		{
			anded.push_back(*i);
		}
	}

	setSelection(std::move(anded));
}


void Selection::xorSelection(std::list<Vertex*> & list)
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
}






