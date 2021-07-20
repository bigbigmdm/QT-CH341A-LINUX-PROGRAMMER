#ifndef EEPROMSIZE_H
#define EEPROMSIZE_H
#include <QDialog>
#include <QtCore>
#include "../src/qhexedit.h"

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
    void on_listWidget_clicked();
signals:    
    void on_listWidget_click(QString chiptext);

private slots:

    void on_buttonBox_accepted();


private:

};

#endif // EEPROMSIZE_H
