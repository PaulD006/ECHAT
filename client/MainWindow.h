#pragma once
#include <QMainWindow>
#include <QListWidget>
#include "NetworkClient.h"

class QListWidget;
class QTextEdit;
class QLineEdit;
class QPushButton;
class NetworkClient;

struct ContactInfo { QString username; /*…*/ };

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(NetworkClient *client, QWidget *parent=nullptr);

private slots:
    void onSendClicked();
    void onIncoming(const QString&, const QByteArray&);
    void onNewContact();
    void onImportContact();

private:
    NetworkClient  *m_client;
    QListWidget    *m_contactList;
    QTextEdit      *m_chatView;
    QLineEdit      *m_messageEdit;
    QPushButton    *m_sendBtn;
};