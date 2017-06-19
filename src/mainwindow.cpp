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
#include "mapeditor.h"

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
zoom(1.0),
titleDirty(false),
name(tr("Untitled")),
escape(Qt::Key_Escape, this),
xKey(Qt::Key_X, this),
yKey(Qt::Key_Y, this),
autosaveTimer(this),
commandList(static_cast<MapEditor*>(this)),
propertiesWindow(static_cast<MapEditor*>(this), this),
m_channel(this),
ui(new Ui::MainWindow)
{
	autosaveTimer.setInterval(5*60*1000);
	autosaveTimer.setTimerType(Qt::VeryCoarseTimer);
	autosaveTimer.setSingleShot(true);

	connect(&autosaveTimer, &QTimer::timeout, this, &MainWindow::documentSave);

	ui->setupUi(this);

	ui->widget->window = this;
	ui->widget->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->widget->connect(ui->widget, &QWidget::customContextMenuRequested, this, &MainWindow::ShowContextMenu);

	connect(ui->actionNew, &QAction::triggered, []()
	{
		MapEditor * w = new MapEditor();
		w->show();
	});

	connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::documentOpen);
	connect(ui->actionSave, &QAction::triggered, this, &MainWindow::documentSave);
	connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::documentSaveAs);
	connect(ui->actionClose, &QAction::triggered, this, &MainWindow::documentNew);

	connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::editUndo);
	connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::editRedo);

	connect(ui->actionCut, &QAction::triggered, this, [this]() { editCopy(); editDelete(); } );
	connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::editCopy);
	connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::editPaste);
	connect(ui->actionDelete, &QAction::triggered, this, &MainWindow::editDelete);

	connect(ui->actionZoom_In, &QAction::triggered, this, [this]() { changeZoom(1/.8); });
	connect(ui->actionZoom_Out, &QAction::triggered, this, [this]() { changeZoom(.8); });
	connect(ui->actionActual_Size, &QAction::triggered, this, [this]()
	{
		zoom = 1.0;
		ui->actionZoom_In->setEnabled(true);
		ui->actionZoom_Out->setEnabled(true);
		ui->widget->needRepaint();
	});

	connect(ui->actionSelect_All, &QAction::triggered, this, &MainWindow::editSelectAll);

	QMenu * menu;

	for(auto i = 0; i < NO_MAPS; ++i)
	{
		menu = ui->menuLoad->addMenu(getMapName(i));

		for(auto j = 0; j < NO_LAYERS; ++j)
		{
			menu->addAction(getChannelName(j), this, [&]() { openImage(i, j); });
		}
	}

	ui->menuView->addSeparator();

	for(auto i = 0; i < NO_MAPS; ++i)
	{
		QAction * action = ui->menuView->addAction(getMapName(i), ui->widget, &ViewWidget::needRepaint );
		action->setCheckable(true);
		action->setChecked(true);
		m_layer.push_back(action);
	}

	ui->menuView->addSeparator();

	for(auto i = 0; i < NO_MAPS; ++i)
	{
		QAction * action = ui->menuView->addAction(getMapName(i), ui->widget, &ViewWidget::needRepaint );
		action->setCheckable(true);
		action->setChecked(i == 0);
		m_channel.addAction(action);
	}

	connect(ui->actionAddRoom, &QAction::triggered  , this, [this]() { onKeyPress(AddRoom); } );
	connect(ui->actionGrab	 , &QAction::triggered  , this, [this]() { onKeyPress(Grab   ); } );
	connect(ui->actionExtrude, &QAction::triggered  , this, [this]() { onKeyPress(Extrude); } );
 	connect(ui->actionSlice  , &QAction::triggered  , this, [this]() { onKeyPress(SelectSlice  ); } );
	connect(&escape			 , &QShortcut::activated, this, [this]() { onKeyPress(Cancel ); } );
	connect(&xKey			 , &QShortcut::activated, this, [this]() { onKeyPress(LockX ); } );
	connect(&yKey			 , &QShortcut::activated, this, [this]() { onKeyPress(LockY ); } );

	connect(ui->horizontalScrollBar, &QScrollBar::valueChanged, ui->widget, &ViewWidget::needRepaint);
	connect(ui->verticalScrollBar, &QScrollBar::valueChanged, ui->widget, &ViewWidget::needRepaint);

	connect(ui->actionAbout, &QAction::triggered, &QApplication::aboutQt);

	connect(ui->actionProperties, &QAction::triggered, this, [this](bool checked)
	{
		if(checked == true)
			propertiesWindow.show();
		else
			propertiesWindow.hide();
	});

	ui->actionUndo->setEnabled(false);
	ui->actionRedo->setEnabled(false);

	setGeometry(
	    QStyle::alignedRect(
	        Qt::LeftToRight,
	        Qt::AlignCenter,
	        size(),
	        qApp->desktop()->availableGeometry()
	    )
	);

}

void MainWindow::onCommandPushed()
{
	if(commandList.isDirty())
	{
		if(!titleDirty)
		{
			titleDirty = true;
			setWindowTitle(tr("%1 * - Map Editor").arg(name));
		}

		if(!autosaveTimer.isActive() && !filename.isNull())
		{
			autosaveTimer.start();
		}
	}
	else
	{
		if(titleDirty)
		{
			titleDirty = false;
			setWindowTitle(tr("%1 - Map Editor").arg(name));
		}

		if(autosaveTimer.isActive())
		{
			autosaveTimer.stop();
		}
	}


	ui->actionUndo->setEnabled(commandList.canRollBack());
	ui->actionRedo->setEnabled(commandList.canRollForward());
	ui->widget->needRepaint();
}

