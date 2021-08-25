#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QColorDialog>
#include <QFontDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "mainwindow.h"

/*****************************************************************************/
/* GLOBAL VARIABLES                                                          */
/*****************************************************************************/
 QString chipname;
 uint32_t chipsize;
 QString pr_error;
 uint32_t syncackpkt;
 uint8_t *readbuf;
 uint8_t *bufaddr;
 uint32_t byteoffset;
 int32_t getnextpkt;
 struct libusb_device_handle *devHandle;
 int32_t currentConfig = 0;
 Counter a;
 QByteArray buf(65536,0xff);
/*****************************************************************************/
/*      c void's                                                             */
/*****************************************************************************/
 void ch341ReadCmdMarshall(uint8_t *buffer, uint32_t addr, struct EEPROM *eeprom_info)
 {
     uint8_t *ptr = buffer;

         *ptr++ = mCH341A_CMD_I2C_STREAM; // 0
         *ptr++ = mCH341A_CMD_I2C_STM_STA; // 1
         // Write address
         *ptr++ = (mCH341A_CMD_I2C_STM_OUT) | ((*eeprom_info).addr_size+1); // 2: I2C bus adddress + EEPROM address
         uint8_t msb_addr;
         if ((*eeprom_info).addr_size >= 2) {
             // 24C32 and more
             msb_addr = addr>>16 & (*eeprom_info).i2c_addr_mask;
             *ptr++ = (EEPROM_I2C_BUS_ADDRESS | msb_addr)<<1; // 3
             *ptr++ = (addr>>8 & 0xFF); // 4
             *ptr++ = (addr>>0 & 0xFF); // 5
         } else {
             // 24C16 and less
             msb_addr = addr>>8 & (*eeprom_info).i2c_addr_mask;
             *ptr++ = (EEPROM_I2C_BUS_ADDRESS | msb_addr)<<1; // 3
             *ptr++ = (addr>>0 & 0xFF); // 4
         }
         // Read
         *ptr++ = mCH341A_CMD_I2C_STM_STA; // 6/5
         *ptr++ = mCH341A_CMD_I2C_STM_OUT | 1; // 7/6
         *ptr++ = (EEPROM_I2C_BUS_ADDRESS | msb_addr)<<1 | 1; // 8/7: Read command

         // Configuration?
         *ptr++ = 0xE0; // 9/8
         *ptr++ = 0x00; // 10/9
         if ((*eeprom_info).addr_size < 2) *ptr++ = 0x10; // x/10
         memcpy(ptr, "\x00\x06\x04\x00\x00\x00\x00\x00\x00", 9); ptr += 9; // 10
         uint32_t size_kb = (*eeprom_info).size/1024;
         *ptr++ = size_kb & 0xFF; // 19
         *ptr++ = (size_kb >> 8) & 0xFF; // 20
         memcpy(ptr, "\x00\x00\x11\x4d\x40\x77\xcd\xab\xba\xdc", 10); ptr += 10;

         // Frame 2
         *ptr++ = mCH341A_CMD_I2C_STREAM;
         memcpy(ptr, "\xe0\x00\x00\xc4\xf1\x12\x00\x11\x4d\x40\x77\xf0\xf1\x12\x00" \
                     "\xd9\x8b\x41\x7e\x00\xe0\xfd\x7f\xf0\xf1\x12\x00\x5a\x88\x41\x7e", 31);
         ptr += 31;

         // Frame 3
         *ptr++ = mCH341A_CMD_I2C_STREAM;
         memcpy(ptr, "\xe0\x00\x00\x2a\x88\x41\x7e\x06\x04\x00\x00\x11\x4d\x40\x77" \
                     "\xe8\xf3\x12\x00\x14\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00", 31);
         ptr += 31;

         // Finalize
         *ptr++ = mCH341A_CMD_I2C_STREAM; // 0xAA
         *ptr++ = 0xDF; // ???
         *ptr++ = mCH341A_CMD_I2C_STM_IN; // 0xC0
         *ptr++ = mCH341A_CMD_I2C_STM_STO; // 0x75
         *ptr++ = mCH341A_CMD_I2C_STM_END; // 0x00

         assert(ptr - buffer == CH341_EEPROM_READ_CMD_SZ);

 }
