#ifndef BACKGROUND_H
#define BACKGROUND_H
#include <QPoint>
#include <QSize>
#include "backgroundimage.h"

class QWidget;
class QPainter;

#define NO_MAPS    3
#define NO_LAYERS  5

class Background
{
public:
	Background();

	BackgroundImage content[NO_MAPS][NO_LAYERS];

	bool canSetImage(QSize size) const;
	QSize dimensions() const;

	void readLayer(FILE * file, int width, int height, int i, uint32_t * offsets, QWidget * parent);
	void writeLayer(FILE * file, int i, uint32_t * offsets, QWidget * parent);

	bool loadBackground();
	bool loadParallaxLayer();

	void clear();

	bool setImage(int map, int channel, QImage && image);
	void draw(QPainter& painter, int map, int channel, QPoint offset, QSize size);

	bool onlyBackground() const;
};

#endif // BACKGROUND_H
