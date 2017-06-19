#include "mapeditor.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QPainter>
#include <iostream>
#include <QMessageBox>
#include "properties.h"

#include "grabcommand.h"
#include "slicecommand.h"
#include "insertioncommand.h"
#include "deletioncommand.h"

#undef min
#undef max

std::vector<MapEditor::ClipboardFace> & MapEditor::smClipboard()
{
	static std::vector<MapEditor::ClipboardFace> vec;
	return vec;
}

MapEditor::ClipboardFace::ClipboardFace(Face * face)
{
	room_type = face->room_type;
	for(int i = 0; i < 4; ++i)
		verticies[i] = face->verticies[i];

}

MapEditor::MapEditor() :
	super(),
	insertionPen(QColor(0xFF, 0xD7, 0x00, 0xFF), 1, Qt::DashLine, Qt::SquareCap, Qt::MiterJoin),
	selectionPen(QColor(0xDD, 0xDD, 0xDD, 0xFF), 1, Qt::DotLine, Qt::SquareCap, Qt::MiterJoin),
	selection(this)
{
	insertionPen.setCosmetic(true);
	mode = Cancel;
	state = 0;
	sliceFace=0L;
	repainted = false;
}

void MapEditor::onLeftDown(QMouseEvent* event)
{
	switch(mode)
	{
	case Cancel:
	{
		startPos = getMousePosition();
		endPos = startPos;

		ui->widget->setMouseTracking(true);
		mode = MouseDown;
	} break;
	case MouseDown:
		break;
	case Select:
		break;
	case Grab:
		onEndGrab();
		break;
	case AddRoom:
	{
		int x = std::min(startPos.x(), endPos.x());
		int y = std::min(startPos.y(), endPos.y());

		int w = std::abs(startPos.x() - endPos.x());
		int h = std::abs(startPos.y() - endPos.y());

		if(abs(startPos.x() - endPos.x()) < 2
		|| abs(startPos.y() - endPos.y()) < 2)
			return;

		QPoint verticies[4];
		verticies[TopLeft	 ] = QPoint(x  , y  );
		verticies[TopRight	 ] = QPoint(x+w, y  );
		verticies[BottomLeft ] = QPoint(x  , y+h);
		verticies[BottomRight] = QPoint(x+w, y+h);

		Face::List_t   faces;
		Vertex::List_t verts;

		faces.push_back(new Face(verticies, 0));
		faces.back()->setFaceList(&(selection.allFaces));


		if(0 > faces.back()->isLocationValid())
		{
			delete faces.back();
			onCancel();
			break;
		}

		for(int i = 0; i < 4;++i)
			verts.push_back(&(faces.back()->verticies[i]));

		InsertionCommand * command = new InsertionCommand(faces, verts);
		selection.selectVerticies(std::move(verts), Qt::NoModifier);

		for(int i = 0; i != 4; ++i)
		{
			faces.back()->searchForDoors((Direction) i);
		}

		commandList.push(command);
		onCancel();
		break;

	} break;
	case Extrude:
		break;
	case SelectSlice:
		if(sliceFace)
			mode = PerformSlice;
		break;
	case PerformSlice:
	{
		float ratio = sliceFace->getRatio(endPos, state);
		SliceCommand * command = new SliceCommand(this, Face::getSplittingFaces(sliceFace, ratio, state));
		selection.selectVerticies(Face::splitFaces(sliceFace, ratio, state), event->modifiers());
		commandList.push(command);
		onCancel();

	} break;
	case Paste:
	{
		switch(isTranslationValid())
		{
		default:
		case QMessageBox::Ok:
			break;
		case QMessageBox::Yes:
		{
			InsertionCommand * command = new InsertionCommand(selection.faces, selection.verts);

			for(auto i = selection.faces.begin(); i != selection.faces.end(); ++i)
			{
				for(int j = 0; j != 4; ++j)
				{
					(*i)->searchForDoors((Direction) j);
				}
			}

			//update ajdacency & permeability
			commandList.push(command);

			for(auto i = selection.verts.begin(); i != selection.verts.end(); ++i)
			{
				(*i)->stored = **i;
			}

			onCancel();
		}   break;
		case QMessageBox::Cancel:
		{
			onCancelPaste();
		} break;
		}

		ui->widget->needRepaint();
	}
	default:
		break;
	}
}

QMessageBox::StandardButton MapEditor::isTranslationValid()
{
	if(startPos == endPos)
		return QMessageBox::Cancel;

	for(auto i = selection.faces.begin(); i != selection.faces.end(); ++i)
	{
		int r = (*i)->isLocationValid();

		if(r >= 0) continue;

		QString mesg;

		switch(r)
		{
		default:
			mesg = tr("At least one room position is invalid.");
		case -1:
			mesg = tr("At least one room is not convex");
		case -2:
			mesg = tr("At least two rooms intersect");
		}

		return QMessageBox::question(this,  QGuiApplication::applicationDisplayName(), mesg,  QMessageBox::Ok|QMessageBox::Cancel);
	}

	return QMessageBox::Yes;
}

