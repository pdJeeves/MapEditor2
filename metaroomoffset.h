#ifndef METAROOMOFFSET_H
#define METAROOMOFFSET_H
#include <QIntValidator>
#include <QDialog>

namespace Ui {
class MetaroomOffset;
}

class MetaroomOffset : public QDialog
{
	Q_OBJECT
	QString & background;
	QPoint & position;
	QSize & size;

public:
	explicit MetaroomOffset(QWidget *parent, QString & background, QPoint & position, QSize & size);
	~MetaroomOffset();

	void validateForm();

private:
	QIntValidator xValidator;
	QIntValidator yValidator;
	Ui::MetaroomOffset *ui;
};

#endif // METAROOMOFFSET_H
