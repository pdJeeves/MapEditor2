#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <list>
#include <QMainWindow>
#include <QFileDialog>
#include <QShortcut>
#include "face.h"
#include "background.h"
#include "commandlist.h"
#include "properties.h"
#include <QActionGroup>
#include <QTimer>

namespace Ui {
class MainWindow;
}

typedef enum CurrentMode
{
	Cancel,
	MouseDown,
	Select,
	Grab,
	AddRoom,
	Extrude,
	SelectSlice,
	PerformSlice,
	Paste,
	LockX,
	LockY
} CurrentMode;

/**
 * @brief The MainWindow class is responsible for basic UI behavior,
 * as well as loading/saving/displaying backgrounds, other behaviors
 * are stored in the MapEditor subclass.
 */

class MainWindow : public QMainWindow
{
friend class ViewWidget;
friend class Properties;
	Q_OBJECT

	float zoom;
	bool titleDirty;

	Background background;
	QString filename;
	QString name;
	QShortcut escape;
	QShortcut xKey;
	QShortcut yKey;
	QTimer autosaveTimer;

public:
	CommandList commandList;
	Properties propertiesWindow;

	explicit MainWindow(QWidget *parent = 0);
	virtual ~MainWindow();

	virtual void draw(QPainter &);
	virtual void clearRooms() = 0;

	virtual void onMouseEvent(QMouseEvent*) = 0;
	virtual void onKeyPress(CurrentMode) = 0;
	virtual void ShowContextMenu(QPoint) = 0;
	virtual QString getToolTip(QPoint) = 0;

	void onCommandPushed();

	virtual void editSelectAll() = 0;

	QPoint getMousePosition();
	QPoint getScreenOffset();
	QSize getScreenSize() const;
	bool c2eWalls() const;

	QSize dimensions() const { return background.dimensions(); }

	void changeZoom(float factor);

	QString getMapName(int) const;
	QString getChannelName(int) const;

	virtual void readRooms(FILE*,uint32_t*) = 0;
	virtual void writeRooms(FILE*,uint32_t*) = 0;

private:
	bool documentNew();
	void documentOpen();
	bool documentSave();
	bool documentSaveAs();
	bool documentClose();

	void closeEvent(QCloseEvent *event);

	void editUndo();
	void editRedo();

	virtual void editCopy() = 0;
	virtual void editPaste() = 0;
	virtual void editDelete() = 0;

	bool showBackground(int i) const;
	int showMapping() const;
	bool dimensionCheck(QSize size);

	bool loadImage(int map, int channel, const QString & name);
	void openImage(int map, int channel);

	bool loadBackground(const QString & name);
	bool loadBackground(FILE * file, const QString & name);

	bool saveBackground(const QString & filename);
	bool saveBackground(FILE * file, const QString & name);

	void initializeBackgroundFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode);

	std::vector<QAction*> m_layer;
	QActionGroup m_channel;

protected:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
