#ifndef NEWREQUEST_H
#define NEWREQUEST_H

#include <QObject>
#include <QAbstractSocket>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QDir>
#include <QDateTime>
#include <QFileInfoList>

class Request : public QObject
{
    Q_OBJECT

    private:
        QAbstractSocket *socket;
        QString *rawData;
        QString rawHead;
        QString rawBody;

        bool isValid();
        int isValidUserPw(bool create);

        struct RequestParsed{
            QString name;   // name of request
            QString rosiVersion;
            QString userId;
            QString checksum;
            QString pw;
            QString data;
            int datalen;
        }request;

    public:
        Request(QAbstractSocket *s, QObject *parent);
        ~Request ();

    private slots:
        void readData();
        void handleError(QAbstractSocket::SocketError);
        void handleError(QString error);
        void executeRequest();

        void addBackup();
        void restoreBackup();
        void listBackups();
        void testUser();      

    signals:
        void requestFinished();

};

#endif // NEWREQUEST_H
