#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>

// ── Estructura para pasar datos del usuario registrado ─────────────
struct UserData {
    QString username;
    QString email;
    QString password;
};

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString  getUsername() const;
    UserData getRegisteredUser() const;   // válido solo si vino del registro
    bool     wasRegistered() const;       // true si el usuario se acaba de crear

private slots:
    void onLogin();
    void onRegister();
    void showLoginPanel();
    void showRegisterPanel();

private:
    void buildLoginPanel();
    void buildRegisterPanel();
    void setInputStyle(QLineEdit *edit);

    // ── Stack ──────────────────────────────────────────────────────
    QStackedWidget *m_stack         = nullptr;

    // ── Panel Login ────────────────────────────────────────────────
    QWidget     *m_loginWidget      = nullptr;
    QLineEdit   *m_userEdit         = nullptr;
    QLineEdit   *m_passEdit         = nullptr;
    QPushButton *m_loginBtn         = nullptr;
    QPushButton *m_cancelBtn        = nullptr;
    QLabel      *m_errorLabel       = nullptr;

    // ── Panel Registro ─────────────────────────────────────────────
    QWidget     *m_registerWidget   = nullptr;
    QLineEdit   *m_regNameEdit      = nullptr;
    QLineEdit   *m_regEmailEdit     = nullptr;
    QLineEdit   *m_regPassEdit      = nullptr;
    QLineEdit   *m_regPass2Edit     = nullptr;
    QPushButton *m_registerBtn      = nullptr;
    QPushButton *m_backBtn          = nullptr;
    QLabel      *m_regErrorLabel    = nullptr;

    // ── Estado ────────────────────────────────────────────────────
    QString  m_username;
    UserData m_newUser;
    bool     m_registered = false;
};