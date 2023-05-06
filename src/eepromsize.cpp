#include "eepromsize.h"
#include "ui_eepromsize.h"
#include <QMessageBox>
EepromSize::EepromSize(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EepromSize)
{
    ui->setupUi(this);
}

EepromSize::~EepromSize()
{
    delete ui;
}

void EepromSize::on_buttonBox_accepted()
{
// QListWidget.currentItem();
//    int itemno =   QListWidget::SelectItems;
//    QString str = QString::number(itemno);
//    QMessageBox::about(this, "Title", str);
//     QString str = QListWidget::currentItem()->text();
 //           QListWidget::currentItem(ListWidget)->text();
 //    QMessageBox::about(this, "Title", str );
}
void EepromSize::show()
{
    QWidget::show();

}
void EepromSize::accept()
{
    emit accepted();
//    QString Size = EepromSize::on_listWidget_clicked;
//    QMessageBox::about(this, "EEPROM type:", "dddd" );
    QDialog::hide();
}


void EepromSize::on_listWidget_clicked()
{

    QString chipmodel = ui->listWidget->currentItem()->text();
//    QMessageBox::about(this, "EEPROM type:", chipmodel );

    emit on_listWidget_click(chipmodel);

}

