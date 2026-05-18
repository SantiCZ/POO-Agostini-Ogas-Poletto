#include "logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QRegularExpression>

// ─────────────────────────────────────────────────────────────────
// Helpers de estilo reutilizables (consistentes con el resto del proyecto)
// ─────────────────────────────────────────────────────────────────
static QString inputStyle() {
    return R"(
        QLineEdit {
            background: #21253A;
            color: #F1F5F9;
            border: 1px solid #2E3347;
            border-radius: 8px;
            padding: 9px 12px;
            font-size: 13px;
        }
        QLineEdit:focus {
            border: 1px solid #4ADE80;
        }
    )";
}

static QString primaryBtnStyle() {
    return R"(
        QPushButton {
            background: #4ADE80;
            color: #0F1117;
            border: none;
            border-radius: 8px;
            padding: 10px;
            font-size: 13px;
            font-weight: 700;
        }
        QPushButton:hover  { background: #22C55E; }
        QPushButton:pressed{ background: #16A34A; }
    )";
}

static QString ghostBtnStyle() {
    return R"(
        QPushButton {
            background: transparent;
            color: #64748B;
            border: 1px solid #2E3347;
            border-radius: 8px;
            padding: 9px;
            font-size: 13px;
        }
        QPushButton:hover { color: #94A3B8; border-color: #475569; }
    )";
}

static QString linkBtnStyle(const QString &color = "#4ADE80") {
    return QString(R"(
        QPushButton {
            background: transparent;
            color: %1;
            border: none;
            font-size: 12px;
            font-weight: 600;
            text-decoration: underline;
        }
        QPushButton:hover { color: white; }
    )").arg(color);
}

// ─────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────
LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("AlcancIA");
    setModal(true);
    setFixedSize(400, 480);
    setStyleSheet(R"(
        QDialog {
            background-color: #0F1117;
            border: 1px solid #2E3347;
            border-radius: 16px;
        }
        QLabel { background: transparent; }
    )");

    m_stack = new QStackedWidget(this);

    buildLoginPanel();
    buildRegisterPanel();

    m_stack->addWidget(m_loginWidget);     // índice 0
    m_stack->addWidget(m_registerWidget);  // índice 1

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(m_stack);

    showLoginPanel();
}

// ─────────────────────────────────────────────────────────────────
// Panel LOGIN
// ─────────────────────────────────────────────────────────────────
void LoginDialog::buildLoginPanel() {
    m_loginWidget = new QWidget();
    m_loginWidget->setStyleSheet("background: transparent;");

    QVBoxLayout *mainL = new QVBoxLayout(m_loginWidget);
    mainL->setContentsMargins(36, 36, 36, 36);
    mainL->setSpacing(14);

    // ── Logo ──────────────────────────────────────────────────────
    QLabel *logo = new QLabel("🏦");
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("font-size: 40px;");

    QLabel *appName = new QLabel("AlcancIA");
    appName->setAlignment(Qt::AlignCenter);
    appName->setStyleSheet("color: #F1F5F9; font-size: 24px; font-weight: 800; letter-spacing: -0.5px;");

    QLabel *tagline = new QLabel("Tu asistente financiero inteligente");
    tagline->setAlignment(Qt::AlignCenter);
    tagline->setStyleSheet("color: #4ADE80; font-size: 12px; font-weight: 500;");

    mainL->addWidget(logo);
    mainL->addWidget(appName);
    mainL->addWidget(tagline);
    mainL->addSpacing(8);

    // ── Separador ─────────────────────────────────────────────────
    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background: #2E3347; border: none; max-height: 1px;");
    mainL->addWidget(sep);
    mainL->addSpacing(4);

    // ── Campos ────────────────────────────────────────────────────
    QLabel *userLabel = new QLabel("Usuario");
    userLabel->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 600; letter-spacing: 0.5px;");

    m_userEdit = new QLineEdit();
    m_userEdit->setPlaceholderText("Ingresá tu usuario");
    m_userEdit->setStyleSheet(inputStyle());
    m_userEdit->setFixedHeight(42);

    QLabel *passLabel = new QLabel("Contraseña");
    passLabel->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 600; letter-spacing: 0.5px;");

    m_passEdit = new QLineEdit();
    m_passEdit->setPlaceholderText("Ingresá tu contraseña");
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setStyleSheet(inputStyle());
    m_passEdit->setFixedHeight(42);
    // Enter en contraseña dispara login
    connect(m_passEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLogin);

    // ── Error ─────────────────────────────────────────────────────
    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet("color: #F87171; font-size: 12px;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->hide();

    mainL->addWidget(userLabel);
    mainL->addWidget(m_userEdit);
    mainL->addWidget(passLabel);
    mainL->addWidget(m_passEdit);
    mainL->addWidget(m_errorLabel);
    mainL->addSpacing(4);

    // ── Botón ingresar ────────────────────────────────────────────
    m_loginBtn = new QPushButton("Ingresar");
    m_loginBtn->setFixedHeight(44);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(primaryBtnStyle());
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);

    // ── Botón cancelar ────────────────────────────────────────────
    m_cancelBtn = new QPushButton("Cancelar");
    m_cancelBtn->setFixedHeight(40);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(ghostBtnStyle());
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    mainL->addWidget(m_loginBtn);
    mainL->addWidget(m_cancelBtn);
    mainL->addStretch();

    // ── Link a registro ───────────────────────────────────────────
    QWidget *regRow = new QWidget();
    regRow->setStyleSheet("background: transparent;");
    QHBoxLayout *regL = new QHBoxLayout(regRow);
    regL->setContentsMargins(0, 0, 0, 0);
    regL->setSpacing(4);

    QLabel *regTxt = new QLabel("¿No tenés cuenta?");
    regTxt->setStyleSheet("color: #475569; font-size: 12px;");

    QPushButton *regLink = new QPushButton("Crear una ahora");
    regLink->setCursor(Qt::PointingHandCursor);
    regLink->setStyleSheet(linkBtnStyle("#4ADE80"));
    connect(regLink, &QPushButton::clicked, this, &LoginDialog::showRegisterPanel);

    regL->addStretch();
    regL->addWidget(regTxt);
    regL->addWidget(regLink);
    regL->addStretch();

    mainL->addWidget(regRow);
}

