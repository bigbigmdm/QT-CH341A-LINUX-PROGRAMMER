#include "programmer.h"
#include "ui_programmer.h"
extern "C"
{
#include <libusb-1.0/libusb.h>
}
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <getopt.h>
#include <limits.h>
#include <sys/mman.h>
#include <assert.h>
#include <QLabel>

FILE *debugout, *verbout;
uint8_t *readbuf1 = NULL;

void Programmer::barNotshowing()
{
    ui->progressBar->setVisible(false);

}

void Programmer::barShowing()
{
    ui->progressBar->setVisible(true);

}

void Programmer::setValue(int val_bar)
{

    ui->progressBar->setValue(val_bar);
}


void Programmer::setTxt(QString prog_txt)
{
    ui->textEdit->append(prog_txt);
    ui->textEdit->ensureCursorVisible();
}

void Programmer::setImg(bool img)
{
    QPixmap pix_on, pix_off;
    pix_on = QPixmap(":/images/ch341_to_form_150_150.png");
    pix_off = QPixmap(":/images/not_found.png");
    if (img)
    {
      ui-> label ->setPixmap(pix_on);
      ui->progressBar->setVisible(true);
    }
    else
    {
      ui-> label ->setPixmap(pix_off);
      ui->progressBar->setVisible(false);
    }
}



Programmer::Programmer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Programmer)
{
    QPixmap pix_on, pix_off;
    pix_on = QPixmap(":/images/ch341_to_form_150_150.png");
    pix_off = QPixmap(":/images/not_found.png");

  ui->setupUi(this);

  ui->textEdit->append("Found..");
  ui->progressBar->setMaximum(100);
  ui->progressBar->setValue(0);
  ui->label->setPixmap(pix_on);

  //QMessageBox::about(this, "Title", QString::number(ii));
// ui->progressBar->setValue(value;);
}

Programmer::~Programmer()
{
    delete ui;
}

void Programmer::on_buttonBox_accepted()
{
//ui->label->setPixmap(pix_off);
}



