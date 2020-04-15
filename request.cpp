#include "request.h"

Request::Request(QAbstractSocket *s, QObject *parent) : QObject(parent)
{
    socket = s;
    rawData = new QString();

    connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(handleError(QAbstractSocket::SocketError)));
}

Request::~Request ()
{
    delete rawData;
    socket->deleteLater();
}

void Request::readData()
{
    qDebug() << "Request received.";
    if(rawData->length() == 0)
    {
        // First package, with header ...
        rawData->append(QString(socket->readAll()));
        QStringList sl = rawData->split("\r\n\r\n");
        if(sl.length() < 2)
        {
            qDebug() << "INVALID HTTP FORMAT!";
            socket->close();
            this->deleteLater();
            return;
        }
        rawHead = sl.at(0);
        request.data = sl.at(1);

        sl = rawHead.split("\r\n");
        // check if it is correct POST http request
        if(!sl.at(0).contains("HTTP/1.1"))       // TODO: better integrity checking!!
        {
            qDebug() << "INVALID HTTP REQUEST!";
            socket->close();
            this->deleteLater();
            return;
        }

        qDebug() << "IS VALID HTTP";
        for(int i = 1; i < sl.length(); i++)
        {
            if(!sl.at(i).contains(':'))
            {
                continue;
            }

            QString val = sl.at(i).split(':').at(1).simplified();

            if(sl.at(i).contains("Rosi-Request:", Qt::CaseInsensitive))
            {
                request.name = val;
            }
            else if(sl.at(i).contains("User-Agent:", Qt::CaseInsensitive))
            {
                request.rosiVersion = val;
            }
            else if(sl.at(i).contains("User-Id:", Qt::CaseInsensitive))
            {
                request.userId = val;
            }
            else if(sl.at(i).contains("Request-Checksum:", Qt::CaseInsensitive))
            {
                request.checksum = val;
            }
            else if(sl.at(i).contains("Content-Length:", Qt::CaseInsensitive))
            {
                request.datalen = val.toInt();
            }
            else if(sl.at(i).contains("Pw-Hash:", Qt::CaseInsensitive))
            {
                request.pw = val;
            }
        }

        qDebug() << "Request: " << request.name;
        qDebug() << "From user " << request.userId << " with agent " << request.rosiVersion;
        qDebug() << "Data checksum: " << request.checksum;
     //   qDebug() << "Data: " << request.data;
    }
    else
    {
        // Additional data
        QString *data = new QString(socket->readAll());
        rawData->append(data);
        request.data.append(data);
        delete data;
    }

    if(request.data.length() == request.datalen && isValid())
    {
        qDebug() << "Checksum OK.";
        executeRequest();

        socket->waitForBytesWritten();
        socket->close();
        this->deleteLater();
    }
    else
    {
        qDebug() << "Invalid Data length/Checksum, waiting for additional tcp package...";
    }
}

// Check integrity of request by calcualting checksum
bool Request::isValid()
{
    QCryptographicHash *reqHash = new QCryptographicHash(QCryptographicHash::Sha256);
    QString data = request.name + request.rosiVersion + request.userId + request.pw + request.data;
    reqHash->addData(data.toLatin1());
    QString hash = QString(reqHash->result().toHex());
    delete reqHash;

    return hash == request.checksum;
}

void Request::handleError(QAbstractSocket::SocketError error)
{
    qDebug() << "Socket error occurred" << error;

}

void Request::handleError(QString error)
{
    qDebug() << "Error occurred: " << error;
}

void Request::executeRequest()
{
    if(request.name == "Add-Backup")
    {
        addBackup();
    }
    else if(request.name == "Restore-Backup")
    {
        restoreBackup();
    }
    else if(request.name == "List-Backups")
    {
        listBackups();
    }
    else if(request.name == "Test-User")
    {
        testUser();
    }
}


void Request::addBackup()
{
    // Save to disk
    if(isValidUserPw(true) < 0)
    {
        socket->write(QString("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\n\r\nERROR INVALID USER PASSWORD").toLatin1());
        return;
    }
    QString path("data/");
    QString filename(request.userId + "." + ("000000000000" + QString::number(QDateTime::currentDateTime().toTime_t())).right(12));
    QDir dir;
    if(!dir.exists(path))
    {
        dir.mkpath(path);
    }
    QFile file(path + filename);
    file.open(QIODevice::WriteOnly);
    file.write(request.data.toLatin1());
    file.close();
    qDebug() << "Backup written.";

    socket->write("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\n\r\nBACKUP_SAVED");
}