//  ch341setstream()
//  set the i2c bus speed (speed: 0 = 20kHz; 1 = 100kHz, 2 = 400kHz, 3 = 750kHz)
 int32_t ch341setstream(struct libusb_device_handle *devHandle, uint32_t speed) {
     int32_t ret;
     uint8_t ch341outBuffer[EEPROM_READ_BULKOUT_BUF_SZ], *outptr;
     int32_t actuallen = 0;

     outptr = ch341outBuffer;

     *outptr++ = mCH341A_CMD_I2C_STREAM;
     *outptr++ = mCH341A_CMD_I2C_STM_SET | (speed & 0x3);
     *outptr   = mCH341A_CMD_I2C_STM_END;

     ret = libusb_bulk_transfer(devHandle, BULK_WRITE_ENDPOINT, ch341outBuffer, 3, &actuallen, DEFAULT_TIMEOUT);

     if(ret < 0) {
       return -1;
     }
     return 0;
 }
 void cbBulkIn(struct libusb_transfer *transfer) {
     switch(transfer->status) {
         case LIBUSB_TRANSFER_COMPLETED:
 // display the contents of the BULK IN data buffer
             memcpy(readbuf + byteoffset, transfer->buffer, transfer->actual_length);
             getnextpkt = 1;
             break;
         default:
             getnextpkt = -1;
     }
     return;
 }


 void cbBulkOut(struct libusb_transfer *transfer) {
     syncackpkt = 1;
 //pr_error = pr_error + "\ncbBulkOut(): Sync/Ack received: status " + QString::number(transfer->status) +"\n";
     return;
 }


 // parseEEPsize()
 // passed an EEPROM name (case-sensitive), returns its byte size
 int32_t parseEEPsize(char* eepromname, struct EEPROM *eeprom) {
     int i;

     for(i=0; eepromlist[i].size; i++)
         if(strstr(eepromlist[i].name, eepromname)) {
             memcpy(eeprom, &(eepromlist[i]), sizeof(struct EEPROM));
             return(eepromlist[i].size);
         }
     return -1;
 }
/*****************************************************************************/
/*  INIT CH341A                                                              */
/*****************************************************************************/
answer init_ch341()
{
    answer ans;
//    struct libusb_device_handle *devHandle;
    struct libusb_device *dev;
    libusb_context *ctx;
    int addr_341, bus_341, ret, i;
    int32_t currentConfig = 0;
    int bus_speed = 1;
    uint8_t ch341DescriptorBuffer[0x12];
    static int speed_table[] = {20, 100, 400, 750};
    //INIT PROGRAMMER
            libusb_init( &ctx );
            devHandle = libusb_open_device_with_vid_pid(NULL, USB_LOCK_VENDOR, USB_LOCK_PRODUCT);

            if (!devHandle)
                {
                ans.ans_txt = ans.ans_txt + "Not found CH341A Programmer \n";
                ans.ans_byte = 0;
                return ans;
                }
                  else
               {
                ans.ans_txt = "Found CH341A Programmer \n";

                if(!(dev = libusb_get_device(devHandle)))
                {
                   ans.ans_txt = ans.ans_txt +  "Couldnt get bus number and address of device\n";
                   ans.ans_byte = 0;
                   return ans;
                }
                   else
                {
                addr_341 = libusb_get_device_address(dev);
                bus_341 = libusb_get_bus_number(dev);
                ans.ans_txt = ans.ans_txt + "Device = " + QString::number(addr_341) + "  ";
                ans.ans_txt = ans.ans_txt + "Bus = " + QString::number(bus_341) + "\n";

                if(libusb_kernel_driver_active(devHandle, DEFAULT_INTERFACE))
                    {
                     ret = libusb_detach_kernel_driver(devHandle, DEFAULT_INTERFACE);
                       if(ret) {
                          ans.ans_txt = ans.ans_txt +"Failed to detach kernel driver.\n";
                          ans.ans_byte = 0;
                          return ans; //return 0;
                               }

                    }
                     ret = libusb_get_configuration(devHandle, &currentConfig);
                     if(ret) {
                        ans.ans_txt = ans.ans_txt +"Failed to get current device configuration.\n";
                        ans.ans_byte = 0;
                        return ans;
                        //return 0;
                             }

                     if(currentConfig != DEFAULT_CONFIGURATION)
                             ret = libusb_set_configuration(devHandle, currentConfig);

                     if(ret) {
                             ans.ans_txt = ans.ans_txt +"Failed to set device configuration to " + QString::number(DEFAULT_CONFIGURATION) +"\n";
                             ans.ans_byte = 0;
                             return ans;
                             }


                     ret = libusb_claim_interface(devHandle, DEFAULT_INTERFACE); // interface 0

                     if(ret) {
                             ans.ans_txt = ans.ans_txt +"Failed to claim interface" + QString::number(DEFAULT_INTERFACE) + "\n";
                             ans.ans_byte = 0;
                             return ans;
                             }

                     ret = libusb_get_descriptor(devHandle, LIBUSB_DT_DEVICE, 0x00, ch341DescriptorBuffer, 0x12);

                         if(ret < 0) {
                             ans.ans_txt = ans.ans_txt +"Failed to get device descriptor.\n";
                             ans.ans_byte = 0;
                             return ans;
                             }
                         ans.ans_txt = ans.ans_txt + "Device reported its revision " +
                                 QString::number(ch341DescriptorBuffer[12]) + "." + QString::number(ch341DescriptorBuffer[13]) + "\n";

                         for(i=0;i<0x12;i++)
                             ans.ans_txt = ans.ans_txt + (QString::number(ch341DescriptorBuffer[i], 16).rightJustified(2, '0')) +" ";
                         ans.ans_txt = ans.ans_txt +"\n";


                         if(ch341setstream(devHandle, bus_speed) < 0) {
                         ans.ans_txt = ans.ans_txt +  "Couldnt set i2c bus speed\n";
                         ans.ans_byte = 0;
                          }
                         else ans.ans_txt = ans.ans_txt + "Set i2c bus speed to " + QString::number(speed_table[bus_speed])+ "KHz\n";
                         ans.ans_byte = 1;
}
}
            return ans;
}
/*****************************************************************************/
/* CLOSE CH341A                                                              */
/*****************************************************************************/
answer close_ch341a(struct libusb_device_handle *devHandle)
{
answer ans;
if(devHandle) {
          libusb_release_interface(devHandle, DEFAULT_INTERFACE);
          ans.ans_txt =  "Released device interface " + QString::number(DEFAULT_INTERFACE) +"\n";
          libusb_close(devHandle);
          ans.ans_txt = ans.ans_txt + "Closed USB device\n";
          libusb_exit(NULL);
          ans.ans_byte = -1;

      }
return ans;
}


