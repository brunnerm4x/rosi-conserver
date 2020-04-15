#include "server.h"

Server::Server(QObject *parent) : QObject(parent)
{
    server = new QTcpServer();

    connect(server, SIGNAL(newConnection()), this, SLOT(handleConnection()));

    server->listen(QHostAddress::Any, 12000);
}

void Server::handleConnection()
{
    qDebug() << "\n --> New incoming connection!";

    QTcpSocket *socket = server->nextPendingConnection();

    new Request(socket, this);
}
