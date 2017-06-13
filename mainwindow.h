#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <list>
#include <QMainWindow>
#include <QFileDialog>
#include <QShortcut>
#include "face.h"
#include "background.h"

namespace Ui {
class MainWindow;
}

typedef enum CurrentMode
{
	AddRoom,
	Select,
	Grab,
	Extrude,
	SelectSlice,
	PerformSlice,
	Cancel
} CurrentMode;

class MainWindow : public QMainWindow
{
friend class ViewWidget;
	Q_OBJECT

	float zoom;

	Background background;
	QString filename;
	QShortcut escape;

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	virtual void draw(QPainter &);

	virtual void onMouseEvent(QMouseEvent*) = 0;
	virtual void onKeyPress(CurrentMode) = 0;
	virtual void ShowContextMenu(QPoint) = 0;
	virtual QString getToolTip(QPoint) = 0;

	QPoint getMousePosition();
	QPoint getScreenOffset();
	QSize getScreenSize() const;

private:
	bool documentClose();
	bool documentNew();
	void documentOpen();
	void documentSave();
	void documentSaveAs();


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

protected:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