void MapEditor::onLeftUp(QMouseEvent* event)
{
	int x = std::min(startPos.x(), endPos.x());
	int y = std::min(startPos.y(), endPos.y());

	int w = std::abs(startPos.x() - endPos.x());
	int h = std::abs(startPos.y() - endPos.y());

	switch(mode)
	{
	case Cancel:
		break;
	case MouseDown:
	{
		selection.selectVerticies(selection.getVerticies(QPoint(x, y)), event->modifiers());
		onCancel();
	}	break;
	case Select:
	{
		selection.selectVerticies(selection.getVerticies(QPoint(x, y), QSize(w, h)), event->modifiers());
		onCancel();
	}	break;
	case Grab:
		onEndGrab();
		break;
	case AddRoom:
		break;
	case Extrude:
		break;
	case SelectSlice:
		break;
	case PerformSlice:
		break;
	default:
		break;
	}
}

void MapEditor::clearRooms()
{

}

void MapEditor::onMouseEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		if(event->type() == QEvent::MouseButtonPress)
			onLeftDown(event);
		if(event->type() == QEvent::MouseButtonRelease)
			onLeftUp(event);

		return;
	}

	if(event->type() == QEvent::MouseMove)
	{
		endPos = getMousePosition();

		if(mode == MouseDown)
		{
			if(endPos != startPos)
			{
				if(!selection.isSelected(startPos))
					mode = Select;
				else
				{
					onGrab();
				}
			}
		}
	}
}

void MapEditor::onGrab()
{
	lock_x = false;
	lock_y = false;

	if(c2eWalls())
	{
		lock_x = !selection.canMoveHorizontally();
	}

	if(mode != Paste)
	{
		mode = Grab;

		for(auto i = selection.verts.begin(); i != selection.verts.end(); ++i)
		{
			(*i)->stored = **i;
		}
	}
}

void MapEditor::onEndGrab()
{
	switch(isTranslationValid())
	{
	default:
	case QMessageBox::Ok:
		break;
	case QMessageBox::Yes:
	{
		GrabCommand * command = new GrabCommand(selection);

		for(auto i = selection.faces.begin(); i != selection.faces.end(); ++i)
		{
			for(int j = 0; j != 4; ++j)
			{
				(*i)->searchForDoors((Direction) j);
			}
		}

		commandList.push(command);
		mode = Cancel;

		for(auto i = selection.verts.begin(); i != selection.verts.end(); ++i)
		{
			(*i)->stored = **i;
		}

		onCancel();
	}   break;
	case QMessageBox::Cancel:
		onCancelGrab();
		break;
	}

	ui->widget->needRepaint();
}

void MapEditor::onCancelGrab()
{
	for(auto i = selection.verts.begin(); i != selection.verts.end(); ++i)
	{
		**i = (*i)->stored;
	}

	for(auto i = selection.faces.begin(); i != selection.faces.end(); ++i)
	{
		(*i)->computeBounds();
	}

	onCancel();
}

void MapEditor::onCancelPaste()
{
	Face::List_t list = selection.faces;
	selection.setSelected(Vertex::List_t());

	for(auto i = list.begin(); i != list.end(); ++i)
	{
		delete *i;
	}

	onCancel();
}


void  MapEditor::onCancel()
{


	mode = Cancel;
	state = 0;
	sliceFace = 0L;
	ui->widget->setMouseTracking(false);
	ui->widget->needRepaint();
}


void MapEditor::onKeyPress(CurrentMode m)
{
	if(m == Cancel)
	{
		if(mode == Grab)
			onCancelGrab();
		else if(mode == Paste)
			onCancelPaste();
		else
			onCancel();

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
		if(selection.isValidPosition(startPos))
		{
			ui->widget->setMouseTracking(true);
			mode = AddRoom;
		}
	}

	if(m == SelectSlice)
	{
		ui->widget->setMouseTracking(true);
		mode = SelectSlice;
	}

	if(m == Extrude
	&& selection.isExtrudable())
	{
		std::list<Vertex*> list;

		int N = selection.allFaces.size();
		for(int i = 0; i < N; ++i)
		{
			selection.allFaces[i]->extrude(list);
		}

		if(list.size())
		{
			selection.selectVerticies(std::move(list), Qt::NoModifier);
			ui->widget->setMouseTracking(true);
			onGrab();
			mode = Paste;
		}
	}

	if(m == Grab && selection.verts.size())
	{
		ui->widget->setMouseTracking(true);

		onGrab();
	}
}

