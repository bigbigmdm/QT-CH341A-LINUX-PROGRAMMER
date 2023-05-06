#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "qhexedit.h"
#include "optionsdialog.h"
#include "searchdialog.h"
#include "eepromsize.h"
#include "programmer.h"
#include "counter.h"

#include <assert.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
using namespace std;
extern "C"
{
  #include <libusb-1.0/libusb.h>
}
//INIT
#define USB_LOCK_VENDOR             0x1a86 // Dev : (1a86) QinHeng Electronics
#define USB_LOCK_PRODUCT            0x5512 //       (5512) CH341A in i2c mode
#define DEFAULT_INTERFACE           0x00
#define DEFAULT_CONFIGURATION       0x01
#define MAX_EEPROM_SIZE             131072 /* For 24c1024*/
//SET STREAM
#define	mCH341A_CMD_I2C_STM_SET		0x60
//READ
#define EEPROM_READ_BULKIN_BUF_SZ   0x20
#define EEPROM_READ_BULKOUT_BUF_SZ  0x65
#define IN_BUF_SZ                   0x100
#define BULK_READ_ENDPOINT          0x82   /* bEndpointAddress 0x82  EP 2 IN  (Bulk)*/
#define DEFAULT_TIMEOUT             300    // 300mS for USB timeouts
//MARSHAL
#define EEPROM_I2C_BUS_ADDRESS      0x50
#define	mCH341A_CMD_I2C_STREAM		0xAA
#define	mCH341A_CMD_I2C_STM_STA		0x74
#define	mCH341A_CMD_I2C_STM_OUT		0x80
#define	mCH341A_CMD_I2C_STM_IN		0xC0
#define	mCH341A_CMD_I2C_STM_STO		0x75
#define	mCH341A_CMD_I2C_STM_END		0x00
#define CH341_EEPROM_READ_CMD_SZ    0x65
#define BULK_WRITE_ENDPOINT         0x02   /* bEndpointAddress 0x02  EP 2 OUT (Bulk)*/
struct EEPROM {
    char const *name;
    uint32_t size;
    uint16_t page_size;
    uint8_t addr_size; // Length of addres in bytes
    uint8_t i2c_addr_mask;
};

const static struct EEPROM eepromlist[] = {
  { "24c01",   128,     8,  1, 0x00}, // 16 pages of 8 bytes each = 128 bytes
  { "24c02",   256,     8,  1, 0x00}, // 32 pages of 8 bytes each = 256 bytes
  { "24c04",   512,    16,  1, 0x01}, // 32 pages of 16 bytes each = 512 bytes
  { "24c08",   1024,   16,  1, 0x03}, // 64 pages of 16 bytes each = 1024 bytes
  { "24c16",   2048,   16,  1, 0x07}, // 128 pages of 16 bytes each = 2048 bytes
  { "24c32",   4096,   32,  2, 0x00}, // 32kbit = 4kbyte
  { "24c64",   8192,   32,  2, 0x00},
  { "24c128",  16384,  32/*64*/,  2, 0x00},
  { "24c256",  32768,  32/*64*/,  2, 0x00},
  { "24c512",  65536,  32/*128*/, 2, 0x00},
  { "24c1024", 131072, 32/*128*/, 2, 0x01},
  { "", 0, 0, 0, 0 }
};

struct answer {
    QString ans_txt;
    int ans_byte;
};

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QUndoStack;
class QLabel;
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();


protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void about();
    void dataChanged();
    void open();
    void optionsAccepted();
    void findNext();
    bool save();
    bool saveAs();
    void saveSelectionToReadableFile();
    void saveToReadableFile();
    void setAddress(qint64 address);
    void setOverwriteMode(bool mode);
    void setSize(qint64 size);
    void showOptionsDialog();
    void showSearchDialog();
//
    void showEepromSize();
    void showProgrammer();
    int readActBt();
    int writeActBt();
    void selectActBt();
    void programmerActBt();
    void setChipType(QString ch_type);
    void setValue(int);


signals:
    void workingPercentChanged(qint32 percent);
//
private:
    void init();
    void createActions();
    void createMenus();
    void createStatusBar();
    void createToolBars();
    void loadFile(const QString &fileName);
    void readSettings();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    void writeSettings();

    QString CH_TYPE;
    QString curFile;
    QFile file;
    bool isUntitled;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;

    QAction *openAct;
//
    QAction *readAct;
    QAction *writeAct;
    QAction *programmerAct;
//
    QAction *selectAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *saveReadable;
    QAction *closeAct;
    QAction *exitAct;

    QAction *undoAct;
    QAction *redoAct;
    QAction *saveSelectionReadable;

    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *optionsAct;
    QAction *findAct;
    QAction *findNextAct;

    QHexEdit *hexEdit;
    OptionsDialog *optionsDialog;
    SearchDialog *searchDialog;
//
    EepromSize *eepromSize;
    Programmer *programmer;
    QLabel *lbChipType, *lbChipTypeName;
    QLabel *lbAddress, *lbAddressName;
    QLabel *lbOverwriteMode, *lbOverwriteModeName;
    QLabel *lbSize, *lbSizeName;
};

#endif
