#include "logindialog.h"
#include "datamanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QRegularExpression>
#include <QMessageBox>
#include <QCheckBox>
#include <QSettings>

// ─────────────────────────────────────────────────────────────────
// Helpers de estilo
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
        QPushButton:hover {
            color: #94A3B8;
            border-color: #475569;
        }
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
LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("AlcancIA");
    setModal(true);
    setFixedSize(400, 520);
    setStyleSheet(R"(
        QDialog { background-color: #0F1117; border: 1px solid #2E3347; border-radius: 16px; }
        QLabel  { background: transparent; }
    )");

    m_stack = new QStackedWidget(this);
    buildLoginPanel();
    buildRegisterPanel();
    m_stack->addWidget(m_loginWidget);
    m_stack->addWidget(m_registerWidget);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(m_stack);

    showLoginPanel();

    // Cargar último email recordado (usando QSettings)
    cargarUltimoUsuario();

    // nuevo: conectamos las señales del login real contra el vps
    connect(&DataManager::instance(),
            &DataManager::loginExitoso,
            this,
            &LoginDialog::onLoginExitoso);

    connect(&DataManager::instance(),
            &DataManager::loginFallido,
            this,
            [this](const QString& mensaje) {
                m_loginBtn->setEnabled(true);
                m_loginBtn->setText("Ingresar");
                m_errorLabel->setText(
                    mensaje.isEmpty()
                        ? "Usuario o contraseña incorrectos."
                        : mensaje
                    );
                m_errorLabel->show();
                m_passEdit->clear();
                m_passEdit->setFocus();
            });

    connect(&DataManager::instance(),
            &DataManager::usuarioRegistradoServidor,
            this,
            &LoginDialog::onRegistroRespuesta);

    connect(&DataManager::instance(),
            &DataManager::errorDeRed,
            this,
            [this](const QString& mensaje) {
                if (m_stack->currentIndex() == 1) {
                    m_registerBtn->setEnabled(true);
                    m_registerBtn->setText("Crear cuenta");
                    m_regErrorLabel->setText(mensaje);
                    m_regErrorLabel->show();
                }
                else {
                    m_loginBtn->setEnabled(true);
                    m_loginBtn->setText("Ingresar");
                    m_errorLabel->setText("Error de conexion: " + mensaje);
                    m_errorLabel->show();
                }
            });
}

