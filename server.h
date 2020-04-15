#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QDebug>

#include "request.h"

#define REQUESTSLOTCNT 256

class Server : public QObject
{
    Q_OBJECT

    public:
        Server(QObject *parent);

    private:
        QTcpServer *server;

    private slots:
        void handleConnection();
};

#endif // SERVER_H
