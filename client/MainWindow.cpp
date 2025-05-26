#include "MainWindow.h"
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "Crypto.h"
#include "Storage.h"

MainWindow::MainWindow(NetworkClient *client, QWidget *parent)
 : QMainWindow(parent), m_client(client)
{
    setWindowTitle("EChat");
    resize(800,600);

    auto *central = new QWidget;
    auto *layout = new QHBoxLayout(central);

    m_contactList = new QListWidget;
    layout->addWidget(m_contactList);

    auto *chatLayout = new QVBoxLayout;
    m_chatView   = new QTextEdit; m_chatView->setReadOnly(true);
    chatLayout->addWidget(m_chatView);

    auto *msgLayout = new QHBoxLayout;
    m_messageEdit = new QLineEdit;
    m_sendBtn     = new QPushButton("Send");
    msgLayout->addWidget(m_messageEdit);
    msgLayout->addWidget(m_sendBtn);
    chatLayout->addLayout(msgLayout);

    layout->addLayout(chatLayout);
    setCentralWidget(central);

    // Load contacts
    auto contacts = Storage::loadContacts();
    for(auto &c : contacts) {
        m_contactList->addItem(c.username);
    }

    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    m_client->setMessageHandler([this](const QString &from, const QByteArray &data){
        onIncoming(from, data);
    });
}

void MainWindow::onSendClicked() {
    auto item = m_contactList->currentItem();
    if(!item) return;
    QString to = item->text();
    QByteArray plaintext = m_messageEdit->text().toUtf8();

    // Double-encrypt
    auto iv = Crypto::generateRandom(12);
    auto aesKey = Storage::getConversationKey(to);
    auto inner = Crypto::aesEncrypt(aesKey, iv, plaintext);
    auto pubPem = Storage::getContactPublicKey(to);
    auto outer = Crypto::encryptRSA(pubPem, iv + inner);

    m_client->sendMessage(to, outer);
    m_chatView->append(QString("Me: %1").arg(QString::fromUtf8(plaintext)));
    Storage::appendHistory(to, plaintext);
    m_messageEdit->clear();
}

void MainWindow::onIncoming(const QString &from, const QByteArray &data) {
    // Decrypt outer
    auto privPem = Crypto::getSessionPrivateKey();
    auto combined = Crypto::decryptRSA(privPem, data);
    auto iv = combined.left(12);
    auto inner = combined.mid(12);
    auto aesKey = Storage::getConversationKey(from);
    auto plain = Crypto::aesDecrypt(aesKey, iv, inner);

    QString txt = QString::fromUtf8(plain);
    m_chatView->append(QString("%1: %2").arg(from, txt));
    Storage::appendHistory(from, plain);
}