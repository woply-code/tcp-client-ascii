#include "TcpClientWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TcpClientWindow window;
    window.show();

    return app.exec();
}
