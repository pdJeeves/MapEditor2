#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H
#include <QImage>
#include <vector>

typedef struct _IO_FILE FILE;

class BackgroundImage
{

	uint8_t scanAlpha(const uint16_t x, const uint16_t y);
	uint8_t getCompressionType(int x0, int y0);
	std::vector<uint32_t> GetRawData(uint16_t x0, uint16_t y0);

	std::vector<uint8_t> compressDtx1(std::vector<uint32_t> & uncompressed_image);
	std::vector<uint8_t> compressDtx3(std::vector<uint32_t> & uncompressed_image, uint8_t compression_type);

	std::vector<uint8_t> compressTile(int x0, int y0);
	std::vector<QRgb> readTile(FILE * file, std::vector<uint8_t> & compressed);
public:
	BackgroundImage();

	int map;
	int channel;
	QSize dimensions;

	QImage image;
	std::vector< std::vector<uint8_t> > compressed;

	uint32_t write(FILE * file, QWidget * parent);
	void read(FILE * file, uint16_t tiles_x, uint16_t tiles_y, uint32_t offset, QWidget * parent);

	QSize tiles() const;
	size_t totalTiles() const;

	void clear();
};

#endif // BACKGROUNDIMAGE_H