// ─────────────────────────────────────────────────────────────────
// Panel REGISTRO
// ─────────────────────────────────────────────────────────────────
void LoginDialog::buildRegisterPanel() {
    m_registerWidget = new QWidget();
    m_registerWidget->setStyleSheet("background: transparent;");

    QVBoxLayout *mainL = new QVBoxLayout(m_registerWidget);
    mainL->setContentsMargins(36, 32, 36, 32);
    mainL->setSpacing(12);

    // ── Encabezado ────────────────────────────────────────────────
    QWidget *headerW = new QWidget();
    headerW->setStyleSheet("background: transparent;");
    QHBoxLayout *headerL = new QHBoxLayout(headerW);
    headerL->setContentsMargins(0, 0, 0, 0);

    m_backBtn = new QPushButton("← Volver");
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setStyleSheet(linkBtnStyle("#64748B"));
    connect(m_backBtn, &QPushButton::clicked, this, &LoginDialog::showLoginPanel);

    QLabel *regTitle = new QLabel("Crear cuenta");
    regTitle->setStyleSheet("color: #F1F5F9; font-size: 20px; font-weight: 700;");

    QLabel *regSub = new QLabel("Completá tus datos para registrarte");
    regSub->setStyleSheet("color: #64748B; font-size: 12px;");

    headerL->addWidget(m_backBtn);
    headerL->addStretch();

    mainL->addWidget(headerL->parentWidget());  // el headerW
    mainL->addWidget(regTitle);
    mainL->addWidget(regSub);
    mainL->addSpacing(4);

    // ── Campos ────────────────────────────────────────────────────
    auto makeLabel = [](const QString &txt) {
        QLabel *l = new QLabel(txt);
        l->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 600; letter-spacing: 0.5px;");
        return l;
    };

    m_regNameEdit = new QLineEdit();
    m_regNameEdit->setPlaceholderText("ej: juanperez");
    m_regNameEdit->setFixedHeight(42);
    m_regNameEdit->setStyleSheet(inputStyle());

    m_regEmailEdit = new QLineEdit();
    m_regEmailEdit->setPlaceholderText("ej: juan@email.com");
    m_regEmailEdit->setFixedHeight(42);
    m_regEmailEdit->setStyleSheet(inputStyle());

    m_regPassEdit = new QLineEdit();
    m_regPassEdit->setPlaceholderText("Mínimo 6 caracteres");
    m_regPassEdit->setEchoMode(QLineEdit::Password);
    m_regPassEdit->setFixedHeight(42);
    m_regPassEdit->setStyleSheet(inputStyle());

    m_regPass2Edit = new QLineEdit();
    m_regPass2Edit->setPlaceholderText("Repetí la contraseña");
    m_regPass2Edit->setEchoMode(QLineEdit::Password);
    m_regPass2Edit->setFixedHeight(42);
    m_regPass2Edit->setStyleSheet(inputStyle());
    connect(m_regPass2Edit, &QLineEdit::returnPressed, this, &LoginDialog::onRegister);

    mainL->addWidget(makeLabel("Usuario"));
    mainL->addWidget(m_regNameEdit);
    mainL->addWidget(makeLabel("Email"));
    mainL->addWidget(m_regEmailEdit);
    mainL->addWidget(makeLabel("Contraseña"));
    mainL->addWidget(m_regPassEdit);
    mainL->addWidget(makeLabel("Repetir contraseña"));
    mainL->addWidget(m_regPass2Edit);

    // ── Error ─────────────────────────────────────────────────────
    m_regErrorLabel = new QLabel();
    m_regErrorLabel->setStyleSheet("color: #F87171; font-size: 12px;");
    m_regErrorLabel->setAlignment(Qt::AlignCenter);
    m_regErrorLabel->setWordWrap(true);
    m_regErrorLabel->hide();
    mainL->addWidget(m_regErrorLabel);

    // ── Botón registrar ───────────────────────────────────────────
    m_registerBtn = new QPushButton("Crear cuenta");
    m_registerBtn->setFixedHeight(44);
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    m_registerBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #4ADE80, stop:1 #22C55E);
            color: #0F1117;
            border: none;
            border-radius: 8px;
            font-size: 13px;
            font-weight: 700;
        }
        QPushButton:hover  { background: #22C55E; }
        QPushButton:pressed{ background: #16A34A; }
    )");
    connect(m_registerBtn, &QPushButton::clicked, this, &LoginDialog::onRegister);
    mainL->addWidget(m_registerBtn);

    // ── Link a login ──────────────────────────────────────────────
    QWidget *loginRow = new QWidget();
    loginRow->setStyleSheet("background: transparent;");
    QHBoxLayout *loginRowL = new QHBoxLayout(loginRow);
    loginRowL->setContentsMargins(0, 0, 0, 0);
    loginRowL->setSpacing(4);

    QLabel *loginTxt = new QLabel("¿Ya tenés cuenta?");
    loginTxt->setStyleSheet("color: #475569; font-size: 12px;");

    QPushButton *loginLink = new QPushButton("Iniciá sesión");
    loginLink->setCursor(Qt::PointingHandCursor);
    loginLink->setStyleSheet(linkBtnStyle("#4ADE80"));
    connect(loginLink, &QPushButton::clicked, this, &LoginDialog::showLoginPanel);

    loginRowL->addStretch();
    loginRowL->addWidget(loginTxt);
    loginRowL->addWidget(loginLink);
    loginRowL->addStretch();

    mainL->addWidget(loginRow);
}