// ─────────────────────────────────────────────────────────────────
// LOGIN PANEL
// ─────────────────────────────────────────────────────────────────
void LoginDialog::buildLoginPanel()
{
    m_loginWidget = new QWidget();
    m_loginWidget->setStyleSheet("background: transparent;");

    QVBoxLayout *mainL = new QVBoxLayout(m_loginWidget);
    mainL->setContentsMargins(36, 30, 36, 24);
    mainL->setSpacing(14);

    QLabel *logo = new QLabel("🏦");
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("font-size: 40px;");

    QLabel *appName = new QLabel("AlcancIA");
    appName->setAlignment(Qt::AlignCenter);
    appName->setStyleSheet("color: #F1F5F9; font-size: 24px; font-weight: 800;");

    QLabel *tagline = new QLabel("Tu asistente financiero inteligente");
    tagline->setAlignment(Qt::AlignCenter);
    tagline->setStyleSheet("color: #4ADE80; font-size: 12px; font-weight: 500;");

    mainL->addWidget(logo);
    mainL->addWidget(appName);
    mainL->addWidget(tagline);
    mainL->addSpacing(8);

    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background: #2E3347; border: none; max-height: 1px;");
    mainL->addWidget(sep);
    mainL->addSpacing(4);

    // Email
    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 600;");

    m_userEdit = new QLineEdit();
    m_userEdit->setPlaceholderText("tu@email.com");
    m_userEdit->setStyleSheet(inputStyle());
    m_userEdit->setFixedHeight(42);

    QLabel *passLabel = new QLabel("Contraseña");
    passLabel->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 600;");

    m_passEdit = new QLineEdit();
    m_passEdit->setPlaceholderText("Ingresa tu contraseña");
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setStyleSheet(inputStyle());
    m_passEdit->setFixedHeight(42);
    connect(m_passEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLogin);

    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet("color: #F87171; font-size: 12px;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->hide();

    mainL->addWidget(emailLabel);
    mainL->addWidget(m_userEdit);
    mainL->addWidget(passLabel);
    mainL->addWidget(m_passEdit);
    mainL->addWidget(m_errorLabel);

    // Checkbox "Recordarme"
    m_recordarmeCheck = new QCheckBox("Recordarme");
    m_recordarmeCheck->setStyleSheet("color: #64748B; font-size: 12px;");
    mainL->addSpacing(6);
    mainL->addWidget(m_recordarmeCheck);

    // ─────────────────────────────────────────────────────────────
    // BOTONES HORIZONTALES MAS DELGADOS
    // ─────────────────────────────────────────────────────────────
    QWidget *buttonContainer = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);

    m_loginBtn = new QPushButton("Ingresar");
    m_loginBtn->setFixedHeight(44);
    m_loginBtn->setFixedWidth(130);      // más delgado
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(primaryBtnStyle());
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);

    m_cancelBtn = new QPushButton("Cancelar");
    m_cancelBtn->setFixedHeight(40);
    m_cancelBtn->setFixedWidth(130);     // más delgado
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(ghostBtnStyle());
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_loginBtn);
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addStretch();

    mainL->addWidget(buttonContainer);
    // Eliminado el addStretch() que seguía a los botones

    QWidget *regRow = new QWidget();
    regRow->setStyleSheet("background: transparent;");
    QHBoxLayout *regL = new QHBoxLayout(regRow);
    regL->setContentsMargins(0,0,0,0);

    QLabel *regTxt = new QLabel("¿No tenés cuenta?");
    regTxt->setStyleSheet("color: #475569; font-size: 12px;");

    QPushButton *regLink = new QPushButton("Crear una ahora");
    regLink->setCursor(Qt::PointingHandCursor);
    regLink->setStyleSheet(linkBtnStyle());
    connect(regLink, &QPushButton::clicked, this, &LoginDialog::showRegisterPanel);

    regL->addStretch();
    regL->addWidget(regTxt);
    regL->addWidget(regLink);
    regL->addStretch();
    mainL->addWidget(regRow);
}

