#ifndef CREATURESIO_H
#define CREATURESIO_H
#include <QImage>

typedef struct _IO_FILE FILE;
class QWidget;

QImage importS16(FILE * file, QWidget * parent);
QImage importBlk(FILE * file, QWidget * parent);
QImage importSpr(FILE * file, QWidget * parent);

bool exportSpr(const QImage & image, FILE * file, QWidget * parent);
bool exportS16(const QImage & image, FILE * file, QWidget * parent);
bool exportBlk(const QImage & image, QSize size, FILE * file, QWidget * parent);

bool exportCos(FILE * file, QPoint position, QSize size, QString background, QWidget *);

#endif // CREATURESIO_H
