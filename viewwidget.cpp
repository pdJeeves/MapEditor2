#include "viewwidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QCursor>
#include <QHelpEvent>
#include <QToolTip>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QEvent>

ViewWidget::ViewWidget(QWidget *parent) :
	QWidget(parent),
	window(0L)
{
}


void ViewWidget::mouseMoveEvent(QMouseEvent *)
{
	repaint();
}

void ViewWidget::mousePressEvent(QMouseEvent * event)
{
	window->onMouseEvent(event);
}

void ViewWidget::mouseReleaseEvent(QMouseEvent * event)
{
	window->onMouseEvent(event);
}

void ViewWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	window->onMouseEvent(event);
}

void ViewWidget::keyPressEvent(QKeyEvent * key)
{
	if(!window) return;

	switch(key->key())
	{
	case Qt::Key_Up:
	case Qt::Key_Down:
		window->ui->verticalScrollBar->event(key);
		break;
	case Qt::Key_Left:
	case Qt::Key_Right:
		window->ui->horizontalScrollBar->event(key);
		break;
	}
}

void ViewWidget::keyReleaseEvent(QKeyEvent * key)
{
	ViewWidget::keyPressEvent(key);
}

void ViewWidget::wheelEvent(QWheelEvent * wheel)
{
	if(!window) return;

	if(wheel->modifiers() & Qt::ControlModifier)
	{
		if(wheel->orientation() == Qt::Vertical)
		{
			double angle = wheel->angleDelta().y();
			double factor = pow(1.0015, angle);
			window->zoom *= factor;
			repaint();
		}
	}
	else if(wheel->buttons() != Qt::MidButton)
	{
		if(wheel->orientation() == Qt::Horizontal)
		{
			window->ui->horizontalScrollBar->event(wheel);
		}
		else
		{
			window->ui->verticalScrollBar->event(wheel);
		}
	}
}


bool ViewWidget::event(QEvent *event)
{
	if(event->type() != QEvent::ToolTip)
		return super::event(event);

   QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

   QString string = window->getToolTip(helpEvent->pos());

   if(!string.isEmpty())
   {
	   QToolTip::showText(helpEvent->globalPos(), string);
   }
   else
   {
	   QToolTip::hideText();
	   event->ignore();
   }

   return true;
}

void ViewWidget::paintEvent(QPaintEvent* event)
{
	static int grid_size = 8;
	static QBrush background(Qt::white);
	static QBrush square_color(Qt::gray);

	QPainter painter;
	painter.begin(this);

	painter.fillRect(event->rect(), background);
	for(int x = 0, column = 0; x < event->rect().width(); x += grid_size, ++column)
	{
		for(int y = column & 0x01? grid_size : 0; y < event->rect().height(); y += grid_size*2)
		{
			painter.fillRect(QRect(x, y, grid_size, grid_size), square_color);
		}
	}

	window->draw(painter);
	painter.end();
}
