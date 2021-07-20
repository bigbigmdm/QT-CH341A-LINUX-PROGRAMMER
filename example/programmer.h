#ifndef PROGRAMMER_H
#define PROGRAMMER_H
#include <stdint.h>
#include <QMainWindow>
#include <QMessageBox>


int32_t ch341readEEPROM(struct libusb_device_handle *devHandle, uint8_t *buf, uint32_t bytes, struct EEPROM* eeprom_info);
int32_t ch341writeEEPROM(struct libusb_device_handle *devHandle, uint8_t *buf, uint32_t bytes, struct EEPROM* eeprom_info);
struct libusb_device_handle *ch341configure(uint16_t vid, uint16_t pid);
int32_t ch341setstream(struct libusb_device_handle *devHandle, uint32_t speed);
int32_t parseEEPsize(char* eepromname, struct EEPROM *eeprom);

// callback functions for async USB transfers
void cbBulkIn(struct libusb_transfer *transfer);
void cbBulkOut(struct libusb_transfer *transfer);


#include <QDialog>

namespace Ui {
class Programmer;
}

class Programmer : public QDialog
{
    Q_OBJECT

public:
    explicit Programmer(QWidget *parent = 0);
    ~Programmer();
public slots:
    void setValue(int value);
    void setTxt(QString prog_txt);
    void setImg(bool img);
    void barNotshowing();
    void barShowing();
private slots:
    void on_buttonBox_accepted();


private:
    Ui::Programmer *ui;
};

#endif // PROGRAMMER_H