/*****************************************************************************/
/* Public methods */
/*****************************************************************************/
MainWindow::MainWindow()
{
    setAcceptDrops( true );
    init();
    setCurrentFile("");
}

/*****************************************************************************/
/* Protected methods */
/*****************************************************************************/
void MainWindow::closeEvent(QCloseEvent *)
{
    writeSettings();
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->accept();
}


void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        QList<QUrl> urls = event->mimeData()->urls();
        QString filePath = urls.at(0).toLocalFile();
        loadFile(filePath);
        event->accept();
    }
}

/*****************************************************************************/
/* Private Slots */
/*****************************************************************************/
void MainWindow::about()
{
   QMessageBox::about(this, tr("About QHexEdit"),
            tr("The Prog24 is a free I2C EEPROM programmer tools. The program use the CH341A programmer device. Easy steps to use:\n "
               "1. Connent your CH341A Programmer device into usb port.\n"
               "2. Select the EEPROM chip in menu - 24C01, 24C02 ...24C512.\n"
               "3. For reading from chip select the 'Read from EEPROM' item.\n"
               "4. For saving the dump press the diskette button and setting the name of file.\n"
               "5. For open the existing file press the folder icon and select the file.\n"
               "6. For writing the dump to EEPROM press the 'Write to EEPROM'' buttom."));
}

void MainWindow::dataChanged()
{
    setWindowModified(hexEdit->isModified());
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        loadFile(fileName);


    }
}

void MainWindow::optionsAccepted()
{
    writeSettings();
    readSettings();
}

void MainWindow::findNext()
{
    searchDialog->findNext();
}

bool MainWindow::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    curFile);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

void MainWindow::saveSelectionToReadableFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save To Readable File"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("QHexEdit"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return;
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);
        file.write(hexEdit->selectionToReadableString().toLatin1());
        QApplication::restoreOverrideCursor();

        statusBar()->showMessage(tr("File saved"), 2000);
    }
}

void MainWindow::saveToReadableFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save To Readable File"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("QHexEdit"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return;
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);
        file.write(hexEdit->toReadableString().toLatin1());
        QApplication::restoreOverrideCursor();

        statusBar()->showMessage(tr("File saved"), 2000);
    }
}
//
void Counter::setValue(int value)
{
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
    }
}

//
void MainWindow::setChipType(QString ch_type)
{
    lbChipType->setText(ch_type);
    chipname =  QString( lbChipType->text());
        QByteArray arr_name= chipname.toUtf8(); // to....
        chipsize =0;
        for (uint j=0; j<10; j++)
        {
            if (QString(eepromlist[j].name) == chipname)
            {
                chipsize = eepromlist[j].size;
            }
        }
        setSize(chipsize);
        buf.resize(chipsize);

        if (!file.exists())
        {
           for (uint_least32_t i=0; i<chipsize; i++)
             {
               buf[i]=0xff;
             }
           hexEdit->setData(buf);
        }
}
//

