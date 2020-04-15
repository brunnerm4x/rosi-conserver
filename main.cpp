#include <QCoreApplication>
#include <QDebug>

#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInfo() << "Welcome to ROSIConserver - backup server for Realtime Online Streaming with IOTA plugin. Vers. 0 . 3 ." << BUILD;
    new Server(nullptr);

    return a.exec();
}
