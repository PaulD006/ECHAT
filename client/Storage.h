#pragma once
#include <QString>
#include <vector>

struct Contact {
    QString username;
    QString publicKeyPem;
    QByteArray conversationKey; // AES key
};

namespace Storage {
    void init(const QString &baseDir);
    QString loadConfig(const QString &path, const QString &group, const QString &key);
    void saveConfig(const QString &path, const QString &group, const QString &key, const QString &value);
    std::vector<Contact> loadContacts();
    void saveContacts(const std::vector<Contact> &contacts);
    void appendHistory(const QString &user, const QByteArray &plain);
    QByteArray getConversationKey(const QString &user);
    QString getContactPublicKey(const QString &user);
}