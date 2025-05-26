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
    /// Write out a new .contact file for `peerUser` and return the path.
    QString exportContact(const QString& peerUser);
    /// Read in the given .contact file, register the key, and return that userId.
    QString importContact(const QString& filename);
    /// Persist the AES key for a peer user
    void saveConversationKey(const QString &peerUser, const QByteArray &key);
}