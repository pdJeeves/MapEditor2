#include "background.h"
#include "byte_swap.h"
#include <QPainter>

Background::Background()
{
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 5; ++j)
		{
			content[i][j].map = i;
			content[i][j].channel = j;
		}
	}
}

bool Background::canSetImage(QSize size) const
{
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 5; ++j)
		{
			if(!content[i][j].image.isNull())
			{
				return content[i][j].dimensions == size;
			}
		}
	}

	return true;
}

QSize Background::dimensions() const
{
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 5; ++j)
		{
			if(!content[i][j].image.isNull())
				return content[i][j].dimensions;
		}
	}

	return QSize();
}

void Background::readLayer(FILE * file, int width, int height, int i, uint32_t * offsets, MainWindow * parent)
{
	for(auto j = 0; j < NO_LAYERS; ++j)
	{
		content[i][j].read(file, width, height, byte_swap(offsets[j]), parent);
	}
}

void Background::writeLayer(FILE * file, int i, uint32_t * offsets, MainWindow * parent)
{
	for(auto j = 0; j < NO_LAYERS; ++j)
	{
		offsets[j] = content[i][j].write(file, parent);
	}
}

void Background::clear()
{
	for(int i = 0; i < NO_MAPS; ++i)
	{
		for(int j = 0; j < NO_LAYERS; ++j)
		{
			content[i][j].clear();
		}
	}
}

bool Background::setImage(int map, int channel, QImage &&image)
{
	if(image.isNull())
	{
		return false;
	}

	content[map][channel].clear();
	content[map][channel].dimensions = image.size();

	if((image.width() & 0xFF) == 0L
	&& (image.height() & 0xFF) == 0L)
	{
		content[map][channel].image = std::move(image);
		return true;
	}

	QImage newImage((image.size().width() + 255) & 0xFF00, (image.size().height() + 255) & 0xFF00, image.format());
	newImage.fill(0);

	for(int y = 0; y < image.size().height(); ++y)
	{
		memcpy(newImage.scanLine(y), image.scanLine(y), image.bytesPerLine());
	}

	content[map][channel].image = std::move(newImage);
	return true;
}

void Background::draw(QPainter& painter, int map, int channel, QPoint offset, QSize size)
{
	if(content[map][channel].image.isNull())
		return;

	painter.drawImage(0, 0, content[map][channel].image, offset.x(), offset.y(), size.width(), size.height());
}


bool Background::onlyBackground() const
{
	if(content[0][0].image.isNull())
		return false;

	for(int i = 0; i < NO_MAPS; ++i)
	{
		for(int j = 0; j < NO_MAPS; ++j)
		{
			if(i == 0 && j == 0)
				continue;

			if(!content[i][j].image.isNull())
				return false;
		}
	}

	return true;
}
