#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#ifdef OPUS
#include <bopus.h>
#endif
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioOutput>
#include <QJsonArray>

class Client : public QObject
{
    Q_OBJECT
    QWebSocket* serverCommand;
    QWebSocket* serverData;
    QString clName,_clName;
#ifdef OPUS
    BOpus::Encoder* encoder = nullptr;
    BOpus::Decoder* decoder = nullptr;
#endif
    QAudioOutput* outputAudio = nullptr;
    QAudioInput* inputAudio = nullptr;

    QByteArray buffer;

    void initAudioSystem();

    void closeAudioSystem();

    void initAudioSender();

    void initAudioReceiver();

    QAudioFormat getFormat();

    void auth(bool success,QString errmsg);

    void join(QString name,bool success,QString errmsg);

    void sendData(bool success,QString msg);

    void onUser(QJsonArray array, bool success, QString msg);

public:
    explicit Client(QObject *parent = 0);

    void connectToServer(QString server);

    void makeAuth(QString username);

    void joinToUser(QString username);

    void disconnectFromServer();

    ~Client();

signals:
    void connected();
    void disconnected();
    void error(QString error);
    void joint(QString name);
    void disjoint(QString name);
    void startSendData();
    void madeAuth();
    void allUsers(QStringList list);
public slots:
    void onCommandReceived(const QByteArray &message);
};

#endif // CLIENT_H
