#include "Storage.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QSettings>

namespace Storage {

// Helper: base configuration directory
static QString configDir() {
    // or use QStandardPaths::writableLocation(...)
    return QDir::homePath() + "/config_echat";
}

// Initialize (if needed)
void init(const QString &baseDir) {
    Q_UNUSED(baseDir);
    QDir dir(configDir());
    dir.mkpath(".");
}

// Simple INI-style config loader/saver
QString loadConfig(const QString &path,
                   const QString &group,
                   const QString &key)
{
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(group);
    QString value = settings.value(key).toString();
    settings.endGroup();
    return value;
}

void saveConfig(const QString &path,
                const QString &group,
                const QString &key,
                const QString &value)
{
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(group);
    settings.setValue(key, value);
    settings.endGroup();
}

// Contact list persistence
std::vector<Contact> loadContacts() {
    QString fn = configDir() + "/contacts.json";
    QFile f(fn);
    std::vector<Contact> out;
    if(!f.open(QIODevice::ReadOnly)) return out;
    auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    auto arr = doc.array();
    for(auto v : arr) {
        auto obj = v.toObject();
        Contact c;
        c.username = obj["user"].toString();
        out.push_back(c);
    }
    return out;
}

void saveContacts(const std::vector<Contact> &contacts) {
    QString fn = configDir() + "/contacts.json";
    QJsonArray arr;
    for(auto &c : contacts) {
        QJsonObject o;
        o["user"] = c.username;
        arr.append(o);
    }
    QJsonDocument doc(arr);
    QFile f(fn);
    if(f.open(QIODevice::WriteOnly)) {
        f.write(doc.toJson());
        f.close();
    }
}

// Chat history at rest (encrypted on client side)
void appendHistory(const QString &user, const QByteArray &plain) {
    QString dir = configDir() + "/history";
    QDir().mkpath(dir);
    QString fn = dir + "/" + user + ".log";
    QFile f(fn);
    if(f.open(QIODevice::Append)) {
        f.write(plain);
        f.write("\n");
        f.close();
    }
}

// AES key storage
QByteArray getConversationKey(const QString &user) {
    QString fn = configDir() + "/keys/" + user + ".key";
    QFile f(fn);
    if(!f.open(QIODevice::ReadOnly))
        return {};
    auto data = f.readAll();
    f.close();
    return data;
}

void saveConversationKey(const QString &peerUser, const QByteArray &key) {
    QString dir = configDir() + "/keys";
    QDir().mkpath(dir);
    QString fn = dir + "/" + peerUser + ".key";
    QFile f(fn);
    if(f.open(QIODevice::WriteOnly)) {
        f.write(key);
        f.close();
    }
}

// Contact exchange
QString exportContact(const QString& peerUser) {
    QString dir = configDir() + "/contacts";
    QDir().mkpath(dir);
    QString path = dir + "/" + peerUser + ".contact";

    QByteArray aesKey = getConversationKey(peerUser);

    QJsonObject obj;
    obj["user"] = peerUser;
    obj["key"]  = QString(aesKey.toHex());
    QFile f(path);
    if(f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(obj).toJson());
        f.close();
    }
    return path;
}

QString importContact(const QString& filename) {
    QFile f(filename);
    if(!f.open(QIODevice::ReadOnly))
        return {};
    auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    auto obj  = doc.object();
    QString peer = obj["user"].toString();
    QByteArray key = QByteArray::fromHex(obj["key"].toString().toUtf8());
    saveConversationKey(peer, key);
    return peer;
}

QString getContactPublicKey(const QString &user) {
    QString fn = configDir() + "/contacts/" + user + ".pub.pem";
    return fn;
}

} // namespace Storage