void MainWindow::setAddress(qint64 address)
{
    lbAddress->setText(QString("%1").arg(address, 1, 16));
}

void MainWindow::setOverwriteMode(bool mode)
{
    QSettings settings;
    settings.setValue("OverwriteMode", mode);
    if (mode)
        lbOverwriteMode->setText(tr("Overwrite"));
    else
        lbOverwriteMode->setText(tr("Insert"));
}

void MainWindow::setSize(qint64 size)
{
    lbSize->setText(QString("%1").arg(size));
}

void MainWindow::showOptionsDialog()
{
    optionsDialog->show();
}

void MainWindow::showSearchDialog()
{
    searchDialog->show();
}
//
void MainWindow::showEepromSize()
{
    eepromSize->show();
}
void MainWindow::showProgrammer()
{
    programmer->show();
}

void MainWindow::setValue(int val)
{
    programmer->setValue(val);
}
/*****************************************************************************/
/* Private Methods */
/*****************************************************************************/
void MainWindow::init()
{
    setAttribute(Qt::WA_DeleteOnClose);
    optionsDialog = new OptionsDialog(this);
    connect(optionsDialog, SIGNAL(accepted()), this, SLOT(optionsAccepted()));
    isUntitled = true;

    hexEdit = new QHexEdit;
    setCentralWidget(hexEdit);
    connect(hexEdit, SIGNAL(overwriteModeChanged(bool)), this, SLOT(setOverwriteMode(bool)));
    connect(hexEdit, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
    searchDialog = new SearchDialog(hexEdit, this);
//
    eepromSize = new EepromSize(this);
    programmer = new Programmer(this);
    QObject::connect(&a, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));

//

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    readSettings();

    setUnifiedTitleAndToolBarOnMac(true);
    hexEdit->setData(buf);
}
//
int MainWindow::readActBt()
{
        answer ans_info;
        int i;
        int32_t ret = 0;
        libusb_context *ctx;
        QMessageBox prt_error;
        libusb_init( &ctx );
        //read
        uint8_t ch341outBuffer[EEPROM_READ_BULKOUT_BUF_SZ];
        uint8_t ch341inBuffer[IN_BUF_SZ];
        int readpktcount = 0;
        uint32_t bytestoread;
        struct libusb_transfer *xferBulkIn, *xferBulkOut;
        struct timeval tv = {0, 100};                   // our async polling interval
        struct EEPROM eeprom_info;
        int eepromsize = 0;
        char eepromname[12];
        //write
        chipname =  QString( lbChipType->text());
        QByteArray arr_name= chipname.toUtf8(); // to....
        char* e_name =  arr_name.data();
        chipsize =0;
        for (uint j=0; j<10; j++)
        {
            if (QString(eepromlist[j].name) == chipname)
            {
                chipsize = eepromlist[j].size;
            }
        }
        setSize(chipsize);
        buf.resize(chipsize);
        programmer->show();
        programmer->barShowing();

//INIT PROGRAMMER
        pr_error ="";
        readbuf = (uint8_t *) malloc(MAX_EEPROM_SIZE);   // space to store loaded EEPROM
            if(!readbuf) {
               pr_error = pr_error +  "Couldnt malloc space needed for EEPROM image\n";
                return -1;
                         }

         ans_info = init_ch341();
         pr_error = pr_error + ans_info.ans_txt;
         programmer->setTxt(pr_error);

         //EEPROM MODEL SETTINGS
         if((eepromsize = parseEEPsize(e_name, &eeprom_info)) > 0)
                                 strncpy(eepromname, e_name, 10);

         if (ans_info.ans_byte == 1)
         {

             bytestoread = eepromsize;
             memset(readbuf, 0xff, MAX_EEPROM_SIZE);
             pr_error = pr_error +"\nReading from EEPROM.\n";
             xferBulkIn  = libusb_alloc_transfer(0);
             xferBulkOut = libusb_alloc_transfer(0);
                if(!xferBulkIn || !xferBulkOut)
                  {
                      pr_error = pr_error +  "Couldnt allocate USB transfer structures\n";
                      return -1;
                  }
              byteoffset = 0;
              pr_error = pr_error +  "Allocated USB transfer structures\n";
              memset(ch341inBuffer, 0, EEPROM_READ_BULKIN_BUF_SZ);
              ch341ReadCmdMarshall(ch341outBuffer, 0, &eeprom_info); // Fill output buffer
              libusb_fill_bulk_transfer(xferBulkIn,  devHandle, BULK_READ_ENDPOINT, ch341inBuffer,
              EEPROM_READ_BULKIN_BUF_SZ, cbBulkIn, NULL, DEFAULT_TIMEOUT);
              libusb_fill_bulk_transfer(xferBulkOut, devHandle, BULK_WRITE_ENDPOINT,
              ch341outBuffer, EEPROM_READ_BULKOUT_BUF_SZ, cbBulkOut, NULL, DEFAULT_TIMEOUT);
              pr_error = pr_error + "Filled USB transfer structures\n";
              libusb_submit_transfer(xferBulkIn);
              pr_error = pr_error +  "Submitted BULK IN start packet\n";
              libusb_submit_transfer(xferBulkOut);
              pr_error = pr_error + "Submitted BULK OUT setup packet\n";
              programmer->setTxt(pr_error);

                 while (1) {
                    a.setValue(100*byteoffset/bytestoread);
                    ret = libusb_handle_events_timeout(NULL, &tv);
                        if (ret < 0 || getnextpkt == -1) {          // indicates an error
                            if (ret < 0)
                               pr_error = pr_error + "USB read error : " + QString(strerror(-ret)) + "\n";
                               libusb_free_transfer(xferBulkIn);
                               libusb_free_transfer(xferBulkOut);
                               return -1;//break;
                             }
                             if(getnextpkt == 1) {                       // callback function reports a new BULK IN packet received
                                getnextpkt = 0;                         //   reset the flag
                                readpktcount++;                         //   increment the read packet counter
                                byteoffset += EEPROM_READ_BULKIN_BUF_SZ;
                                   if (byteoffset == bytestoread)
                                     {
                                       break; //return -1;
                                      }

//                                pr_error = pr_error + "\nRe-submitting transfer request to BULK IN endpoint\n";
                                  libusb_submit_transfer(xferBulkIn);     // re-submit request for next BULK IN packet of EEPROM data
                                    if(syncackpkt)
                                      syncackpkt = 0;
                                   // if 4th packet received, we are at end of 0x80 byte data block,
                                   // if it is not the last block, then resubmit request for data
                                    if(readpktcount==4) {
                     //                pr_error = pr_error + "\nSubmitting next transfer request to BULK OUT endpoint\n";
                                       readpktcount = 0;
                                       ch341ReadCmdMarshall(ch341outBuffer, byteoffset, &eeprom_info); // Fill output buffer
                                       libusb_fill_bulk_transfer(xferBulkOut, devHandle, BULK_WRITE_ENDPOINT, ch341outBuffer,
                                         EEPROM_READ_BULKOUT_BUF_SZ, cbBulkOut, NULL, DEFAULT_TIMEOUT);
                                         libusb_submit_transfer(xferBulkOut);// update transfer struct (with new EEPROM page offset)
                                                                             // and re-submit next transfer request to BULK OUT endpoint
                                                     }
                                                 }


                                             }


    //CLOSE DEVICE
            if (byteoffset>0)
              {
               a.setValue(100*byteoffset/bytestoread);
              }

    for (i = 0; i< eepromsize; i++)
    {
        buf[i] = readbuf[i];
    }
        hexEdit->setData(buf);
}
else //ans_info.ans_byte -->0
   {
   //BAR --> HIDDEN, IMAGE --> OFF
   programmer->setImg(false);
   programmer->barNotshowing();
   }
         ans_info = close_ch341a(devHandle);
         pr_error = pr_error + "\n" + ans_info.ans_txt;
         programmer->setTxt(pr_error);
    return 0;
}

