#include "sidebar.h"
#include "stylemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QAction>
#include <QCursor>

// ─── SidebarButton ────────────────────────────────────────────────
SidebarButton::SidebarButton(const QString &icon, const QString &text, QWidget *parent)
    : QPushButton(parent), m_icon(icon), m_text(text)
{
    setFixedHeight(48);
    setFixedWidth(220);
    setCursor(Qt::PointingHandCursor);
    setText(QString("  %1   %2").arg(icon).arg(text));
    setCheckable(true);
    updateStyle(false);
}

void SidebarButton::setActive(bool active) {
    updateStyle(active);
}

void SidebarButton::updateStyle(bool active) {
    if (active) {
        setStyleSheet(R"(
            QPushButton {
                background-color: rgba(74, 222, 128, 0.12);
                color: #4ADE80;
                border: none;
                border-left: 3px solid #4ADE80;
                border-radius: 8px;
                text-align: left;
                padding-left: 16px;
                font-size: 14px;
                font-weight: 600;
            }
        )");
    } else {
        setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                color: #64748B;
                border: none;
                border-left: 3px solid transparent;
                border-radius: 8px;
                text-align: left;
                padding-left: 16px;
                font-size: 14px;
                font-weight: 400;
            }
            QPushButton:hover {
                background-color: rgba(255,255,255,0.04);
                color: #94A3B8;
            }
        )");
    }
}

// ─── Sidebar ──────────────────────────────────────────────────────
Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    setObjectName("sidebar");
    setFixedWidth(240);
    setStyleSheet(StyleManager::sidebarStyle());
    setupUI();
}

void Sidebar::setupUI() {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(10, 0, 10, 20);
    m_layout->setSpacing(4);

    // ── Logo ──────────────────────────────────────────────────────
    QWidget *logoContainer = new QWidget();
    logoContainer->setFixedHeight(80);
    QHBoxLayout *logoLayout = new QHBoxLayout(logoContainer);
    logoLayout->setContentsMargins(10, 0, 0, 0);

    QLabel *logoIcon = new QLabel("🏦");
    logoIcon->setStyleSheet("font-size: 26px; background: transparent;");

    QWidget *logoTextW = new QWidget();
    QVBoxLayout *logoTextL = new QVBoxLayout(logoTextW);
    logoTextL->setContentsMargins(8, 0, 0, 0);
    logoTextL->setSpacing(0);
    logoTextW->setStyleSheet("background: transparent;");

    QLabel *logoName = new QLabel("AlcancIA");
    logoName->setStyleSheet(
        "color: #F1F5F9; font-size: 18px; font-weight: 700; "
        "background: transparent; letter-spacing: -0.3px;"
        );
    QLabel *logoSub = new QLabel("Asistente financiero");
    logoSub->setStyleSheet(
        "color: #4ADE80; font-size: 10px; font-weight: 500; "
        "background: transparent; letter-spacing: 0.5px;"
        );

    logoTextL->addWidget(logoName);
    logoTextL->addWidget(logoSub);
    logoLayout->addWidget(logoIcon);
    logoLayout->addWidget(logoTextW);
    logoLayout->addStretch();

    m_layout->addWidget(logoContainer);

    // ── Separador ─────────────────────────────────────────────────
    QFrame *sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("background-color: #1E2235; border: none; max-height: 1px;");
    m_layout->addWidget(sep1);
    m_layout->addSpacing(12);

    // ── Sección principal ─────────────────────────────────────────
    QLabel *secLabel = new QLabel("MENÚ");
    secLabel->setStyleSheet(
        "color: #374151; font-size: 10px; font-weight: 700; "
        "letter-spacing: 1.5px; padding-left: 18px; background: transparent;"
        );
    m_layout->addWidget(secLabel);
    m_layout->addSpacing(6);

    struct NavItem { QString icon; QString label; };
    QVector<NavItem> items = {
                              {"🏠", "Dashboard"},
                              {"🧾", "Tickets"},
                              {"🔄", "Suscripciones"},
                              {"📊", "Reportes"},
                              };

    m_btnGroup = new QButtonGroup(this);
    m_btnGroup->setExclusive(true);

    for (int i = 0; i < items.size(); ++i) {
        auto *btn = new SidebarButton(items[i].icon, items[i].label, this);
        m_btnGroup->addButton(btn, i);
        m_navButtons.append(btn);
        m_layout->addWidget(btn);

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            setActivePage(i);
            emit pageChanged(i);
        });
    }

    m_layout->addSpacing(12);

    // boton subir ticket: acceso rapido desde cualquier pagina
    QPushButton *uploadBtn = new QPushButton("📷   Subir Ticket");
    uploadBtn->setFixedHeight(42);
    uploadBtn->setFixedWidth(220);
    uploadBtn->setCursor(Qt::PointingHandCursor);
    uploadBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #4ADE80, stop:1 #22C55E);
            color: #0F1117;
            border: none;
            border-radius: 10px;
            font-size: 13px;
            font-weight: 700;
            text-align: left;
            padding-left: 18px;
        }
        QPushButton:hover { background: #22C55E; }
        QPushButton:pressed { background: #16A34A; }
    )");
    connect(uploadBtn, &QPushButton::clicked, this, [this]() {
        // navega a tickets y abre el dialogo de subida
        setActivePage(1);
        emit pageChanged(1);
        emit uploadTicketRequested();
    });
    m_layout->addWidget(uploadBtn);

    m_layout->addStretch();

    // ── Separador ─────────────────────────────────────────────────
    QFrame *sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("background-color: #1E2235; border: none; max-height: 1px;");
    m_layout->addWidget(sep2);
    m_layout->addSpacing(12);

    // ── Indicador de sincronización ───────────────────────────────
    m_syncIcon = new QLabel();
    m_syncIcon->setMinimumWidth(160);
    m_syncIcon->setFixedHeight(28);
    m_syncIcon->setAlignment(Qt::AlignCenter);
    m_syncIcon->setStyleSheet("background: transparent; font-size: 14px; letter-spacing: 4px;");
    setSyncStatus(true, 0);
    m_layout->addWidget(m_syncIcon, 0, Qt::AlignCenter);
    m_layout->addSpacing(6);

    // ── Perfil de usuario (clickeable → menú) ─────────────────────
    m_profileBtn = new QPushButton();
    m_profileBtn->setFixedHeight(56);
    m_profileBtn->setCursor(Qt::PointingHandCursor);
    m_profileBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1A1D27;
            border: 1px solid #2E3347;
            border-radius: 10px;
            text-align: left;
            padding: 0px;
        }
        QPushButton:hover {
            background-color: #21253A;
            border: 1px solid #4ADE80;
        }
        QPushButton:pressed {
            background-color: #2E3347;
        }
    )");

    // Layout interno del botón de perfil
    QWidget *profileInner = new QWidget(m_profileBtn);
    profileInner->setAttribute(Qt::WA_TransparentForMouseEvents);
    profileInner->setStyleSheet("background: transparent;");
    QHBoxLayout *profileL = new QHBoxLayout(profileInner);
    profileL->setContentsMargins(10, 8, 10, 8);
    profileL->setSpacing(10);

    QLabel *avatar = new QLabel("👤");
    avatar->setFixedSize(36, 36);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        "background-color: #21253A; border-radius: 18px; font-size: 18px;"
        );

    QWidget *userInfoW = new QWidget();
    userInfoW->setStyleSheet("background: transparent;");
    QVBoxLayout *userInfoL = new QVBoxLayout(userInfoW);
    userInfoL->setContentsMargins(0, 0, 0, 0);
    userInfoL->setSpacing(1);

    m_userNameLabel = new QLabel("Mi Cuenta");
    m_userNameLabel->setStyleSheet(
        "color: #F1F5F9; font-size: 13px; font-weight: 600; background: transparent;"
        );
    m_userEmailLabel = new QLabel("usuario@email.com");
    m_userEmailLabel->setStyleSheet(
        "color: #64748B; font-size: 11px; background: transparent;"
        );

    userInfoL->addWidget(m_userNameLabel);
    userInfoL->addWidget(m_userEmailLabel);
    profileL->addWidget(avatar);
    profileL->addWidget(userInfoW, 1);

    // Flecha indicando que es clickeable
    QLabel *arrowL = new QLabel("⋮");
    arrowL->setStyleSheet("color: #475569; font-size: 16px; background: transparent;");
    profileL->addWidget(arrowL);

    // Ajustar tamaño del widget interno al botón
    profileInner->setGeometry(0, 0, m_profileBtn->width(), m_profileBtn->height());
    connect(m_profileBtn, &QPushButton::clicked, this, &Sidebar::onProfileClicked);

    m_layout->addWidget(m_profileBtn);
    m_layout->addSpacing(10);

    setActivePage(0);
}

