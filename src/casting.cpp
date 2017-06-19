#if 0
#include "face.h"
#include <QVector2D>


template<class T>
static void SwapQThing(T & it)
{
	it = T(it.y(), it.x());
}

QPoint Face::SingleCast(QPoint begin, QPoint end)
{
	QRect box = GetRect(begin, end);

	QVector2D r_p(begin);
	QVector2D r_d(end - begin);

	if(r_d.x() == 0 && r_d.y() == 0)
		return end;

	float length = r_d;
	r_d.normalize();

	bool swapped = false;
	if(r_d.x() == 0)
	{
		SwapQThing(r_p);
		SwapQThing(r_d);
		swapped = true;
	}

	for(auto i = allFaces().begin(); i != allFaces().end(); ++i)
	{
		if(box.intersects((*i)->bounds[4]))
		{
			length = (*i)->performCast(box, r_p, r_d, length, swapped);
		}
	}

	return (r_d * length).toPoint();
}

QPoint Face::DualCast(QPoint begin, QPoint end)
{
	QRect box = GetRect(begin, end);

	QVector2D r_p(begin);
	QVector2D r_d(end - begin);

	if(r_d.x() == 0 && r_d.y() == 0)
		return end;

	float length = r_d;
	r_d.normalize();

	bool swapped = false;
	if(r_d.x() == 0)
	{
		SwapQThing(r_p);
		SwapQThing(r_d);
		swapped = true;
	}

	for(auto i = allFaces().begin(); i != allFaces().end(); ++i)
	{
		if(box.intersects((*i)->bounds[4]))
		{
			length = (*i)->performCast(box, r_p, r_d, length, swapped);
		}
	}

	return (r_d * length).toPoint();
}


static
float DoCast(QPoint s_p, QPoint s_d, QVector2D r_p, QVector2D r_d, float length)
{
	float t2 = (s_d.x()*r_d.y() - s_d.y()*r_d.x());

	if(t2)
	{
		t2 = (r_d.x() * (s_p.y() - r_p.y()) + r_d.y() * (s_p.x() - r_p.x())) / t2;

		if(0 < t2 && t2 < 1)
		{
			float t1 = (s_p.x() + s_d.x()*t2 - r_p.x()) / r_d.x();

			if(t1 > 0 && t1 < length)
			{
				return t1;
			}
		}
	}

	return length;
}

float Face::performCast(QRect box, QVector2D r_p, QVector2D r_d, float length, bool swapped)
{
	QPoint points;
//copy to stack for better caching
	for(int i = 0; i < 4; ++i)
	{
		points[i] = verticies[i];
	}

	for(int i = 0; i < 4; ++i)
	{
		if(!bounds[i].intersects(box))
			continue;

		EndPoint_t e = GetCorners((Direction) i);

		QPoint s_p = (QPoint) verticies[e.first];
		QPoint s_d = verticies[e.second] - s_p;

		if(swapped)
		{
			SwapQThing(s_p);
			SwapQThing(s_d);
		}

		length = DoCast(s_p, s_d, r_p, r_d, length);
	}

	return length;
}

#endif