void Request::restoreBackup()
{
    if(isValidUserPw(true) < 0)
    {
        socket->write(QString("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\n\r\nERROR INVALID USER PASSWORD").toLatin1());
        return;
    }

    QDir dir("data/");
    QStringList nameFilters;
    nameFilters.append(request.userId + ".????????????");
    QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name | QDir::Reversed);
    QStringList sl = request.data.split("\r\n");
    int restoreNo = -1;
    for(int i = 0, total = sl.length(); i < total; ++i)
    {
        if(!sl.at(i).contains(':'))
        {
            continue;
        }
        QString val = sl.at(i).split(':').at(1).simplified();
        if(sl.at(i).contains("Restore-Number:"))
        {
            restoreNo = val.toInt();
            break;
        }
    }
    if(restoreNo < 0 || restoreNo > files.length() - 1)
    {
        handleError("Invalid Request Data!");
        socket->write("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\n\r\nERROR: INVALID RESTORE NUMBER");
        return;
    }
    qDebug() << "Opening file: " << files.at(restoreNo).filePath();
    QFile backupFile(files.at(restoreNo).filePath());
    backupFile.open(QFile::ReadOnly);
    QString data = backupFile.readAll();

    QCryptographicHash *reqHash = new QCryptographicHash(QCryptographicHash::Sha256);
    reqHash->addData(data.toLatin1());
    QString hash = QString(reqHash->result().toHex());

    socket->write(QString("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\nData-Checksum:" + hash + "\r\n\r\n" + data).toLatin1());
    backupFile.close();
}

void Request::listBackups()
{
    if(isValidUserPw(true) < 0)
    {
        socket->write(QString("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\n\r\nBackups-Count: -1\r\nERROR INVALID USER PASSWORD").toLatin1());
        return;
    }

    QString path("data/");
    QDir dir(path);
    QStringList nameFilters;
    nameFilters.append(request.userId + ".????????????");
    QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name | QDir::Reversed);
    QString backupList("");
    for(int i = 0; i < files.length(); i++)
    {
        QStringList splittedFile = files.at(i).fileName().split(".");
        backupList += "\r\n" + QString::number(i) + ":" + splittedFile.at(splittedFile.length() - 1);
    }
    socket->write(QString("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\n\r\nBackups-Count: " + QString::number(files.length()) + backupList).toLatin1());
}


// Check if user exists, if so, check PW; if not and create == true, create new userfile
// return 0 user exist, PW OK
//        1 user not existing, NOT created
//        2 user not existing, created
//        -1 user existing, PW FAIL
//        -2 UNKNOWN ERROR
int Request::isValidUserPw(bool create)
{
    QString path("data/");
    QDir dir(path);
    if(!dir.exists(path))
    {
        dir.mkpath(path);
    }

    QFile file(path + request.userId);
    if(file.open(QIODevice::ReadOnly))
    {
        // User exists
        QString pw(file.readAll());
        pw = pw.simplified();
        file.close();

        if(QString::compare(pw, request.pw) == 0)
            return 0;    // Known user OK
        return -1;      // Known user FAIL

    }
    else
    {
        // User does not exist
        if(!create)
            return 1;

        QFile usrFileNew(path + request.userId);
        if(usrFileNew.open(QIODevice::WriteOnly))
        {
            usrFileNew.write(request.pw.toLatin1());
            usrFileNew.close();
            return 2;
        }
        else
            return -2;
    }
}

void Request::testUser()
{
    switch(isValidUserPw(false))
    {
        case 0:
            // Known user OK
            socket->write(QString("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\n\r\nKNOWN_USER").toLatin1());
            break;
        case 1:
        case 2:
            // User does not exist
            socket->write(QString("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\n\r\nNEW_USER").toLatin1());
            break;

        case -2:
        case -1:
        default:
            // Unknown user with wrong password or ERROR
            socket->write(QString("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\n\r\nWRONG_USER").toLatin1());
            break;
    }
}












