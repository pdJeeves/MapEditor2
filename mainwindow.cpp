#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMimeData>
#include <QCursor>
#include "creaturesio.h"
#include <QPainter>
#include "byte_swap.h"
#include "metaroomoffset.h"

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
escape(Qt::Key_Escape, this),
ui(new Ui::MainWindow)
{
	zoom = 1.0;

	ui->setupUi(this);

	ui->widget->window = this;
	ui->widget->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->widget->connect(ui->widget, &QWidget::customContextMenuRequested, this, &MainWindow::ShowContextMenu);

	connect(ui->actionNew, &QAction::triggered, this, &MainWindow::documentNew);
	connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::documentOpen);
	connect(ui->actionSave, &QAction::triggered, this, &MainWindow::documentSave);
	connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::documentSaveAs);

	connect(ui->actionShow_Background, &QAction::triggered, this, [this]() { ui->widget->repaint(); } );
	connect(ui->actionShow_Foreground, &QAction::triggered, this, [this]() { ui->widget->repaint(); } );
	connect(ui->actionShow_Cutouts   , &QAction::triggered, this, [this]() { ui->widget->repaint(); } );

	connect(ui->actionAddRoom, &QAction::triggered  , this, [this]() { onKeyPress(AddRoom); } );
	connect(ui->actionSelect , &QAction::triggered  , this, [this]() { onKeyPress(Select ); } );
	connect(ui->actionGrab	 , &QAction::triggered  , this, [this]() { onKeyPress(Grab   ); } );
	connect(ui->actionExtrude, &QAction::triggered  , this, [this]() { onKeyPress(Extrude); } );
 	connect(ui->actionSlice  , &QAction::triggered  , this, [this]() { onKeyPress(SelectSlice  ); } );
	connect(&escape			 , &QShortcut::activated, this, [this]() { onKeyPress(Cancel ); } );

	connect(ui->horizontalScrollBar, &QScrollBar::valueChanged, [this](int) { ui->widget->repaint(); });
	connect(ui->verticalScrollBar, &QScrollBar::valueChanged, [this](int) { ui->widget->repaint(); });

	setGeometry(
	    QStyle::alignedRect(
	        Qt::LeftToRight,
	        Qt::AlignCenter,
	        size(),
	        qApp->desktop()->availableGeometry()
	    )
	);

}

MainWindow::~MainWindow()
{
	delete ui;
}


static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

bool MainWindow::dimensionCheck(QSize size)
{
	if(background.canSetImage(size))
		return true;

	QMessageBox::information(this,  QGuiApplication::applicationDisplayName(),
		tr("The given background would not match the current dimensions. %1x%2 vs %3x%4")
		.arg(size.width()).arg(size.height()).arg(background.dimensions().width()).arg(background.dimensions().height()));
	return false;
}




bool MainWindow::documentClose()
{
	return true;
}

bool MainWindow::documentNew()
{
	if(documentClose())
	{
		background.clear();
	}
	return true;
}

void MainWindow::documentOpen()
{
	if(!documentClose())
		return;

	Background temp = std::move(background);
	background.clear();

	QFileDialog dialog(this, tr("Open File"));
	initializeBackgroundFileDialog(dialog, QFileDialog::AcceptOpen);

	bool accepted;
	while ((accepted = (dialog.exec() == QDialog::Accepted)) && !loadBackground(dialog.selectedFiles().first())) {}

	if(!accepted)
	{
		background = std::move(temp);
	}
	else
	{
		filename.clear();
	}

	ui->widget->repaint();
}

bool MainWindow::loadBackground(const QString & name)
{
	FILE * file = fopen(name.toStdString().c_str(), "rb");
	if(!file)
	{
		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
			tr("Unable to open file '%1' for reading.").arg(name));
		return false;
	}

	bool r = loadBackground(file, name);

	fclose(file);
	return r;
}

