#include "properties.h"
#include "ui_properties.h"
#include "mapeditor.h"
#include "ui_mainwindow.h"
#include <QLineEdit>
#include <QComboBox>
#include <QDesktopWidget>
#include "permeabilitycommand.h"
#include "roomtypecommand.h"

Properties::Properties(MapEditor * window, QWidget *parent) :
QDialog(parent),
window(window),
permValidator(0, 100),
ui(new Ui::Properties)
{
	ui->setupUi(this);
	table = ui->tableWidget;

	setGeometry(
	    QStyle::alignedRect(
	        Qt::LeftToRight,
	        Qt::AlignCenter,
	        size(),
	        qApp->desktop()->availableGeometry()
	    )
	);

	table->setColumnCount(2);
}

Properties::~Properties()
{
	delete ui;
}

void Properties::closeEvent(QCloseEvent *)
{
	window->ui->actionProperties->setChecked(false);
}

void Properties::onSelectionChanged()
{
	table->clear();

	if(window->selection.isOnlyFaces())
	{
		int type = -1;
		int no_faces = 0;

		for(auto i = window->selection.faces.begin(); i != window->selection.faces.end(); ++i)
		{
			if(!(*i)->isSelected())
				continue;

			++no_faces;

			if(type == -1)
				type = (*i)->room_type;
			else if(type != (*i)->room_type)
			{
				type = -2;
				break;
			}
		}

		if(no_faces)
		{
			table->setRowCount(1);
			table->setItem(0, 0, new QTableWidgetItem(tr("Room Type")));

			QComboBox * comboBox = new QComboBox(0L);

			QStringList types;
			types
				<< tr(" 0 - Atmosphere")
				<< tr(" 1 - Wooden Walkway")
				<< tr(" 2 - Concrete Walkway")
				<< tr(" 3 - Indoor Concrete")
				<< tr(" 4 - Outdoor Concrete")
				<< tr(" 5 - Normal Soil")
				<< tr(" 6 - Boggy Soil")
				<< tr(" 7 - Drained Soil")
				<< tr(" 8 - Fresh Water")
				<< tr(" 9 - Salt Water")
				<< tr("10 - Ettin Home");

			comboBox->insertItems(0, types);
			if(type >= 0) comboBox->setCurrentIndex(type);

			table->setCellWidget(0, 1, comboBox);
			table->connect(comboBox, SIGNAL(activated(int)), this, SLOT(setRoomType(int)));
		}
	}
	else
	{
		int perm = -1;
		int no_edges = 0;

		for(auto i = window->selection.faces.begin(); i != window->selection.faces.end(); ++i)
		{
			for(int j = 0; j < 4; ++j)
			{
				if(!(*i)->isEdgeSelected((Direction) j))
					continue;

				++no_edges;

				int temp = (*i)->getPermeability((Direction) j);

				if(temp == -1)
					continue;

				if(perm == -1)
					perm = temp;
				else if(perm != temp)
				{
					perm = -2;
					break;
				}
			}
		}

		if(no_edges)
		{
			table->setRowCount(1);
			table->setItem(0, 0, new QTableWidgetItem(tr("permeability")));

			QLineEdit * line = new QLineEdit(perm >= 0? QString::number(perm) : QString(" - "), 0L);
			line->setValidator(&permValidator);
			table->setCellWidget(0, 1, line);
			connect(line, &QLineEdit::editingFinished, this, &Properties::setPermeability);
		}
	}

	table->resizeColumnsToContents();
	table->resizeRowsToContents();
	table->repaint();
}

void Properties::setPermeability()
{
	int perm = static_cast<QLineEdit*>(table->cellWidget(0, 1))->text().toInt();

	if(0 <= perm && perm <= 100)
	{
		window->commandList.push(PermeabilityCommand::Create(window->selection, perm));
	}
}

void Properties::setRoomType(int k)
{
	window->commandList.push(RoomTypeCommand::Create(window->selection, k));
}


