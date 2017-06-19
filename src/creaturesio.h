#ifndef CREATURESIO_H
#define CREATURESIO_H
#include <QImage>
#include <cstdio>

class MainWindow;
class MapEditor;

QImage importS16(FILE * file, MainWindow * parent);
QImage importBlk(FILE * file, MainWindow * parent);
QImage importSpr(FILE * file, MainWindow * parent);

bool exportSpr(const QImage & image, FILE * file, MainWindow * parent);
bool exportS16(const QImage & image, FILE * file, MainWindow * parent);
bool exportBlk(const QImage & image, QSize size, FILE * file, MainWindow * parent);

bool exportCos(FILE * file, QPoint position, QSize size, QString background, MapEditor *window);

#endif // CREATURESIO_H
