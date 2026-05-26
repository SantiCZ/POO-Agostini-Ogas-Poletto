#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    QString getUsername() const;
    bool wasRegistered() const;

private slots:
    void onLogin();
    void onRegister();
    void showLoginPanel();
    void showRegisterPanel();
    // nuevo: respuesta del vps al login real
    void onLoginExitoso(int idUsuario, const QString& nombre);
    // respuesta del vps al registro
    void onRegistroRespuesta(bool exito, const QString& mensaje);

private:
    void buildLoginPanel();
    void buildRegisterPanel();

    QStackedWidget *m_stack = nullptr;

    // LOGIN
    QWidget     *m_loginWidget  = nullptr;
    QLineEdit   *m_userEdit     = nullptr;  // ahora es el email
    QLineEdit   *m_passEdit     = nullptr;
    QPushButton *m_loginBtn     = nullptr;
    QPushButton *m_cancelBtn    = nullptr;
    QLabel      *m_errorLabel   = nullptr;

    // REGISTRO
    QWidget     *m_registerWidget  = nullptr;
    QLineEdit   *m_regUsernameEdit = nullptr;
    QLineEdit   *m_regEmailEdit    = nullptr;
    QLineEdit   *m_regPassEdit     = nullptr;
    QLineEdit   *m_regPass2Edit    = nullptr;
    QPushButton *m_registerBtn     = nullptr;
    QPushButton *m_backBtn         = nullptr;
    QLabel      *m_regErrorLabel   = nullptr;

    QString m_username;
    bool    m_registered = false;
};