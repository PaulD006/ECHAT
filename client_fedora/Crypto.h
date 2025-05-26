#pragma once
#include <QString>
#include <QByteArray>

namespace Crypto {
    void init(); 
    QByteArray generateSessionKeyPair(QString &publicPem, QString &privatePem);
    QByteArray encryptRSA(const QString &pubPem, const QByteArray &data);
    QByteArray decryptRSA(const QString &privPem, const QByteArray &data);
    QByteArray aesEncrypt(const QByteArray &key, const QByteArray &iv, const QByteArray &plain);
    QByteArray aesDecrypt(const QByteArray &key, const QByteArray &iv, const QByteArray &cipher);
    QByteArray generateRandom(int length);
    QString getSessionPrivateKey();
}