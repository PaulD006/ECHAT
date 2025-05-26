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
    if(parts.size() != 2) return false;
    QString host = parts[0];
    quint16 port = parts[1].toUInt();

    // Create/reset the socket
    if(m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
    }
    m_socket = new QSslSocket(this);

    // 1) Disable ALL verification (for now)
    m_socket->setPeerVerifyMode(QSslSocket::VerifyNone);
    connect(m_socket,
            SIGNAL(sslErrors(const QList<QSslError>&)),
            m_socket,
            SLOT(ignoreSslErrors()));

    // 2) Kick off the TLS connect
    m_socket->connectToHostEncrypted(host, port);

    // 3) Wait for the handshake, not just TCP
    if(!m_socket->waitForEncrypted(5000)) {
        qWarning() << "TLS handshake failed:" << m_socket->errorString();
        return false;
    }
    return true;
}

bool NetworkClient::login(const QString &user, const QString &pass) {
    // Build JSON
    QJsonObject obj{{"type","login"},{"user",user},{"pass",pass}};
    QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    // Wrap it in a proper HTTP POST
    QByteArray req =
        "POST / HTTP/1.1\r\n"
        "Host: " + m_socket->peerAddress().toString().toUtf8() + "\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " + QByteArray::number(json.size()) + "\r\n"
        "Connection: close\r\n"     // tell the server you’re done
        "\r\n" +
        json;

    // Send request fully
    qint64 written = m_socket->write(req);
    if (written != req.size()) return false;
    if (!m_socket->waitForBytesWritten(3000)) return false;

    // Read the response
    if (!m_socket->waitForReadyRead(5000)) return false;
    QByteArray resp = m_socket->readAll();

    // Simple parse: split headers/body
    int idx = resp.indexOf("\r\n\r\n");
    if (idx < 0) return false;
    QByteArray body = resp.mid(idx + 4);
    QJsonObject reply = QJsonDocument::fromJson(body).object();
    return reply.value("type").toString() == "login_response"
        && reply.value("status").toString() == "ok";
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