bool MainWindow::loadBackground(FILE * file, const QString & name)
{
	int dot = name.lastIndexOf('.');

	if(0 < name.indexOf("spr", dot, Qt::CaseInsensitive))
		return background.setImage(0, 0, importSpr(file, this));
	else if(0 < name.indexOf("s16", dot, Qt::CaseInsensitive))
		return background.setImage(0, 0, importS16(file, this));
	else if(0 < name.indexOf("blk", dot, Qt::CaseInsensitive))
		return background.setImage(0, 0, importBlk(file, this));

	int _one;
	fread(&_one, 4, 1, file);

	if(byte_swap(_one) != 1)
	{
		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
			tr("%1 is not a kreatures file.").arg(name));
		return false;
	}

	uint16_t width, height;
	std::array<uint32_t, 32> offsets;
	offsets.fill(0);

	fread(&width, 2, 1, file);
	fread(&height, 2, 1, file);

	width = byte_swap(width);
	height = byte_swap(height);

	if(0 < name.indexOf("bak", dot, Qt::CaseInsensitive))
	{
		fread(&offsets[0], sizeof(uint32_t), offsets.size(), file);

		background.readLayer(file, width, height, 0, &offsets[0], this);
		background.readLayer(file, width, height, 1, &offsets[5], this);
		background.readLayer(file, width, height, 2, &offsets[10], this);
		return true;
	}
	else if(0 < name.indexOf("plx", dot, Qt::CaseInsensitive))
	{
		fread(&offsets[0], sizeof(uint32_t), 5, file);
		background.readLayer(file, width, height, 0, &offsets[0], this);
		return true;
	}

	return false;
}

void MainWindow::initializeBackgroundFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
	QStringList filters;
	filters << "Kreatures Background (*.bak)"
			<< "Kreatures Layer (*.plx)";

	if (acceptMode == QFileDialog::AcceptOpen)
		filters	<< "C1 Background (*.spr)"
				<< "C2 Background (*.s16)"
				<< "c2e Background (*.blk)";
	else if(acceptMode == QFileDialog::AcceptSave && background.onlyBackground())
	{
		if(background.dimensions() == QSize(58*144, 8*150))
			filters << "C1 Background (*.spr)";
		if(background.dimensions() == QSize(58*144, 16*150))
			filters << "C2 Background (*.s16)";

		filters << "c2e Background (*.blk)";
	}

	if (acceptMode == QFileDialog::AcceptSave)
		filters	<< "Kreatures Room Map (*.rmp)"
				<< "c2e Room Map (*.cos)";

	dialog.setNameFilters(filters);

    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("bak");
}

bool MainWindow::loadImage(int map, int channel, const QString & name)
{
	QImageReader reader(name);
	if(!dimensionCheck(reader.size()))
		return false;

	reader.setAutoTransform(true);
	QImage newImage = reader.read();

	if (newImage.isNull())
	{
		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
		tr("Cannot load %1: %2")
		.arg(QDir::toNativeSeparators(name), reader.errorString()));
		return false;
	}

	background.setImage(map, channel, std::move(newImage));
	return true;
}


void MainWindow::openImage(int map, int channel)
{
	QFileDialog dialog(this, tr("Load Image"));
	initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

	while (dialog.exec() == QDialog::Accepted && !loadImage(map, channel, dialog.selectedFiles().first())) {}
}

void MainWindow::documentSave()
{
	if(!filename.isNull())
	{
		saveBackground(filename);
	}
	else
	{
		documentSaveAs();
	}
}

void MainWindow::documentSaveAs()
{
	QFileDialog dialog(this, tr("Save File"));
	initializeBackgroundFileDialog(dialog, QFileDialog::AcceptSave);

	bool accepted;
	while ((accepted = (dialog.exec() == QDialog::Accepted)) && !saveBackground(dialog.selectedFiles().first())) {}

	if(accepted)
	{
		filename = dialog.selectedFiles().first();
	}
}

