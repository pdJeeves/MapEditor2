#ifndef MAPEDITOR_H
#define MAPEDITOR_H
#include "selection.h"
#include "face.h"
#include "vertex.h"
#include "mainwindow.h"
#include <QPen>

class MapEditor : public MainWindow
{
typedef MainWindow super;
	CurrentMode mode;
	QPoint startPos;
	QPoint endPos;

	Selection selection;

	QPen   insertionPen;
	QPen   selectionPen;

public:
	MapEditor();

	void onCancel();
	void draw(QPainter &);

	void onMouseEvent(QMouseEvent*);
	void onKeyPress(CurrentMode);

	void ShowContextMenu(QPoint) {}
	QString getToolTip(QPoint) { return QString(); }
};

#endif // MAPEDITOR_H