int MainWindow::writeActBt()
{
    answer ans_info;
    uint32_t i;
    int32_t ret = 0;
    libusb_context *ctx;
    QMessageBox prt_error;
    libusb_init( &ctx );
    struct EEPROM eeprom_info;
    uint32_t eepromsize = 0;
    char eepromname[12];
    //write
    uint8_t ch341outBuffer[512/*EEPROM_WRITE_BUF_SZ*/];
    uint8_t *outptr, *bufptr;
    uint8_t i2cCmdBuffer[256];
    uint32_t byteoffset = 0;
    uint32_t bytes;
    uint16_t page_size = 0;
    uint16_t address_size = 0;
    uint16_t address_mask = 0;
    uint16_t con28 = 28;
    uint8_t addrbytecount = 0; // 24c32 and 24c64 (and other 24c??) use 3 bytes for addressing
    int32_t actuallen = 0;
    chipname =  QString( lbChipType->text());
    QByteArray arr_name= chipname.toUtf8(); // to....
    char* e_name =  arr_name.data();
    chipsize =0;
    for (uint j=0; j<10; j++)
    {
        if (QString(eepromlist[j].name) == chipname)
        {
            chipsize = eepromlist[j].size;
            page_size = eepromlist[j].page_size;
            address_size = eepromlist[j].addr_size;
            address_mask = eepromlist[j].i2c_addr_mask;
            addrbytecount = address_size+1;
        }
    }
    setSize(chipsize);
    bytes = chipsize;
     programmer->show();
     programmer->barShowing();

//INIT PROGRAMMER
    pr_error ="";
    readbuf = (uint8_t *) malloc(MAX_EEPROM_SIZE);   // space to store loaded EEPROM
        if(!readbuf) {
           pr_error = pr_error +  "Couldnt malloc space needed for EEPROM image\n";
            return -1;
                     }


//EEPROM MODEL SETTINGS
                 if((eepromsize = parseEEPsize(e_name, &eeprom_info)) > 0)
                                         strncpy(eepromname, e_name, 10);
                 programmer->setTxt(pr_error);
//WRITING
                 a.setValue(0);
                 buf =hexEdit->data();
                 for (i = 0; i< eepromsize; i++)
                     {
                       readbuf[i] = buf[i];
                     }

                 hexEdit->dataAt(0,chipsize);
                 bufptr = readbuf;

                 ans_info = init_ch341();
                 pr_error = pr_error + ans_info.ans_txt;
                 if (ans_info.ans_byte == 1)
                         {
//while bytes
                 pr_error = pr_error + "\nWritting to EEPROM\n";
                 programmer->setTxt(pr_error);
                 while(bytes) {
                         outptr = i2cCmdBuffer;
                         if (address_size >= 2) {
                             *outptr++ = (uint8_t) (0xa0 | (byteoffset >> 16 & address_mask<<1));  // EEPROM device address
                             *outptr++ = (uint8_t) (byteoffset >> 8 & 0xff);     // MSB (big-endian) byte address
                         } else {
                             *outptr++ = (uint8_t) (0xa0 | (byteoffset >> 8 & address_mask<<1));  // EEPROM device address
                         }
                         *outptr++ = (uint8_t) (byteoffset & 0xff);          // LSB of 16-bit    byte address

                         memcpy(outptr, bufptr, page_size); // Copy one page

                         byteoffset += page_size;
                         bufptr     += page_size;
                         bytes      -= page_size;

                         outptr    = ch341outBuffer;
                         uint16_t page_size_left = page_size + addrbytecount;
                         uint8_t part_no = 0;
                         uint8_t *i2cBufPtr = i2cCmdBuffer;
                         while(page_size_left) {
                             uint8_t to_write = qMin(page_size_left, con28);
  a.setValue(100*byteoffset/chipsize);
                             *outptr++ = mCH341A_CMD_I2C_STREAM;
                             if (part_no == 0) { // Start packet
                                 *outptr++ = mCH341A_CMD_I2C_STM_STA;
                             }
                             *outptr++ = mCH341A_CMD_I2C_STM_OUT | to_write;
                             memcpy(outptr, i2cBufPtr, to_write);
                             outptr += to_write;
                             i2cBufPtr += to_write;
                             page_size_left -= to_write;

                             if (page_size_left == 0) { // Stop packet
                                 *outptr++ = mCH341A_CMD_I2C_STM_STO;
                             }
                             *outptr++ = mCH341A_CMD_I2C_STM_END;
                             part_no++;
                         }
                         uint32_t payload_size = outptr - ch341outBuffer;

                         for(i=0; i < payload_size; i++) {

                         }
                         //fprintf(debugout, "\n");

                         ret = libusb_bulk_transfer(devHandle, BULK_WRITE_ENDPOINT,
                             ch341outBuffer, payload_size, &actuallen, DEFAULT_TIMEOUT);

                         if(ret < 0) {
                             pr_error = pr_error + "Failed to write to EEPROM\n";
                             return -1;
                         }

                         //fprintf(debugout, "Writing [aa 5a 00] to EEPROM\n");   // Magic CH341a packet! Undocumented, unknown purpose

                         outptr    = ch341outBuffer;
                         *outptr++ = mCH341A_CMD_I2C_STREAM;
                         *outptr++ = 0x5a;                           // what is this 0x5a??
                         *outptr++ = mCH341A_CMD_I2C_STM_END;

                         ret = libusb_bulk_transfer(devHandle, BULK_WRITE_ENDPOINT, ch341outBuffer, 3, &actuallen, DEFAULT_TIMEOUT);

                         if(ret < 0) {
                             fprintf(stderr, "Failed to write to EEPROM: '%s'\n", strerror(-ret));
                             return -1;
                         }

                     }

}
                 else //ans_info.ans_byte -->0
                    {
                    //BAR --> HIDDEN, IMAGE --> OFF
                    programmer->setImg(false);
                    programmer->barNotshowing();
                    }

//CLOSE DEVICE
    if (byteoffset>0)
      {
//       a.setValue(100*byteoffset/chipsize);
      }
   ans_info = close_ch341a(devHandle);
   pr_error = pr_error + "\n" + ans_info.ans_txt;
   programmer->setTxt(pr_error);




return 0;
}
void MainWindow::selectActBt()
{
    eepromSize->show();
//    QMessageBox::about(this, "Title", QString::number(0));

}
void MainWindow::programmerActBt()
{
    answer ans_info;
    //clear variables
    programmer->show();
    pr_error = "";
    programmer->setTxt(pr_error);
    programmer->setImg(FALSE);

    //init ch341 programmer, get informations about this programmer
    ans_info = init_ch341();
    pr_error = pr_error + ans_info.ans_txt;
    programmer->setImg(ans_info.ans_byte);
    programmer->setTxt(pr_error);
    //CLOSE DEVICE
    ans_info = close_ch341a(devHandle);
    pr_error = pr_error + "\n" + ans_info.ans_txt;
    programmer->setTxt(pr_error);
    programmer->barNotshowing();

}
//
void MainWindow::createActions()
{
    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
//
    readAct = new QAction(QIcon(":/images/read.png"), tr("&Read..."), this);
    readAct->setStatusTip(tr("Read from EEPROM"));
    connect(readAct, SIGNAL(triggered()), this, SLOT(readActBt()));


    writeAct = new QAction(QIcon(":/images/write.png"), tr("&Write..."), this);
    writeAct->setStatusTip(tr("Write to EEPROM"));
    connect(writeAct, SIGNAL(triggered()), this, SLOT(writeActBt()));

    selectAct = new QAction(QIcon(":/images/chip_type.png"), tr("&Select..."), this);
    selectAct->setStatusTip(tr("Select EEPROM type"));
    connect(selectAct, SIGNAL(triggered()), this, SLOT(selectActBt()));
//

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    saveReadable = new QAction(tr("Save &Readable..."), this);
    saveReadable->setStatusTip(tr("Save document in readable form"));
    connect(saveReadable, SIGNAL(triggered()), this, SLOT(saveToReadableFile()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    undoAct = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    connect(undoAct, SIGNAL(triggered()), hexEdit, SLOT(undo()));

    redoAct = new QAction(QIcon(":/images/redo.png"), tr("&Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo);
    connect(redoAct, SIGNAL(triggered()), hexEdit, SLOT(redo()));

    saveSelectionReadable = new QAction(tr("&Save Selection Readable..."), this);
    saveSelectionReadable->setStatusTip(tr("Save selection in readable form"));
    connect(saveSelectionReadable, SIGNAL(triggered()), this, SLOT(saveSelectionToReadableFile()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    findAct = new QAction(QIcon(":/images/find.png"), tr("&Find/Replace"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setStatusTip(tr("Show the Dialog for finding and replacing"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(showSearchDialog()));

    findNextAct = new QAction(tr("Find &next"), this);
    findNextAct->setShortcuts(QKeySequence::FindNext);
    findNextAct->setStatusTip(tr("Find next occurrence of the searched pattern"));
    connect(findNextAct, SIGNAL(triggered()), this, SLOT(findNext()));

    optionsAct = new QAction(tr("&Options"), this);
    optionsAct->setStatusTip(tr("Show the Dialog to select applications options"));
    connect(optionsAct, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));
//
    programmerAct = new QAction(tr("&Programmer"), this);
    programmerAct->setStatusTip(tr("Progmammer connecting info"));
    connect(programmerAct, SIGNAL(triggered()), this, SLOT(programmerActBt()));



//
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(saveReadable);
    fileMenu->addSeparator();
//
    fileMenu->addAction(selectAct);
    fileMenu->addAction(readAct);
    fileMenu->addAction(writeAct);
//
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addAction(saveSelectionReadable);
    editMenu->addSeparator();
    editMenu->addAction(findAct);
    editMenu->addAction(findNextAct);
    editMenu->addSeparator();
    editMenu->addAction(optionsAct);
    editMenu->addAction(programmerAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createStatusBar()
{
//
    // Chip model label
    lbChipTypeName = new QLabel();
    lbChipTypeName->setText(tr("Chip:"));
    statusBar()->addPermanentWidget(lbChipTypeName);
    lbChipType = new QLabel();
    lbChipType->setFrameShape(QFrame::Panel);
    lbChipType->setFrameShadow(QFrame::Sunken);
    lbChipType->setMinimumWidth(50);
    lbChipType->setText("24c01");
    statusBar()->addPermanentWidget(lbChipType);
    connect(eepromSize, SIGNAL(on_listWidget_click(QString)), this, SLOT(setChipType(QString)));
//
    // Address Label
    lbAddressName = new QLabel();
    lbAddressName->setText(tr("Address:"));
    statusBar()->addPermanentWidget(lbAddressName);
    lbAddress = new QLabel();
    lbAddress->setFrameShape(QFrame::Panel);
    lbAddress->setFrameShadow(QFrame::Sunken);
    lbAddress->setMinimumWidth(50);
    statusBar()->addPermanentWidget(lbAddress);
    connect(hexEdit, SIGNAL(currentAddressChanged(qint64)), this, SLOT(setAddress(qint64)));

    // Size Label
    lbSizeName = new QLabel();
    lbSizeName->setText(tr("Size:"));
    statusBar()->addPermanentWidget(lbSizeName);
    lbSize = new QLabel();
    lbSize->setFrameShape(QFrame::Panel);
    lbSize->setFrameShadow(QFrame::Sunken);
    lbSize->setMinimumWidth(50);
    statusBar()->addPermanentWidget(lbSize);
    connect(hexEdit, SIGNAL(currentSizeChanged(qint64)), this, SLOT(setSize(qint64)));

    // Overwrite Mode Label
    lbOverwriteModeName = new QLabel();
    lbOverwriteModeName->setText(tr("Mode:"));
    statusBar()->addPermanentWidget(lbOverwriteModeName);
    lbOverwriteMode = new QLabel();
    lbOverwriteMode->setFrameShape(QFrame::Panel);
    lbOverwriteMode->setFrameShadow(QFrame::Sunken);
    lbOverwriteMode->setMinimumWidth(50);
    statusBar()->addPermanentWidget(lbOverwriteMode);
    setOverwriteMode(hexEdit->overwriteMode());

    statusBar()->showMessage(tr("Ready"), 2000);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);
    editToolBar->addAction(findAct);
    editToolBar = addToolBar(tr("Chip"));
//

    editToolBar->addAction(selectAct);
    editToolBar->addAction(readAct);
    editToolBar->addAction(writeAct);
//

}

void MainWindow::loadFile(const QString &fileName)
{
    file.setFileName(fileName);
    if (!hexEdit->setData(file)) {
        QMessageBox::warning(this, tr("QHexEdit"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);

//***************************************************************
    for (uint j=0; j<10; j++)
    {
        if (eepromlist[j].size == file.size())
        {
            chipname = eepromlist[j].name;
            chipsize = eepromlist[j].size;
        }
    }
    QMessageBox::about(this, "Title", QString(chipname));
    MainWindow::setSize(chipsize);
    MainWindow::setChipType(chipname);

//****************************************************************
}

void MainWindow::readSettings()
{
    QSettings settings;
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(610, 460)).toSize();
    move(pos);
    resize(size);

    hexEdit->setAddressArea(settings.value("AddressArea").toBool());
    hexEdit->setAsciiArea(settings.value("AsciiArea").toBool());
    hexEdit->setHighlighting(settings.value("Highlighting").toBool());
    hexEdit->setOverwriteMode(settings.value("OverwriteMode").toBool());
    hexEdit->setReadOnly(settings.value("ReadOnly").toBool());

    hexEdit->setHighlightingColor(settings.value("HighlightingColor").value<QColor>());
    hexEdit->setAddressAreaColor(settings.value("AddressAreaColor").value<QColor>());
    hexEdit->setSelectionColor(settings.value("SelectionColor").value<QColor>());
    hexEdit->setFont(settings.value("WidgetFont").value<QFont>());

    hexEdit->setAddressWidth(settings.value("AddressAreaWidth").toInt());
    hexEdit->setBytesPerLine(settings.value("BytesPerLine").toInt());
}

bool MainWindow::saveFile(const QString &fileName)
{
    QString tmpFileName = fileName + ".~tmp";

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFile file(tmpFileName);
    bool ok = hexEdit->write(file);
    if (QFile::exists(fileName))
        ok = QFile::remove(fileName);
    if (ok)
    {
        file.setFileName(tmpFileName);
        ok = file.copy(fileName);
        if (ok)
            ok = QFile::remove(tmpFileName);
    }
    QApplication::restoreOverrideCursor();

    if (!ok) {
        QMessageBox::warning(this, tr("QHexEdit"),
                             tr("Cannot write file %1.")
                             .arg(fileName));
        return false;
    }

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = fileName.isEmpty();
    setWindowModified(false);
    if (fileName.isEmpty())
        setWindowFilePath("QHexEdit");
    else
        setWindowFilePath(curFile + " - QHexEdit");
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}
