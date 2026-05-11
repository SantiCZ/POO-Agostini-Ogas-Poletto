#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    QString getUsername() const;

private slots:
    void onLogin();

private:
    QLineEdit *m_userEdit;
    QLineEdit *m_passEdit;
    QPushButton *m_loginBtn;
    QPushButton *m_cancelBtn;
    QLabel *m_errorLabel;
    QString m_username;
};