#ifndef SELECTION_H
#define SELECTION_H
#include "direction.h"
#include <list>
#include <QPoint>
#include <QSize>

struct Vertex;
struct Face;

struct Selection
{
	std::list<Vertex*> verts;

	bool isSelected(Vertex *) const;

	bool toggleVertex(Vertex * it);
	bool selectVertex(Vertex *it);
	bool deselectVertex(Vertex * it);

	bool isExtrudable() const;
	bool canMoveHorizontally() const;

	void setSelection(std::list<Vertex *> &&list);
	void andSelection(std::list<Vertex*> & list);
	void orSelection(std::list<Vertex*> & list);
	void xorSelection(std::list<Vertex*> & list);

	void clear();

	static std::list<Vertex*> getVerticies(QPoint pos, QSize size);
	static std::list<Vertex*> getVerticies(QPoint pos);
};

#endif // SELECTION_H
