#include "mapeditor.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QCoreApplication::addLibraryPath(".");
	QApplication a(argc, argv);
	MapEditor w;
	w.show();

	return a.exec();
}
