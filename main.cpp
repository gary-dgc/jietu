#include "dialog.h"
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QGraphicsView>
#include <QMainWindow>
//#include <QMainWindowLayout>
#include "cscreenshotmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
//    w.setWindowFlags(w.windowFlags() | Qt::Popup);
    w.show();
//    CScreenShotManager manager;
//    manager.startScreenShot();

    return a.exec();
}
