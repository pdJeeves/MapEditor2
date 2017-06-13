#include "backgroundimage.h"
#include <QApplication>
#include <QProgressDialog>
#include <cstdio>
#include <squish.h>
#include "byte_swap.h"
#include <iostream>

//helper classes used by compression
struct BLOCK_128
{
	uint8_t data[16];

	operator bool() const
	{
		return
		(data[0] | (data[1] & ~0x05) | data[2] | data[3] |
		 data[4] | data[5] | data[6] | data[7] |
		 data[8] | data[9] | data[10] | data[11] |
		 data[12] | data[13] | data[14] | data[15]);
	}
};

struct BLOCK_64
{
	uint8_t data[8];

	operator bool() const
	{
		static uint8_t cmp[] = { 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF };
		auto r = memcmp(data, cmp, 8);
		return r;
	}
};

BackgroundImage::BackgroundImage() :
	map(0),
	channel(0)
{

}

uint8_t BackgroundImage::scanAlpha(const uint16_t x, const uint16_t y)
{
	uint8_t min = 255;
	uint8_t max = 0;

	if(qAlpha(image.pixel(x, y)) == qAlpha(image.pixel(x+3, y+3)))
	{
		return qAlpha(image.pixel(x, y)) > 16;
	}

	for(uint16_t _y = y; _y < y+4; ++_y)
	{
		for(uint16_t _x = x; _x < x+4; ++_x)
		{
			uint8_t c = qAlpha(image.pixel(_x, _y));

			min = std::min(min, c);
			max = std::max(max, c);
		}
	}

	if(((min >> 4) == 0 || (max >> 4) == 0x0F))
		return 1;

	if(0x01 > (max - min))
		return 5;

	return 3;
}

uint8_t BackgroundImage::getCompressionType(int x0, int y0)
{
	if(!image.hasAlphaChannel())
		return 1;

	uint16_t compression_1 = 0;
	uint16_t compression_3 = 0;
	uint16_t compression_5 = 0;

	for(uint16_t y = 0; y < 256 && y + y0 < image.height(); y += 4)
	{
		for(uint16_t x = 0; x < 256 && x + x0 < image.width(); x += 4)
		{
			switch(scanAlpha(x, y))
			{
			case 1:
				++compression_1;
				break;
			case 3:
				++compression_3;
				break;
			case 5:
				++compression_5;
				break;
			}
		}
	}

	if(!(compression_1|compression_3|compression_5))
	{
		return 0;
	}

	if(compression_1 < (compression_3+compression_5)/8)
	{
		if(compression_3 > 8 && compression_3 > compression_5)
			return 3;
		else if(compression_5 > 8 && compression_5 >= sqrt(compression_3)/2)
			return 5;
	}

	return 1;
}

uint32_t packBytes(uint32_t a, uint32_t b, uint32_t c,uint32_t d)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return (((((d << 8) | c ) << 8) | b) << 8) | a;
#else
	return (((((a << 8) | b ) << 8) | c) << 8) | d;
#endif
}

std::vector<uint32_t> BackgroundImage::GetRawData(uint16_t x0, uint16_t y0)
{
	std::vector<uint32_t> uncompressed_image(65536, 0);

	for(uint32_t i = 0; i < 0x10000; ++i)
	{
		int x = (i & 0xFF);
		int y = (i >> 8);

		auto c = image.pixel(x + x0, y + y0);
		if(!qAlpha(c))
			continue;

		if(channel >= 2)
			c = (uint32_t) qRgba(qGreen(c),0 ,qRed(c), 0);

		uncompressed_image[i] = packBytes(qRed(c), qGreen(c), qBlue(c), qAlpha(c));
	}

	return uncompressed_image;
}

template<class T>
void vectorWrite(std::vector<uint8_t> & file, int size, T * data)
{
	int N = file.size();
	file.resize(file.size() + sizeof(T)*size);
	memcpy(&file[N], data, size*sizeof(T));
}