// ─────────────────────────────────────────────────────────────────
// Slots de navegación
// ─────────────────────────────────────────────────────────────────
void LoginDialog::showLoginPanel() {
    m_errorLabel->hide();
    m_stack->setCurrentIndex(0);
    setFixedHeight(480);
}

void LoginDialog::showRegisterPanel() {
    m_regErrorLabel->hide();
    m_regNameEdit->clear();
    m_regEmailEdit->clear();
    m_regPassEdit->clear();
    m_regPass2Edit->clear();
    m_stack->setCurrentIndex(1);
    setFixedHeight(580);
}

// ─────────────────────────────────────────────────────────────────
// Lógica de LOGIN
// ─────────────────────────────────────────────────────────────────
void LoginDialog::onLogin() {
    QString u = m_userEdit->text().trimmed();
    QString p = m_passEdit->text();

    if (u.isEmpty() || p.isEmpty()) {
        m_errorLabel->setText("Completá usuario y contraseña.");
        m_errorLabel->show();
        return;
    }

    // Credenciales demo — reemplazar por consulta a DatabaseManager
    if (u == "alcancia" && p == "1234") {
        m_username   = u;
        m_registered = false;
        accept();
    } else {
        m_errorLabel->setText("Usuario o contraseña incorrectos.");
        m_errorLabel->show();
        m_passEdit->clear();
        m_passEdit->setFocus();
    }
}

// ─────────────────────────────────────────────────────────────────
// Lógica de REGISTRO
// ─────────────────────────────────────────────────────────────────
void LoginDialog::onRegister() {
    QString nombre = m_regNameEdit->text().trimmed();
    QString email  = m_regEmailEdit->text().trimmed();
    QString pass1  = m_regPassEdit->text();
    QString pass2  = m_regPass2Edit->text();

    // ── Validaciones ──────────────────────────────────────────────
    if (nombre.isEmpty() || email.isEmpty() || pass1.isEmpty() || pass2.isEmpty()) {
        m_regErrorLabel->setText("Completá todos los campos.");
        m_regErrorLabel->show();
        return;
    }
    if (nombre.length() < 3) {
        m_regErrorLabel->setText("El usuario debe tener al menos 3 caracteres.");
        m_regErrorLabel->show();
        return;
    }
    // Validación simple de email
    QRegularExpression emailRx(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)");
    if (!emailRx.match(email).hasMatch()) {
        m_regErrorLabel->setText("El email no tiene un formato válido.");
        m_regErrorLabel->show();
        return;
    }
    if (pass1.length() < 6) {
        m_regErrorLabel->setText("La contraseña debe tener al menos 6 caracteres.");
        m_regErrorLabel->show();
        return;
    }
    if (pass1 != pass2) {
        m_regErrorLabel->setText("Las contraseñas no coinciden.");
        m_regErrorLabel->show();
        m_regPass2Edit->clear();
        m_regPass2Edit->setFocus();
        return;
    }

    // ── Guardar y cerrar ──────────────────────────────────────────
    // Aquí irá la llamada a DatabaseManager::instance().createUser(...)
    m_newUser    = { nombre, email, pass1 };
    m_username   = nombre;
    m_registered = true;
    accept();
}

// ─────────────────────────────────────────────────────────────────
// Getters
// ─────────────────────────────────────────────────────────────────
QString  LoginDialog::getUsername()       const { return m_username; }
UserData LoginDialog::getRegisteredUser() const { return m_newUser; }
bool     LoginDialog::wasRegistered()     const { return m_registered; }