MainWindow::~MainWindow()
{
	delete ui;
}

QString MainWindow::getMapName(int i) const
{
	switch(i)
	{
	case 0: return tr("Background");
	case 1: return tr("Foreground");
	default: return tr("Cutaways");
	}
}

QString MainWindow::getChannelName(int i) const
{
	switch(i)
	{
	case 0: return tr("Baked");
	case 1: return tr("Albedo");
	case 2: return tr("Normal");
	case 3: return tr("Microsurface");
	case 4: return tr("Metalicity");
	default: return tr("Luminosity");

	}
}

bool MainWindow::c2eWalls() const
{
	return ui->actionLock_Walls->isChecked();
}

void MainWindow::editUndo()
{
	commandList.rollBack();
	ui->actionUndo->setEnabled(commandList.canRollBack());
	ui->actionRedo->setEnabled(commandList.canRollForward());
}

void MainWindow::editRedo()
{
	commandList.rollForward();
	ui->actionUndo->setEnabled(commandList.canRollBack());
	ui->actionRedo->setEnabled(commandList.canRollForward());
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

void MainWindow::changeZoom(float factor)
{
	zoom *= factor;
	if(zoom <= .25)
	{
		zoom = .25;
		ui->actionZoom_In->setEnabled(true);
		ui->actionZoom_Out->setEnabled(false);
	}
	else if(zoom >= 4)
	{
		zoom = 4;
		ui->actionZoom_In->setEnabled(false);
		ui->actionZoom_Out->setEnabled(true);
	}

	ui->widget->repaint();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if(documentClose())
		QMainWindow::closeEvent(event);
}

bool MainWindow::documentClose()
{
	if(commandList.isDirty())
	{
		QMessageBox::StandardButton button = QMessageBox::question(
			this,
			QGuiApplication::applicationDisplayName(),
			tr("Save changes to %1?").arg(name),
			QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
			QMessageBox::Cancel);

		switch(button)
		{
		case QMessageBox::No:
			return true;
		case QMessageBox::Yes:
			return documentSave();
		default:
			return false;
		}
	}

	return true;
}

bool MainWindow::documentNew()
{
	if(documentClose())
	{
		zoom = 1.0;

		background.clear();
		filename.clear();
		name = tr("Untitled");
		commandList.clear();

		clearRooms();

		return true;
	}

	return false;
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
		return;
	}

	QString path = dialog.selectedFiles().first();

	filename.clear();
	clearRooms();
	autosaveTimer.stop();

	name = path.right(path.size() - (path.lastIndexOf('/')+1));

	if(0 < path.indexOf("bak", path.lastIndexOf('.'), Qt::CaseInsensitive))
		filename = path;

	titleDirty = true;
	commandList.clear();
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
	std::array<uint32_t, 64> offsets;
	offsets.fill(0);

	fread(&width, 2, 1, file);
	fread(&height, 2, 1, file);

	width = byte_swap(width);
	height = byte_swap(height);

	if(0 < name.indexOf("bak", dot, Qt::CaseInsensitive))
	{
		fread(&offsets[0], sizeof(uint32_t), offsets.size(), file);

		background.readLayer(file, width, height, 0, &offsets[0], this);
		background.readLayer(file, width, height, 1, &offsets[8], this);
		background.readLayer(file, width, height, 2, &offsets[16], this);
		readRooms(file, &offsets[32]);
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
		filters	<< "c2e Room Map (*.cos)";

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

bool MainWindow::documentSave()
{
	if(!filename.isNull())
	{
		bool r = saveBackground(filename);
		commandList.onSave();
		return r;
	}
	else
	{
		return documentSaveAs();
	}
}

bool MainWindow::documentSaveAs()
{
	QFileDialog dialog(this, tr("Save File"));
	initializeBackgroundFileDialog(dialog, QFileDialog::AcceptSave);

	bool accepted;
	while ((accepted = (dialog.exec() == QDialog::Accepted)) && !saveBackground(dialog.selectedFiles().first())) {}

	if(!accepted)
		return false;

	QString path = dialog.selectedFiles().first();

	name = path.right(path.size() - (path.lastIndexOf('/')+1));

	if(0 < path.indexOf("bak", path.lastIndexOf('.'), Qt::CaseInsensitive))
		filename = path;

	titleDirty = true;
	commandList.onSave();

	return true;
}

bool MainWindow::saveBackground(const QString & name)
{
	int dot = name.lastIndexOf('.');

	if(0 < name.indexOf("cos", dot, Qt::CaseInsensitive))
	{
		int slash = name.lastIndexOf('/')+1;
		QPoint position;
		QSize size		   = background.dimensions();
		QString background = name.mid(slash,(dot) - slash);

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

		bool r = exportCos(file, position, size, background, static_cast<MapEditor*>(this));
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

	std::array<uint32_t, 64> offsets;
	offsets.fill(0);

	uint32_t offset_pos = ftell(file);
	fseek(file, sizeof(uint32_t)*offsets.size(), SEEK_CUR);

	if(0 < name.indexOf("bak", dot, Qt::CaseInsensitive))
	{
		background.writeLayer(file, 0, &offsets[0], this);
		background.writeLayer(file, 1, &offsets[8], this);
		background.writeLayer(file, 2, &offsets[16], this);
		writeRooms(file, &offsets[32]);
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
	return m_layer[i % m_layer.size()]->isChecked();
}

int MainWindow::showMapping() const
{
	QList<QAction*> list = m_channel.actions();
	int i = 0;

	for(auto j = list.begin(); j != list.end(); ++i, ++j)
	{
		if((*j)->isChecked())
			return i;
	}

	return 0;
}



