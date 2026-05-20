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
    void onRegistroRespuesta(bool exito, const QString &mensaje); // <--- NUEVO: Slot para la respuesta de red

private:
    void buildLoginPanel();
    void buildRegisterPanel();

    QStackedWidget *m_stack = nullptr;

    QWidget     *m_loginWidget    = nullptr;
    QLineEdit   *m_userEdit       = nullptr;
    QLineEdit   *m_passEdit       = nullptr;
    QPushButton *m_loginBtn       = nullptr;
    QPushButton *m_cancelBtn      = nullptr;
    QLabel      *m_errorLabel     = nullptr;

    QWidget     *m_registerWidget = nullptr;
    QLineEdit   *m_regUsernameEdit= nullptr;   // ¡importante, no cambiar!
    QLineEdit   *m_regEmailEdit   = nullptr;
    QLineEdit   *m_regPassEdit    = nullptr;
    QLineEdit   *m_regPass2Edit   = nullptr;
    QPushButton *m_registerBtn    = nullptr;
    QPushButton *m_backBtn        = nullptr;
    QLabel      *m_regErrorLabel  = nullptr;

    QString  m_username;
    bool     m_registered = false;
};