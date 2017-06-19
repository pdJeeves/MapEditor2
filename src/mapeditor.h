#ifndef MAPEDITOR_H
#define MAPEDITOR_H
#include "commandlist.h"
#include "selection.h"
#include "face.h"
#include "vertex.h"
#include "mainwindow.h"
#include <QPen>
#include <QMessageBox>

class MapEditor : public MainWindow
{
typedef MainWindow super;
friend class Properties;
	CurrentMode mode;
	QPoint startPos;
	QPoint endPos;

	int       state;
	Face    * sliceFace;

	QPen   insertionPen;
	QPen   selectionPen;

	QPoint new_room[4];

	bool lock_x;
	bool lock_y;
	bool repainted;

	QMessageBox::StandardButton isTranslationValid();
	void onGrab();
	void onEndGrab();
	void onCancelGrab();
	void onCancelPaste();

	struct ClipboardFace
	{
		ClipboardFace(Face * face);
		QPoint verticies[4];
		uint8_t room_type;
	};

	static std::vector<ClipboardFace> & smClipboard();

public:
	Selection selection;

	MapEditor();

	void editCopy();
	void editPaste();
	void editDelete();

	void onCancel();
	void draw(QPainter &);

	void clearRooms();

	void onMouseEvent(QMouseEvent*);
	void onLeftDown(QMouseEvent*);
	void onLeftUp(QMouseEvent*);

	void onKeyPress(CurrentMode);

	void ShowContextMenu(QPoint) {}
	QString getToolTip(QPoint) { return QString(); }

	void editSelectAll();

	bool isValidPosition(QPoint p);
};

#endif // MAPEDITOR_H
