#include <QApplication>
#include <QIcon>

#include "mainwindow.h"




int main(int argc, char *argv[])
{
    //Q_INIT_RESOURCE(qhexedit);
    QApplication app(argc, argv);
    app.setApplicationName("24Cxx Programmer");
    app.setOrganizationName("24Cxx Programmer");
    app.setWindowIcon(QIcon(":images/24Cxx_icon64.ico"));

    // Identify locale and load translation if available
    QString locale = QLocale::system().name();
    QTranslator translator;
    translator.load(QString("qhexedit_") + locale);
    app.installTranslator(&translator);

    MainWindow *mainWin = new MainWindow;
    mainWin->show();

    return app.exec();
}
