#include "metaroomoffset.h"
#include "ui_metaroomoffset.h"
#include <QPushButton>
#include <QDesktopWidget>

MetaroomOffset::MetaroomOffset(QWidget *parent, QString &background, QPoint &position, QSize &size) :
QDialog(parent),
background(background),
position(position),
size(size),
xValidator(0, 100000 - size.width (), this),
yValidator(0, 100000 - size.height(), this),
ui(new Ui::MetaroomOffset)
{
	ui->setupUi(this);

	ui->posx->setValidator( &xValidator );
	ui->posy->setValidator( &yValidator );

	ui->width->setText(QString::number(size.width()));
	ui->height->setText(QString::number(size.height()));

	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	connect(ui->posx, &QLineEdit::editingFinished, this, &MetaroomOffset::validateForm);
	connect(ui->posy, &QLineEdit::editingFinished, this, &MetaroomOffset::validateForm);
	connect(ui->background, &QLineEdit::editingFinished, this, &MetaroomOffset::validateForm);

	setGeometry(
	    QStyle::alignedRect(
	        Qt::LeftToRight,
	        Qt::AlignCenter,
	        this->QDialog::size(),
	        qApp->desktop()->availableGeometry()
	    )
	);
}

MetaroomOffset::~MetaroomOffset()
{
	delete ui;
}

void MetaroomOffset::validateForm()
{
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
			ui->posx->hasAcceptableInput()
		&&  ui->posy->hasAcceptableInput()
		&&  ui->background->hasAcceptableInput()
	);
}
