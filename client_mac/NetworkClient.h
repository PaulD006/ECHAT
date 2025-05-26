#pragma once

#include <QObject>
#include <QSslSocket>
#include <QString>
#include <QByteArray>
#include <functional>

class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject *parent=nullptr);
    ~NetworkClient() override;
    bool connectToServer(const QString &hostPort);
    bool login(const QString &user, const QString &pass);
    void disconnectFromServer();
    void sendMessage(const QString &to, const QByteArray &data);
    void setMessageHandler(std::function<void(const QString&, const QByteArray&)> handler);
signals:
    void messageReceived(const QString&, const QByteArray&);
private slots:
    void onReadyRead();
private:
    QSslSocket *m_socket;
    std::function<void(const QString&, const QByteArray&)> m_handler;
    QByteArray m_buffer;
};