// ─────────────────────────────────────────────────────────────────
// REGISTER PANEL
// ─────────────────────────────────────────────────────────────────
void LoginDialog::buildRegisterPanel()
{
    m_registerWidget = new QWidget();
    m_registerWidget->setStyleSheet("background: transparent;");

    QVBoxLayout *mainL = new QVBoxLayout(m_registerWidget);
    mainL->setContentsMargins(36, 32, 36, 32);
    mainL->setSpacing(12);

    QWidget *headerW = new QWidget();
    headerW->setStyleSheet("background: transparent;");
    QHBoxLayout *headerL = new QHBoxLayout(headerW);
    headerL->setContentsMargins(0,0,0,0);

    m_backBtn = new QPushButton("← Volver");
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setStyleSheet(linkBtnStyle("#64748B"));
    connect(m_backBtn, &QPushButton::clicked, this, &LoginDialog::showLoginPanel);
    headerL->addWidget(m_backBtn);
    headerL->addStretch();

    QLabel *title = new QLabel("Crear cuenta");
    title->setStyleSheet("color: #F1F5F9; font-size: 20px; font-weight: 700;");

    QLabel *sub = new QLabel("Completá tus datos para registrarte");
    sub->setStyleSheet("color: #64748B; font-size: 12px;");

    mainL->addWidget(headerW);
    mainL->addWidget(title);
    mainL->addWidget(sub);

    auto makeLabel = [](const QString &text) {
        QLabel *l = new QLabel(text);
        l->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 600;");
        return l;
    };

    m_regUsernameEdit = new QLineEdit();
    m_regUsernameEdit->setPlaceholderText("ej: Juan Perez");
    m_regUsernameEdit->setFixedHeight(42);
    m_regUsernameEdit->setStyleSheet(inputStyle());

    m_regEmailEdit = new QLineEdit();
    m_regEmailEdit->setPlaceholderText("ej: juan@email.com");
    m_regEmailEdit->setFixedHeight(42);
    m_regEmailEdit->setStyleSheet(inputStyle());

    m_regPassEdit = new QLineEdit();
    m_regPassEdit->setPlaceholderText("Minimo 6 caracteres");
    m_regPassEdit->setEchoMode(QLineEdit::Password);
    m_regPassEdit->setFixedHeight(42);
    m_regPassEdit->setStyleSheet(inputStyle());

    m_regPass2Edit = new QLineEdit();
    m_regPass2Edit->setPlaceholderText("Repeti la contraseña");
    m_regPass2Edit->setEchoMode(QLineEdit::Password);
    m_regPass2Edit->setFixedHeight(42);
    m_regPass2Edit->setStyleSheet(inputStyle());
    connect(m_regPass2Edit, &QLineEdit::returnPressed, this, &LoginDialog::onRegister);

    mainL->addWidget(makeLabel("Nombre"));
    mainL->addWidget(m_regUsernameEdit);
    mainL->addWidget(makeLabel("Email"));
    mainL->addWidget(m_regEmailEdit);
    mainL->addWidget(makeLabel("Contraseña"));
    mainL->addWidget(m_regPassEdit);
    mainL->addWidget(makeLabel("Repetir contraseña"));
    mainL->addWidget(m_regPass2Edit);

    m_regErrorLabel = new QLabel();
    m_regErrorLabel->setStyleSheet("color: #F87171; font-size: 12px;");
    m_regErrorLabel->setAlignment(Qt::AlignCenter);
    m_regErrorLabel->setWordWrap(true);
    m_regErrorLabel->hide();
    mainL->addWidget(m_regErrorLabel);

    m_registerBtn = new QPushButton("Crear cuenta");
    m_registerBtn->setFixedHeight(44);
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    m_registerBtn->setStyleSheet(primaryBtnStyle());
    connect(m_registerBtn, &QPushButton::clicked, this, &LoginDialog::onRegister);
    mainL->addWidget(m_registerBtn);

    QWidget *loginRow = new QWidget();
    loginRow->setStyleSheet("background: transparent;");
    QHBoxLayout *loginRowL = new QHBoxLayout(loginRow);
    loginRowL->setContentsMargins(0,0,0,0);

    QLabel *loginTxt = new QLabel("¿Ya tenés cuenta?");
    loginTxt->setStyleSheet("color: #475569; font-size: 12px;");

    QPushButton *loginLink = new QPushButton("Iniciá sesión");
    loginLink->setCursor(Qt::PointingHandCursor);
    loginLink->setStyleSheet(linkBtnStyle());
    connect(loginLink, &QPushButton::clicked, this, &LoginDialog::showLoginPanel);

    loginRowL->addStretch();
    loginRowL->addWidget(loginTxt);
    loginRowL->addWidget(loginLink);
    loginRowL->addStretch();
    mainL->addWidget(loginRow);
}

// ─────────────────────────────────────────────────────────────────
// NAV
// ─────────────────────────────────────────────────────────────────
void LoginDialog::showLoginPanel()
{
    m_errorLabel->hide();
    m_stack->setCurrentIndex(0);
    setFixedHeight(520);
}

void LoginDialog::showRegisterPanel()
{
    m_regErrorLabel->hide();
    m_regUsernameEdit->clear();
    m_regEmailEdit->clear();
    m_regPassEdit->clear();
    m_regPass2Edit->clear();
    m_stack->setCurrentIndex(1);
    setFixedHeight(580);
}

// ─────────────────────────────────────────────────────────────────
// LOGIN
// ─────────────────────────────────────────────────────────────────
void LoginDialog::onLogin()
{
    QString email    = m_userEdit->text().trimmed();
    QString password = m_passEdit->text();

    if (email.isEmpty() || password.isEmpty())
    {
        m_errorLabel->setText("Completa email y contraseña.");
        m_errorLabel->show();
        return;
    }

    m_loginBtn->setEnabled(false);
    m_loginBtn->setText("Conectando...");
    m_errorLabel->hide();

    DataManager::instance().loginRed(email, password);
}