std::vector<uint8_t> BackgroundImage::compressDtx1(std::vector<uint32_t> & uncompressed_image)
{
	std::vector<uint8_t> data;

	uint32_t size = GetStorageRequirements(256, 256, squish::kDxt1);

	{
		uint8_t compression_type = 1;
		vectorWrite(data, 1, &compression_type);
		uint32_t length = byte_swap(size);
		vectorWrite(data, 1, &length);
	}

	std::vector<BLOCK_64> blocks(size >> 3);
	squish::CompressImage((uint8_t*) uncompressed_image.data(), 256, 256, (void*) blocks.data(),
		squish::kDxt1 | squish::kColourIterativeClusterFit);

#if !defined(C16_COMPRESSION)
	uint32_t length = 0;
	vectorWrite(data, 1, &length);
	length = blocks.size();
	length = byte_swap(size);
	vectorWrite(data, 1, &length);
	vectorWrite(data, blocks.size(), &blocks[0]);
#else
	for(size_t i = 0; i < blocks.size(); )
	{
		uint32_t length;
		for(length = 0; i < blocks.size() && !blocks[i]; ++i, ++length) {}

		length <<= 3;
		length = byte_swap(length);
		vectorWrite(data, 1, &length);
		if(i >= size)
		{
			break;
		}

		for(length = 0; i+length < blocks.size() && blocks[i+length]; ++length)  {}

		{
			uint32_t len = length << 3;
			len = byte_swap(len);
			vectorWrite(data, 1, &len);
		}

		vectorWrite(data, length, &blocks[i]);
		i += length;
	}
#endif

	return data;
}

std::vector<uint8_t> BackgroundImage::compressDtx3(std::vector<uint32_t> & uncompressed_image, uint8_t compression_type)
{
	std::vector<uint8_t> data;

	uint32_t size = squish::GetStorageRequirements(256, 256, compression_type == 3? squish::kDxt3 : squish::kDxt5);

	{
		vectorWrite(data, 1, &compression_type);
		uint32_t length = byte_swap(size);
		vectorWrite(data, 1, &length);
	}

	std::vector<BLOCK_128> blocks(size >> 4);

	squish::CompressImage((uint8_t*) uncompressed_image.data(), 256, 256, (void*) blocks.data(), compression_type | squish::kColourIterativeClusterFit | squish::kWeightColourByAlpha);

#if !defined(C16_COMPRESSION)
	{
		uint32_t length = 0;
		vectorWrite(data, 1, &length);
		length = byte_swap(size);
		vectorWrite(data, 1, &length);
		vectorWrite(data, size, &blocks[0]);
	}
#else
	for(size_t i = 0; i < blocks.size(); )
	{
		uint32_t length = 0;
		for(length = 0; i < blocks.size() && !blocks[i]; ++i, ++length) {}

		length <<= 4;
		length = byte_swap(length);
		vectorWrite(data, 1, &length);

		if(i >= 0x10000)
		{
			break;
		}

		for(length = 0; i+length < blocks.size() && blocks[i+length]; ++length)  {}

		{
			uint32_t len = length << 4;
			len = byte_swap(len);
			vectorWrite(data, 1, &len);
		}

		vectorWrite(data, length, &blocks[i]);
		i += length;
	}
#endif

	return data;
}

std::vector<uint8_t> BackgroundImage::compressTile(int x0, int y0)
{
	uint8_t compression_type = getCompressionType(x0, y0);

	std::vector<uint32_t> uncompressed_image = GetRawData(x0, y0);

	if(compression_type == 0)
	{
		std::vector<uint8_t> data;
		data.push_back(0);
		return data;
	}

	if(compression_type == 1)
	{
		return compressDtx1(uncompressed_image);
	}

	return compressDtx3(uncompressed_image, compression_type);
}

uint32_t BackgroundImage::write(FILE * file, QWidget * parent)
{
	if(image.isNull())
	{
		return 0;
	}

	std::vector<uint32_t> image_offsets(totalTiles(), 0);
	compressed.resize(totalTiles());

	const uint32_t r = ftell(file);
	fseek(file, image_offsets.size()*4, SEEK_CUR);

	QProgressDialog progress(parent->tr("Compressing Map %1 Channel %2...").arg(map).arg(channel), 0L, 0, totalTiles(), parent);

	int32_t tiles_written = 0;

	for(int x = 0; x < tiles().width(); ++x)
	{
		for(int y = 0; y < tiles().height(); ++y)
		{
			int tile = x*tiles().height() + y;

			if(compressed[tile].size() == 0)
			{
				compressed[tile] = compressTile(x*256, y*256);
			}

			if(compressed[tile].size() > 1)
			{
				tiles_written += 1;
				image_offsets[tile] = byte_swap((uint32_t) ftell(file));
				fwrite(compressed[tile].data(), 1, compressed[tile].size(), file);
			}

			progress.setValue(tile);

			if((tile & 0x03) == 0)
			{
				qApp->processEvents();
			}
		}
	}

	size_t pos = ftell(file);
	fseek(file, r, SEEK_SET);

	if(tiles_written)
	{
		fwrite(&image_offsets[0], sizeof(uint32_t), image_offsets.size(), file);
		fseek(file, pos, SEEK_SET);
	}
	else
	{
		return 0;
	}

	return r;
}

