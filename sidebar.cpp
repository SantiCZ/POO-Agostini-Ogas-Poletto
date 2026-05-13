#include "sidebar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

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
    setStyleSheet(R"(
        QWidget#sidebar {
            background-color: #13161F;
            border-right: 1px solid #1E2235;
        }
    )");
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

    // Botones de navegación
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

    m_layout->addStretch();

    // ── Separador ─────────────────────────────────────────────────
    QFrame *sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("background-color: #1E2235; border: none; max-height: 1px;");
    m_layout->addWidget(sep2);
    m_layout->addSpacing(12);

    // ── Perfil de usuario ─────────────────────────────────────────
    QWidget *profileW = new QWidget();
    profileW->setStyleSheet("background: transparent;");
    QHBoxLayout *profileL = new QHBoxLayout(profileW);
    profileL->setContentsMargins(12, 8, 12, 8);
    profileL->setSpacing(10);

    QLabel *avatar = new QLabel("👤");
    avatar->setFixedSize(36, 36);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        "background-color: #21253A; border-radius: 18px; "
        "font-size: 18px;"
    );

    QWidget *userInfoW = new QWidget();
    userInfoW->setStyleSheet("background: transparent;");
    QVBoxLayout *userInfoL = new QVBoxLayout(userInfoW);
    userInfoL->setContentsMargins(0, 0, 0, 0);
    userInfoL->setSpacing(1);

    QLabel *userName = new QLabel("Mi Cuenta");
    userName->setStyleSheet(
        "color: #F1F5F9; font-size: 13px; font-weight: 600; background: transparent;"
    );
    QLabel *userEmail = new QLabel("usuario@email.com");
    userEmail->setStyleSheet(
        "color: #64748B; font-size: 11px; background: transparent;"
    );

    userInfoL->addWidget(userName);
    userInfoL->addWidget(userEmail);
    profileL->addWidget(avatar);
    profileL->addWidget(userInfoW);
    profileL->addStretch();

    m_layout->addWidget(profileW);

    // Activar primer botón
    setActivePage(0);
}

void Sidebar::setActivePage(int index) {
    m_activeIndex = index;
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setActive(i == index);
        m_navButtons[i]->setChecked(i == index);
    }
}