// nuevo: slot que se ejecuta cuando el vps confirma el login exitoso
void LoginDialog::onLoginExitoso(int idUsuario, const QString& nombre)
{
    Q_UNUSED(idUsuario);

    m_loginBtn->setEnabled(true);
    m_loginBtn->setText("Ingresar");

    m_username   = nombre;
    m_registered = false;

    // Guardar email si el checkbox está marcado
    if (m_recordarmeCheck && m_recordarmeCheck->isChecked())
    {
        QSettings settings("AlcancIA", "Login");
        settings.setValue("ultimoEmail", m_userEdit->text());
    }
    else
    {
        QSettings settings("AlcancIA", "Login");
        settings.remove("ultimoEmail");
    }

    accept();
}

// ─────────────────────────────────────────────────────────────────
// REGISTER
// ─────────────────────────────────────────────────────────────────
void LoginDialog::onRegister()
{
    QString username = m_regUsernameEdit->text().trimmed();
    QString email    = m_regEmailEdit->text().trimmed();
    QString pass1    = m_regPassEdit->text();
    QString pass2    = m_regPass2Edit->text();

    if (username.isEmpty() || pass1.isEmpty())
    {
        m_regErrorLabel->setText("Completa nombre y contraseña.");
        m_regErrorLabel->show();
        return;
    }

    if (username.length() < 3)
    {
        m_regErrorLabel->setText("El nombre debe tener al menos 3 caracteres.");
        m_regErrorLabel->show();
        return;
    }

    if (pass1.length() < 6)
    {
        m_regErrorLabel->setText("La contraseña debe tener al menos 6 caracteres.");
        m_regErrorLabel->show();
        return;
    }

    if (pass1 != pass2)
    {
        m_regErrorLabel->setText("Las contraseñas no coinciden.");
        m_regErrorLabel->show();
        m_regPass2Edit->clear();
        m_regPass2Edit->setFocus();
        return;
    }

    if (!email.isEmpty())
    {
        QRegularExpression regex(
            R"(^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}$)",
            QRegularExpression::CaseInsensitiveOption
            );
        if (!regex.match(email).hasMatch())
        {
            m_regErrorLabel->setText("Ingresa un email valido.");
            m_regErrorLabel->show();
            return;
        }
    }

    if (email.isEmpty())
        email = username + "@email.com";

    m_registerBtn->setEnabled(false);
    m_registerBtn->setText("Conectando al VPS...");

    DataManager::instance().registrarUsuarioRed(username, email, pass1);
}

// ─────────────────────────────────────────────────────────────────
// RESPUESTA REGISTRO
// ─────────────────────────────────────────────────────────────────
void LoginDialog::onRegistroRespuesta(bool exito, const QString& mensaje)
{
    m_registerBtn->setEnabled(true);
    m_registerBtn->setText("Crear cuenta");

    if (exito)
    {
        QMessageBox::information(
            this, "Registro exitoso",
            "La cuenta fue creada correctamente.\nAhora ingresa tus datos para entrar."
            );
        showLoginPanel();
        m_userEdit->setText(m_regEmailEdit->text());
        m_passEdit->setFocus();
    }
    else
    {
        m_regErrorLabel->setText(mensaje);
        m_regErrorLabel->show();
    }
}

// ─────────────────────────────────────────────────────────────────
// GETTERS
// ─────────────────────────────────────────────────────────────────
QString LoginDialog::getUsername() const { return m_username; }
bool    LoginDialog::wasRegistered() const { return m_registered; }

// ─────────────────────────────────────────────────────────────────
// Cargar último email recordado desde QSettings
// ─────────────────────────────────────────────────────────────────
void LoginDialog::cargarUltimoUsuario()
{
    QSettings settings("AlcancIA", "Login");
    QString ultimoEmail = settings.value("ultimoEmail").toString();
    if (!ultimoEmail.isEmpty())
    {
        m_userEdit->setText(ultimoEmail);
        if (m_recordarmeCheck)
            m_recordarmeCheck->setChecked(true);
        m_passEdit->setFocus();
    }
}