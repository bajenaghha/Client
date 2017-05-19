#include "client.h"

void Client::initAudioSystem()
{
    buffer.clear();
#ifdef OPUS
    delete encoder;
    delete decoder;
#endif
    delete outputAudio;
    delete inputAudio;

    auto format = getFormat();
#ifdef OPUS
    encoder = new BOpus::Encoder(format, BOpus::VOIP , 24000);
    decoder = new BOpus::Decoder(format);
#endif
    outputAudio = new QAudioOutput(format,this);
#ifdef OPUS
    outputAudio->setBufferSize(BOpus::frameSize(format)/2);
#endif
    inputAudio = new QAudioInput(format,this);
}

void Client::closeAudioSystem()
{
    buffer.clear();

    if (outputAudio)
    {
        outputAudio->stop();
        outputAudio->deleteLater();
    }

    if (inputAudio)
    {
        inputAudio->stop();
        inputAudio->deleteLater();
    }
#ifdef OPUS
    delete encoder;
    delete decoder;
#endif
    inputAudio =nullptr;
    outputAudio =nullptr;
#ifdef OPUS
    encoder=nullptr;
    decoder = nullptr;
#endif
}

void Client::initAudioSender()
{
    auto input = inputAudio->start();
#ifdef OPUS
    auto fsize = (BOpus::frameSize(inputAudio->format())*60)/1000;
#endif
    connect(input,&QIODevice::readyRead,[=](){
#ifdef OPUS
        buffer.append(input->readAll());
        while (buffer.size()>=fsize) {

            QByteArray opusdata = encoder->encode(QByteArray::fromRawData(buffer.data(),fsize));
            buffer.remove(0,fsize);
            serverData->sendBinaryMessage(opusdata);
        }
#else
        serverData->sendBinaryMessage(input->readAll());
#endif
    });
}

void Client::initAudioReceiver()
{
    QIODevice* output = outputAudio->start();

    connect(serverData,&QWebSocket::binaryMessageReceived,[=](QByteArray opusdata){
        #ifdef OPUS
        output->write(decoder->decode(opusdata));
#else
        output->write(opusdata);
#endif
    });
}

QAudioFormat Client::getFormat()
{
    QAudioFormat format;
    //    // Set up the desired format, for example:
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    return format;
}

void Client::auth(bool success, QString errmsg)
{
    qDebug()<<success <<errmsg;
    if (success)
    {
        clName = _clName;
        emit madeAuth();
    }
    else
        emit error(errmsg);
}

void Client::join(QString name, bool success, QString errmsg)
{
    qDebug()<<"Join: "<<name<<success<<errmsg;
    if (success)
    {
        QUrl url = serverCommand->requestUrl();
        url.setPort(url.port()+1);
        serverData->disconnect(); //disconnect all slots

        connect(serverData,&QWebSocket::connected,[=](){
            emit this->joint(name);
            serverData->sendTextMessage(clName);
        });
        connect(serverData,&QWebSocket::disconnected,[=](){
            closeAudioSystem();
            serverData->disconnect();
            emit this->disjoint(name);
        });

        serverData->open(url);
    }
    else
    {
        emit error(errmsg);
    }
}

void Client::sendData(bool success, QString msg)
{
    if (!success)
    {
        emit error(msg);
        return;
    }
    initAudioSystem();
    initAudioReceiver();
    initAudioSender();
    emit startSendData();
}

void Client::onUser(QJsonArray array, bool success, QString msg)

{
    if (!success)
        emit error(msg);
    else
    {
        QStringList list;
        for(auto item:array)
        {
            list<<item.toString();
        }
        emit allUsers(list);
    }
}

Client::Client(QObject *parent) : QObject(parent)
{
    serverCommand = new QWebSocket(QString(),QWebSocketProtocol::VersionLatest, this);
    serverData = new QWebSocket(QString(),QWebSocketProtocol::VersionLatest, this);

    connect(serverCommand,&QWebSocket::binaryMessageReceived,this,&Client::onCommandReceived);
    connect(serverCommand,&QWebSocket::connected,[&](){
        emit connected();
    });
    connect(serverCommand,&QWebSocket::disconnected,[&](){
        emit disconnected();
    });

    connect(serverCommand,static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
            [&](QAbstractSocket::SocketError err){
        emit error(serverCommand->errorString());
    });


}

void Client::connectToServer(QString server)
{
    clName = QString();
    serverCommand->open(server);
}

void Client::makeAuth(QString username)
{
    _clName = username;

    QJsonObject servercall;
    servercall["requestType"] = "auth";

    QJsonObject auth;
    auth["userName"] = username;

    servercall["auth"] = auth;
    serverCommand->sendBinaryMessage(QJsonDocument(servercall).toBinaryData());
}

void Client::joinToUser(QString username)
{
    QJsonObject servercall;
    servercall["requestType"] = "joinPeer";

    QJsonObject peer;
    peer["userName"] = username;

    servercall["peer"] = peer;
    serverCommand->sendBinaryMessage(QJsonDocument(servercall).toBinaryData());
}

void Client::disconnectFromServer()
{
    serverCommand->close();
}

Client::~Client()
{
    //TODO
}

void Client::onCommandReceived(const QByteArray &message)
{
    auto datadoc = QJsonDocument::fromBinaryData(message);
    if (!datadoc.isObject())
    {
        //TODO:
        //error message
        qDebug()<<"Object received from server is not a Json.";
        emit error("Object received from server is not a Json.");
        return;
    }
    auto data = datadoc.object();
    if (data["isOK"]==false)
    {
        qDebug()<<"isOK is not true";
        emit error("isOK is not true");
        return;
    }

    if (data["responseType"]=="auth")
    {
        auto authObj = data["auth"].toObject();
        auth(authObj["success"].toBool(),authObj["msg"].toString());
    }
    else if(data["responseType"]=="joinPeer")
    {
        auto peerObj = data["peer"].toObject();
        join(peerObj["userName"].toString(),peerObj["success"].toBool(),peerObj["msg"].toString());
    }
    else if (data["responseType"]=="sendData")
    {
        auto sendDataObj = data["sendData"].toObject();
        sendData(sendDataObj["success"].toBool(),sendDataObj["msg"].toString());
    }
    else if (data["responseType"]=="getUsers")
    {
        auto getUsersObj = data["getUsers"].toObject();
        onUser(getUsersObj["users"].toArray(), getUsersObj["success"].toBool(),getUsersObj["msg"].toString());
    }
}