void Sidebar::onProfileClicked() {
    // menu simplificado: solo logout
    // las opciones de editar perfil y cambiar contrasena se sacaron
    // porque no tienen implementacion activa todavia
    QMenu *menu = new QMenu(this);
    menu->setStyleSheet(R"(
        QMenu {
            background-color: #1A1D27;
            border: 1px solid #2E3347;
            border-radius: 10px;
            padding: 6px;
        }
        QMenu::item {
            color: #E2E8F0;
            font-size: 13px;
            padding: 10px 16px;
            border-radius: 6px;
        }
        QMenu::item:selected {
            background-color: #21253A;
            color: #F1F5F9;
        }
        QMenu::separator {
            height: 1px;
            background-color: #2E3347;
            margin: 4px 8px;
        }
    )");

    // encabezado con nombre (no clickeable)
    QAction *headerAction = new QAction(
        QString("👤  %1").arg(m_userNameLabel->text()), this
        );
    headerAction->setEnabled(false);
    menu->addAction(headerAction);
    menu->addSeparator();

    // cerrar sesion
    QAction *logoutAction = new QAction("🚪   Cerrar sesión", this);
    connect(logoutAction, &QAction::triggered, this, &Sidebar::logoutRequested);
    menu->addAction(logoutAction);

    QPoint pos = m_profileBtn->mapToGlobal(QPoint(0, -menu->sizeHint().height() - 4));
    menu->exec(pos);
}

void Sidebar::setActivePage(int index) {
    m_activeIndex = index;
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setActive(i == index);
        m_navButtons[i]->setChecked(i == index);
    }
}

void Sidebar::setUserInfo(const QString &name, const QString &email) {
    if (m_userNameLabel)  m_userNameLabel->setText(name);
    if (m_userEmailLabel) m_userEmailLabel->setText(email);
}

void Sidebar::setSyncStatus(bool online, int pending) {
    if (!m_syncIcon) return;
    if (online && pending == 0) {
        m_syncIcon->setText("✅  ☁️");
        m_syncIcon->setToolTip("Todo sincronizado");
    } else if (online && pending > 0) {
        m_syncIcon->setText("🔄  ☁️");
        m_syncIcon->setToolTip(QString("Sincronizando (%1 cambios pendientes)").arg(pending));
    } else {
        m_syncIcon->setText("🔴  ☁️");
        m_syncIcon->setToolTip(QString("Desconectado. %1 cambios locales pendientes").arg(pending));
    }
}