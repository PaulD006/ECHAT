#include "NetworkClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>

NetworkClient::NetworkClient(QObject *parent)
 : QObject(parent), m_socket(new QSslSocket(this))
{
    connect(m_socket, &QSslSocket::readyRead, this, &NetworkClient::onReadyRead);
}

NetworkClient::~NetworkClient() {
    m_socket->disconnectFromHost();
}

bool NetworkClient::connectToServer(const QString &hostPort) {
    auto parts = hostPort.split(':');
    if(parts.size()!=2) return false;
    QString host = parts[0];
    quint16 port = parts[1].toUInt();
    m_socket->connectToHostEncrypted(host, port);
    return m_socket->waitForConnected(5000);
}

bool NetworkClient::login(const QString &user, const QString &pass) {
    QJsonObject req { {"type","login"},{"user",user},{"pass",pass} };
    auto out = QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n";
    m_socket->write(out);
    if(!m_socket->waitForReadyRead(5000)) return false;
    auto line = m_socket->readLine();
    auto resp = QJsonDocument::fromJson(line).object();
    return resp["type"]=="login_response" && resp["status"]=="ok";
}

void NetworkClient::disconnectFromServer() {
    m_socket->disconnectFromHost();
}

void NetworkClient::sendMessage(const QString &to, const QByteArray &data) {
    // data is already outer-encrypted binary; base64 encode
    QJsonObject msg {
        {"type","msg"},
        {"to", to},
        {"data", QString::fromLatin1(data.toBase64())}
    };
    auto out = QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n";
    m_socket->write(out);
}

void NetworkClient::setMessageHandler(std::function<void(const QString&, const QByteArray&)> handler) {
    m_handler = std::move(handler);
}

void NetworkClient::onReadyRead() {
    while(m_socket->canReadLine()) {
        auto line = m_socket->readLine();
        auto obj = QJsonDocument::fromJson(line).object();
        if(obj["type"]=="msg") {
            QString from = obj["from"].toString();
            QByteArray data = QByteArray::fromBase64(obj["data"].toString().toLatin1());
            if(m_handler) m_handler(from, data);
            emit messageReceived(from,data);
        }
    }
}