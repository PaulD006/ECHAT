#pragma once
#include <QDialog>
#include "NetworkClient.h"

class QLineEdit;
class QPushButton;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    NetworkClient* client() const { return m_client; }

private slots:
    void onLoginClicked();

private:
    QLineEdit *m_serverEdit, *m_userEdit, *m_passEdit;
    QPushButton *m_loginBtn;
    NetworkClient *m_client = nullptr;
};