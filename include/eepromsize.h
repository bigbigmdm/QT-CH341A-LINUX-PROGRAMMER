#ifndef EEPROMSIZE_H
#define EEPROMSIZE_H
#include <QMainWindow>
#include <QDialog>
#include <QtCore>
#include <QMouseEvent>
#include "qhexedit.h"

namespace Ui {
    class EepromSize;
}

class EepromSize : public QDialog
{
    Q_OBJECT
public:
    explicit EepromSize(QWidget *parent = 0);
    ~EepromSize();
    Ui::EepromSize *ui;
    void show();

public slots:
    virtual void accept();
    
signals:    
    void listWidget_click(QString chiptext);

private slots:
    void on_buttonBox_accepted();
    void listWidget_clicked();

private:

};

#endif // EEPROMSIZE_H
