#ifndef PROPERTIES_H
#define PROPERTIES_H
#include <QIntValidator>
#include <QDialog>

namespace Ui {
class Properties;
}

class MapEditor;
class Selection;
class QTableWidget;

class Properties : public QDialog
{
	Q_OBJECT
	MapEditor * window;
	QTableWidget * table;

public:
	explicit Properties(MapEditor * window, QWidget *parent = 0);
	~Properties();

	void onSelectionChanged();
	void closeEvent(QCloseEvent *);

public slots:
	void setRoomType(int);


private:
	void setPermeability();


	QIntValidator permValidator;
	Ui::Properties *ui;
};

#endif // PROPERTIES_H