bool MainWindow::saveBackground(const QString & name)
{
	int dot = name.lastIndexOf('.');

	if(0 < name.indexOf("cos", dot, Qt::CaseInsensitive))
	{
		int slash = name.lastIndexOf('/')+1;
		QPoint position;
		QSize size		   = background.dimensions();
		QString background = name.mid(slash,(dot-1) - slash);

		MetaroomOffset dialog(this, background, position, size);
		dialog.show();
		if(dialog.exec() == QDialog::Rejected)
		{
			return false;
		}

		FILE * file = fopen(name.toStdString().c_str(), "w");
		if(!file)
		{
			QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
				tr("Unable to open file '%1' for writing.").arg(name));
			return false;
		}

		bool r = exportCos(file, position, size, background, this);
		fclose(file);
		return r;
	}

	FILE * file = fopen(name.toStdString().c_str(), "wb");
	if(!file)
	{
		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
			tr("Unable to open file '%1' for writing.").arg(name));
		return false;
	}

	bool r = saveBackground(file, name);

	fclose(file);
	return r;
}

bool MainWindow::saveBackground(FILE * file, const QString & name)
{
	int dot = name.lastIndexOf('.');

	if(0 < name.indexOf("spr", dot, Qt::CaseInsensitive))
		return exportSpr(background.content[0][0].image, file, this);
	else if(0 < name.indexOf("s16", dot, Qt::CaseInsensitive))
		return exportS16(background.content[0][0].image, file, this);
	else if(0 < name.indexOf("blk", dot, Qt::CaseInsensitive))
		return exportBlk(background.content[0][0].image, background.dimensions(), file, this);

	int _one = byte_swap((uint32_t) 1);
	fwrite(&_one, 4, 1, file);

	uint16_t width = byte_swap((uint16_t) background.dimensions().width());
	uint16_t height = byte_swap((uint16_t) background.dimensions().height());
	fwrite(&width, 2, 1, file);
	fwrite(&height, 2, 1, file);

	std::array<uint32_t, 32> offsets;
	offsets.fill(0);

	uint32_t offset_pos = ftell(file);
	fseek(file, sizeof(uint32_t)*offsets.size(), SEEK_CUR);

	if(0 < name.indexOf("bak", dot, Qt::CaseInsensitive))
	{
		background.writeLayer(file, 0, &offsets[0], this);
		background.writeLayer(file, 1, &offsets[5], this);
		background.writeLayer(file, 2, &offsets[10], this);
	}
	else if(0 < name.indexOf("plx", dot, Qt::CaseInsensitive))
	{
		background.writeLayer(file, 0, &offsets[0], this);
	}
	else
	{
		return false;
	}

	fseek(file, offset_pos, SEEK_SET);
	fwrite(&offsets[0], sizeof(uint32_t), offsets.size(), file);

	return true;
}



QSize MainWindow::getScreenSize() const
{
	return ui->widget->size() / zoom;
}


QPoint MainWindow::getScreenOffset()
{
	QSize s0 = background.dimensions();

	if(s0 == QSize())
		return QPoint();

	s0 = s0 -  getScreenSize();

	QPoint offset(ui->horizontalScrollBar->value() * s0.width() / 255,
				  ui->verticalScrollBar->value() * s0.height() / 255);

	return offset;
}


QPoint MainWindow::getMousePosition()
{
	QSize dimensions = background.dimensions();

	if(dimensions == QSize())
		return QPoint();

	QPoint pos = ui->widget->mapFromGlobal(QCursor::pos()) / zoom;
	QPoint offset = getScreenOffset();

	pos += offset;

	pos.setX(std::max(0, std::min(pos.x(), dimensions.width())));
	pos.setY(std::max(0, std::min(pos.y(), dimensions.height())));

	return pos;
}

void MainWindow::draw(QPainter & painter)
{
	QPoint offset = getScreenOffset();
	QSize  size  = getScreenSize();

	painter.scale(zoom, zoom);

	for(int i = 0; i < NO_MAPS; ++i)
	{
		if(!showBackground(i))
			continue;

		if(background.content[i][showMapping()].image.isNull())
			continue;

		background.draw(painter, i, showMapping(), offset, size);
	}
}

bool MainWindow::showBackground(int i) const
{
	switch(i)
	{
	case 0: return ui->actionShow_Background->isChecked();
	case 1: return ui->actionShow_Foreground->isChecked();
	case 2: return ui->actionShow_Cutouts->isChecked();
	}
	return false;
}

int MainWindow::showMapping() const
{
	return 0;
}



