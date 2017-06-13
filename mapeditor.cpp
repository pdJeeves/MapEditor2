#include "mapeditor.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QPainter>
#include <iostream>

#undef min
#undef max

MapEditor::MapEditor() :
	super(),
	insertionPen(QColor(0xFF, 0xD7, 0x00, 0xFF), 1, Qt::DashLine, Qt::SquareCap, Qt::MiterJoin),
	selectionPen(QColor(0xDD, 0xDD, 0xDD, 0xFF), 1, Qt::DotLine, Qt::SquareCap, Qt::MiterJoin)
{
	insertionPen.setCosmetic(true);
	mode = Cancel;


}

void MapEditor::onMouseEvent(QMouseEvent* event)
{
	if(mode == Cancel)
		return;

	int x = std::min(startPos.x(), endPos.x());
	int y = std::min(startPos.y(), endPos.y());

	int w = std::abs(startPos.x() - endPos.x());
	int h = std::abs(startPos.y() - endPos.y());

	if(mode == AddRoom
	&& event->type() == QEvent::MouseButtonPress
	&& event->button() == Qt::LeftButton)
	{
		if(abs(startPos.x() - endPos.x()) < 2
		|| abs(startPos.y() - endPos.y()) < 2)
			return;

		QPoint verticies[4];
		verticies[TopLeft	 ] = QPoint(x  , y  );
		verticies[TopRight	 ] = QPoint(x+w, y  );
		verticies[BottomLeft ] = QPoint(x  , y+h);
		verticies[BottomRight] = QPoint(x+w, y+h);

		if(Face::isValidPosition(verticies))
		{
			selection.setSelection(Face::addFace(verticies));
			onCancel();
		}
	}
	if(mode == Select
	&& event->type() == QEvent::MouseButtonPress
	&& event->button() == Qt::LeftButton)
	{
		std::list<Vertex*> list = Selection::getVerticies(QPoint(x, y), QSize(w, h));

		if(event->modifiers() == Qt::ControlModifier)
			selection.xorSelection(list);
		if(event->modifiers() == Qt::ShiftModifier)
			selection.orSelection(list);
		if(event->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
			selection.andSelection(list);
		else
			selection.setSelection(std::move(list));

		onCancel();
	}

}

void  MapEditor::onCancel()
{
	mode = Cancel;
	ui->widget->setMouseTracking(false);
	ui->widget->repaint();
}


void MapEditor::onKeyPress(CurrentMode m)
{
	if(m == Cancel)
	{
		if(mode == AddRoom || mode == Select)
		{
			onCancel();
		}

		return;
	}

	if(mode != Cancel)
	{
		return;
	}

	startPos = getMousePosition();
	endPos = startPos;

	if(m == AddRoom)
	{
		if(Face::isValidPosition(startPos))
		{
			ui->widget->setMouseTracking(true);
			mode = AddRoom;
		}
	}

	if(m == Select)
	{
		ui->widget->setMouseTracking(true);
		mode = Select;
	}

	if(m == SelectSlice)
	{
		ui->widget->setMouseTracking(true);
		mode = SelectSlice;
	}

}

void MapEditor::draw(QPainter & painter)
{
	endPos = getMousePosition();

	super::draw(painter);

	QPoint pos = getScreenOffset();
	QSize size = getScreenSize();

	painter.translate(-pos);

	for(auto i = Face::allFaces().begin(); i != Face::allFaces().end(); ++i)
	{
		if((*i)->doesIntersect(pos, size))
		{
			(*i)->drawOutline(painter);
		}
	}

	for(auto i = Face::allFaces().begin(); i != Face::allFaces().end(); ++i)
	{
		if((*i)->doesIntersect(pos, size))
		{
			(*i)->drawDoors(painter);
		}
	}

	if(mode == SelectSlice || mode == PerformSlice)
	{
		float min;
		Direction dir;

		for(auto i = Face::allFaces().begin(); i != Face::allFaces().end(); ++i)
		{
			if((*i)->doesIntersect(pos, size)
			&& (*i)->doesContain(endPos, min, dir))
			{
				float ratio = min;
				bool  direction = (dir == Left || dir == Right);

				if(mode == SelectSlice)
				{
					Face::drawSplit(*i, painter, .5, direction);
					break;
				}

			}
		}
	}


	if(mode == AddRoom || mode == Select)
	{
		if(mode == AddRoom)
			painter.setPen(insertionPen);
		if(mode == Select)
			painter.setPen(selectionPen);

		painter.setBrush(Qt::NoBrush);
		QPoint verticies[4] =
		{
			QPoint(startPos.x(), startPos.y()),
			QPoint(startPos.x(), endPos  .y()),
			QPoint(endPos  .x(), endPos  .y()),
			QPoint(endPos  .x(), startPos.y())
		};

		painter.drawPolygon(verticies, 4);
	}
}
