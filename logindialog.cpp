#include "logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Iniciar sesión - AlcancIA");
    setModal(true);
    setFixedSize(380, 260);
    setStyleSheet("QDialog { background-color: #0F1117; border-radius: 16px; }");

    QVBoxLayout *mainL = new QVBoxLayout(this);
    mainL->setContentsMargins(30, 30, 30, 30);
    mainL->setSpacing(16);

    QLabel *title = new QLabel("🏦  AlcancIA");
    title->setStyleSheet("color: #F1F5F9; font-size: 22px; font-weight: 700;");
    title->setAlignment(Qt::AlignCenter);

    m_userEdit = new QLineEdit();
    m_userEdit->setPlaceholderText("Usuario");
    m_userEdit->setStyleSheet("background:#21253A; color:#F1F5F9; border:1px solid #2E3347; border-radius:8px; padding:8px 12px;");

    m_passEdit = new QLineEdit();
    m_passEdit->setPlaceholderText("Contraseña");
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setStyleSheet("background:#21253A; color:#F1F5F9; border:1px solid #2E3347; border-radius:8px; padding:8px 12px;");

    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet("color: #F87171; font-size: 12px;");
    m_errorLabel->hide();

    QHBoxLayout *btnL = new QHBoxLayout();
    m_cancelBtn = new QPushButton("Cancelar");
    m_cancelBtn->setStyleSheet("background:transparent; color:#64748B; border:1px solid #2E3347; border-radius:6px; padding:6px;");
    m_loginBtn = new QPushButton("Ingresar");
    m_loginBtn->setStyleSheet("background:#4ADE80; color:#0F1117; border:none; border-radius:6px; padding:6px; font-weight:700;");
    btnL->addWidget(m_cancelBtn);
    btnL->addWidget(m_loginBtn);

    mainL->addWidget(title);
    mainL->addWidget(m_userEdit);
    mainL->addWidget(m_passEdit);
    mainL->addWidget(m_errorLabel);
    mainL->addLayout(btnL);

    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void LoginDialog::onLogin() {
    QString u = m_userEdit->text().trimmed();
    QString p = m_passEdit->text();
    // Credenciales fijas para demo (cambialas si querés)
    if (u == "alcancia" && p == "1234") {
        m_username = u;
        accept();
    } else {
        m_errorLabel->setText("Usuario o contraseña incorrectos");
        m_errorLabel->show();
    }
}

QString LoginDialog::getUsername() const { return m_username; }