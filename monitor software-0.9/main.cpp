#include "widget.h"
#include <QSystemTrayIcon>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("GPS物联");
    a.setOrganizationDomain("GPS.edu.cn");
    a.setQuitOnLastWindowClosed(false);
    Widget w;
    w.setWindowFlags((w.windowFlags() & ~Qt::WindowMinMaxButtonsHint));
    w.show();
    return a.exec();
}
