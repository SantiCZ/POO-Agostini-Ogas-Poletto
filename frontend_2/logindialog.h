#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QCheckBox>

/*
 * LoginDialog
 * Responsabilidad de clase:
 * Pantalla modal de autenticacion y registro.
 * Maneja dos paneles internos: inicio de sesion y alta de usuario.
 *
 * Herencia:
 * Hereda de QDialog para ejecutarse como ventana modal antes de MainWidget.
 *
 * SLOT:
 * Sus slots reciben clicks de botones y respuestas asincronicas de DataManager.
 */
class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString getUsername() const;
    bool wasRegistered() const;

private slots:
    // Validan formularios y delegan el proceso en DataManager.
    void onLogin();
    void onRegister();

    // Cambian el panel visible dentro del QStackedWidget.
    void showLoginPanel();
    void showRegisterPanel();

    // Reciben respuestas asincronicas del servidor.
    void onLoginExitoso(int idUsuario, const QString& nombre);
    void onRegistroRespuesta(bool exito, const QString& mensaje);

private:
    void buildLoginPanel();
    void buildRegisterPanel();

    // Carga el ultimo usuario local para mejorar la experiencia de login.
    void cargarUltimoUsuario();   // <--- nuevo

    QStackedWidget *m_stack = nullptr;

    // Widgets del panel de login.
    QWidget     *m_loginWidget  = nullptr;
    QLineEdit   *m_userEdit     = nullptr;  // email
    QLineEdit   *m_passEdit     = nullptr;
    QPushButton *m_loginBtn     = nullptr;
    QPushButton *m_cancelBtn    = nullptr;
    QLabel      *m_errorLabel   = nullptr;
    QCheckBox   *m_recordarmeCheck = nullptr;   // <--- nuevo

    // Widgets del panel de registro.
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
