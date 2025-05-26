#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include "Storage.h"

LoginDialog::LoginDialog(QWidget *parent)
 : QDialog(parent)
{
    setWindowTitle("EChat Login");
    auto *layout = new QVBoxLayout(this);

    auto *sLayout = new QHBoxLayout;
    sLayout->addWidget(new QLabel("Server:"));
    m_serverEdit = new QLineEdit;
    m_serverEdit->setText(Storage::loadConfig("config_echat/echatclient.config", "Network", "servers"));
    sLayout->addWidget(m_serverEdit);
    layout->addLayout(sLayout);

    auto *uLayout = new QHBoxLayout;
    uLayout->addWidget(new QLabel("Username:"));
    m_userEdit = new QLineEdit;
    layout->addLayout(uLayout);
    uLayout->addWidget(m_userEdit);

    auto *pLayout = new QHBoxLayout;
    pLayout->addWidget(new QLabel("Password:"));
    m_passEdit = new QLineEdit;
    m_passEdit->setEchoMode(QLineEdit::Password);
    pLayout->addWidget(m_passEdit);
    layout->addLayout(pLayout);

    m_loginBtn = new QPushButton("Login");
    layout->addWidget(m_loginBtn);
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
}

void LoginDialog::onLoginClicked() {
    QString server = m_serverEdit->text().split(',').first().trimmed();
    QString user   = m_userEdit->text();
    QString pass   = m_passEdit->text();

    m_client = new NetworkClient(this);
    if (!m_client->connectToServer(server)) {
        QMessageBox::critical(this, "Connection Error", "Failed to connect to server.");
        delete m_client; m_client = nullptr;
        return;
    }
    if (!m_client->login(user, pass)) {
        QMessageBox::critical(this, "Login Failed", "Invalid credentials.");
        m_client->disconnectFromServer();
        delete m_client; m_client = nullptr;
        return;
    }
    // Save last username
    Storage::saveConfig("config_echat/echatclient.config", "User", "last_username", user);
    accept();
}