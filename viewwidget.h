#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QWidget>

class MainWindow;

class ViewWidget : public QWidget
{
typedef QWidget super;

	Q_OBJECT
public:
	explicit ViewWidget(QWidget *parent = 0);
	MainWindow * window;

	void mouseMoveEvent 		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mousePressEvent		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mouseReleaseEvent		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mouseDoubleClickEvent	(QMouseEvent * event)	Q_DECL_OVERRIDE;

	void wheelEvent				(QWheelEvent * event)   Q_DECL_OVERRIDE;
	void keyPressEvent			(QKeyEvent * event)		Q_DECL_OVERRIDE;
	void keyReleaseEvent		(QKeyEvent * event)		Q_DECL_OVERRIDE;

	bool event					(QEvent *event)			Q_DECL_OVERRIDE;

	void paintEvent				(QPaintEvent * event)	Q_DECL_OVERRIDE;
};

#endif // VIEWWIDGET_H
