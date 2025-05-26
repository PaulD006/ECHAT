#pragma once
#include <QMainWindow>
#include "NetworkClient.h"

class QListWidget;
class QTextEdit;
class QLineEdit;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(NetworkClient *client, QWidget *parent = nullptr);

private slots:
    void onSendClicked();
    void onIncoming(const QString &from, const QByteArray &data);

private:
    NetworkClient *m_client;
    QListWidget *m_contactList;
    QTextEdit    *m_chatView;
    QLineEdit    *m_messageEdit;
    QPushButton  *m_sendBtn;
};