template<class T>
T vectorRead(std::vector<uint8_t> & buff, FILE * file)
{
	T t;
	fread(&t, sizeof(T), 1, file);

	int N = buff.size();
	buff.resize(N + sizeof(T));
	memcpy(&buff[N], &t, sizeof(T));

	return byte_swap(t);
}

std::vector<QRgb> BackgroundImage::readTile(FILE * file, std::vector<uint8_t> & compressed)
{
	uint8_t compression_type = vectorRead<uint8_t>(compressed, file);
	uint32_t size = vectorRead<uint32_t>(compressed, file);

	std::vector<uint8_t> image_data(size, 0);

	for(uint32_t l = 0; l < size; )
	{
		uint32_t len = vectorRead<uint32_t>(compressed, file);;

		switch(compression_type)
		{
		case 1:
			for(; l+8 <= size && len; len -= 8, l += 8)
			{
				memset(image_data.data() + (l+4), 0xFF, 4);
			}
			break;
		case 3:
			break;
		case 5:
			for(; l+16 <= size && len; len -= 16, l += 16)
			{
				image_data[l+1] = 0x05;
			}
			break;
		}

		l += len;

		if(l >= size)
		{
			break;
		}

		len = vectorRead<uint32_t>(compressed, file);
		fread(&image_data[l], 1, len, file);

		int N = compressed.size();
		compressed.resize(N + len);
		memcpy(&compressed[N], &image_data[l], len);

		l += len;
	}

	switch(compression_type)
	{
	default:
		compression_type = squish::kDxt1;
		break;
	case 3:
		compression_type = squish::kDxt3;
		break;
	case 5:
		compression_type = squish::kDxt5;
		break;
	}

	std::vector<QRgb> uncompressed_image(65536);
	squish::DecompressImage((uint8_t*) uncompressed_image.data(), 256, 256, image_data.data(), compression_type);
	return uncompressed_image;
}


void BackgroundImage::clear()
{
	dimensions = QSize();
	std::vector< std::vector<uint8_t> > temp;
	compressed.swap(temp);
	image = QImage();
}

void BackgroundImage::read(FILE * file, uint16_t width, uint16_t height, uint32_t offset, QWidget * parent)
{
	clear();

	if(!offset)
		return;

	dimensions = QSize(width, height);

	int tiles_x = (width + 255) >> 8;
	int tiles_y = (height + 255) >> 8;

	image = QImage(QSize(tiles_x << 8, tiles_y << 8), QImage::Format_ARGB32);
	image.fill(0);

	compressed.resize(tiles_x*tiles_y);

	std::vector<uint32_t> image_offsets(tiles_x*tiles_y);
	fseek(file, offset, SEEK_SET);
	fread(&image_offsets[0], sizeof(uint32_t), image_offsets.size(), file);

	QProgressDialog progress(parent->tr("Loading Map %1 Channel %2...").arg(map).arg(channel), 0L, 0, image_offsets.size(), parent);

	std::cerr << image.width() << "\t" << image.height() << std::endl;

	for(uint16_t tile = 0; tile < image_offsets.size(); ++tile)
	{
		if(!image_offsets[tile])
			continue;

		int x0 = (tile / tiles_y) << 8;
		int y0 = (tile % tiles_y) << 8;

		fseek(file, byte_swap(image_offsets[tile]), SEEK_SET);
		std::vector<QRgb> uncompressed_image = readTile(file, compressed[tile]);

		for(int l = 0; l < 0x10000; ++l)
		{
			QRgb c = uncompressed_image[l];
			if(c)
			{
				c = packBytes(qRed(c), qGreen(c), qBlue(c), qAlpha(c));

				if(channel >= 2)
				{
					c = qRgba(qBlue(c), qRed(c), 0, 255);
				}
			}

			uint8_t x = l & 0xFF;
			uint8_t y = l >> 8;

			image.setPixel(x0 + x, y0 + y, c);
		}


		progress.setValue(tile);

		if((tile & 0x03) == 0)
		{
			qApp->processEvents();
		}
	}
}

QSize BackgroundImage::tiles() const
{
	return QSize((image.width() + 255) >> 8, (image.height()+ 255) >> 8);
}

size_t BackgroundImage::totalTiles() const
{
	return tiles().width() * tiles().height();
}