void MapEditor::editCopy()
{
	if(selection.faces.size() == 0)
		return;

	std::vector<ClipboardFace> list;
	int min_x = 65535, min_y = 65535;
	int max_x =     0, max_y =     0;

	for(auto i = selection.faces.begin(); i != selection.faces.end(); ++i)
	{
		if((*i)->isSelected())
		{
			list.push_back(ClipboardFace(*i));

			for(int j = 0; j < 4; ++j)
			{
				min_x = std::min(min_x, (*i)->verticies[j].x);
				max_x = std::max(max_x, (*i)->verticies[j].x);

				min_y = std::min(min_y, (*i)->verticies[j].y);
				max_y = std::max(max_y, (*i)->verticies[j].y);
			}
		}
	}

	QPoint p((min_x+max_x)/2, (min_y+max_y)/2);

	for(auto i = list.begin(); i != list.end(); ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			i->verticies[j] -= p;
		}
	}

	smClipboard().swap(list);
}

void MapEditor::editPaste()
{
	if(mode != Cancel) return;
	if(smClipboard().empty()) return;

	startPos = getMousePosition();
	endPos = startPos;
	ui->widget->setMouseTracking(true);

	Vertex::List_t verticies;

	for(auto i = smClipboard().begin(); i != smClipboard().end(); ++i)
	{
		QPoint v[4];
		for(int j = 0; j < 4; ++j)
			v[j] = i->verticies[j] + startPos;

		Face * face = new Face(v, i->room_type);

		for(int j = 0; j < 4; ++j)
			verticies.push_back(&(face->verticies[j]));

		face->setFaceList(&selection.allFaces);
	}

	selection.selectVerticies(std::move(verticies));
	mode = Paste;
	onGrab();
}

void MapEditor::editDelete()
{
	if(selection.faces.size() == 0)
		return;

	Face::List_t list;

	for(auto i = selection.faces.begin(); i != selection.faces.end(); ++i)
	{
		if((*i)->isSelected())
		{
			list.push_back(*i);
			(*i)->setFaceList(0L);
		}
	}

	DeletionCommand * command = new DeletionCommand(list);

	for(auto i = list.begin(); i != list.end(); ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			if((*i)->adjacent[j])
				(*i)->adjacent[j]->searchForDoors((Direction) (j^1));
		}
	}

	commandList.push(command);
	selection.selectVerticies(Vertex::List_t());
}

void MapEditor::draw(QPainter & painter)
{
	repainted |= (endPos != startPos);

	QSize background = dimensions();

	if(mode == Grab || mode == Paste || mode == Extrude)
	{
		QPoint delta = endPos - startPos;

		if(lock_x) delta.setX(0);
		if(lock_y) delta.setY(0);

		delta = selection.adjustDelta(delta, background);

		for(auto i = selection.verts.begin(); i != selection.verts.end(); ++i)
		{
			**i = (*i)->stored + delta;
		}

		for(auto i = selection.faces.begin(); i != selection.faces.end(); ++i)
		{
			(*i)->validateTranslation();
			(*i)->computeBounds();
		}

	}

	super::draw(painter);

	QPoint pos = getScreenOffset();
	QSize size = getScreenSize();

	painter.translate(-pos);

	QRect screen(pos, size);


	for(auto i = selection.allFaces.begin(); i != selection.allFaces.end(); ++i)
	{
		if((*i)->doesIntersect(screen))
		{
			(*i)->drawOutline(painter);
		}
	}

	for(auto i = selection.allFaces.begin(); i != selection.allFaces.end(); ++i)
	{
		if((*i)->doesIntersect(screen))
		{
			(*i)->drawDoors(painter);
		}
	}

	if(mode == SelectSlice)
	{
		Direction dir;

		for(auto i = selection.allFaces.begin(); i != selection.allFaces.end(); ++i)
		{
			if((*i)->doesIntersect(screen)
			&& (*i)->doesContain(endPos, dir))
			{
				if(dir == InvalidDirection)
				{
					sliceFace = 0L;
					state     = false;
				}
				else
				{
					sliceFace = *i;
					state = (dir == Left || dir == Right);
					Face::drawSplit(sliceFace, painter, .5, state);
				}

				break;
			}
		}
	}

	if(mode == PerformSlice)
	{
		Face::drawSplit(sliceFace, painter, sliceFace->getRatio(endPos, state), state);
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

void MapEditor::editSelectAll()
{
	Selection::List_t list;

	for(auto i = selection.allFaces.begin(); i != selection.allFaces.end(); ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			list.push_back(&((*i)->verticies[j]));
		}
	}

	selection.selectVerticies(std::move(list), Qt::NoModifier);
}