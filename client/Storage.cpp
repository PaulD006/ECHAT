#include "Storage.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "Crypto.h"

namespace Storage{
static QString s_baseDir;



static QString configDir() {
    return QDir::homePath() + "/config_echat";
}

QString exportContact(const QString& peerUser) {
    // ensure contacts folder exists
    QDir dir(configDir() + "/contacts");
    dir.mkpath(".");

    // path for the new .contact
    QString path = dir.filePath(peerUser + ".contact");

    // grab the raw AES key you generated earlier:
    QByteArray aesKey = getConversationKey(peerUser);

    // write JSON { "user": "...", "key": "<hex>" }
    QJsonObject obj;
    obj["user"] = peerUser;
    obj["key"]  = QString(aesKey.toHex());
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(QJsonDocument(obj).toJson());
    f.close();

    return path;
}

QString importContact(const QString& filename) {
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) return QString();
    auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    auto obj = doc.object();
    QString peer = obj["user"].toString();
    QByteArray key = QByteArray::fromHex(obj["key"].toString().toUtf8());

    // store that key for later chats:
    saveConversationKey(peer, key);

    return peer;
}

/// Saves the raw AES key for peerUser into config_echat/keys/peerUser.key
void saveConversationKey(const QString &peerUser, const QByteArray &key) {
    QDir dir(configDir() + "/keys");
    dir.mkpath(".");
    QString path = dir.filePath(peerUser + ".key");
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(key);
        f.close();
    }
}

void Storage::init(const QString &baseDir) {
    s_baseDir = baseDir;
    QDir d;
    if(!d.exists(baseDir)) d.mkpath(baseDir);
    if(!d.exists(baseDir + "/history")) d.mkpath(baseDir + "/history");
    if(!d.exists(baseDir + "/contacts.json")) {
        QFile f(baseDir + "/contacts.json");
        f.open(QIODevice::WriteOnly);
        f.write("{\"contacts\":[]}");
        f.close();
    }
}

QString Storage::loadConfig(const QString &path, const QString &group, const QString &key) {
    QSettings s(path, QSettings::IniFormat);
    s.beginGroup(group);
    auto v = s.value(key).toString();
    s.endGroup();
    return v;
}

void Storage::saveConfig(const QString &path, const QString &group, const QString &key, const QString &value) {
    QSettings s(path, QSettings::IniFormat);
    s.beginGroup(group);
    s.setValue(key, value);
    s.endGroup();
}

std::vector<Contact> Storage::loadContacts() {
    QFile f(s_baseDir + "/contacts.json");
    std::vector<Contact> out;
    if(!f.open(QIODevice::ReadOnly)) return out;
    auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    auto arr = doc.object()["contacts"].toArray();
    for(auto v : arr) {
        auto o = v.toObject();
        Contact c;
        c.username = o["user"].toString();
        c.publicKeyPem = o["pub"].toString();
        c.conversationKey = QByteArray::fromBase64(o["key"].toString().toLatin1());
        out.push_back(c);
    }
    return out;
}

void Storage::saveContacts(const std::vector<Contact> &contacts) {
    QJsonArray arr;
    for(auto &c : contacts) {
        QJsonObject o;
        o["user"] = c.username;
        o["pub"]  = c.publicKeyPem;
        o["key"]  = QString::fromLatin1(c.conversationKey.toBase64());
        arr.append(o);
    }
    QJsonObject root;
    root["contacts"] = arr;
    QJsonDocument doc(root);
    QFile f(s_baseDir + "/contacts.json");
    if(f.open(QIODevice::WriteOnly)) {
        f.write(doc.toJson());
        f.close();
    }
}

void Storage::appendHistory(const QString &user, const QByteArray &plain) {
    QFile f(s_baseDir + "/history/" + user + ".log");
    if(f.open(QIODevice::Append)) {
        f.write(plain + "\\n");
        f.close();
    }
}

QByteArray Storage::getConversationKey(const QString &user) {
    auto contacts = loadContacts();
    for(auto &c : contacts)
        if(c.username == user)
            return c.conversationKey;
    // not found: generate a random new key?
    auto key = Crypto::generateRandom(32);
    return key;
}

QString Storage::getContactPublicKey(const QString &user) {
    auto contacts = loadContacts();
    for(auto &c : contacts)
        if(c.username == user)
            return c.publicKeyPem;
    return {};
}
}