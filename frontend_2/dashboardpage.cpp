#include "dashboardpage.h"
#include "stylemanager.h"
#include "datamanager.h"
#include "admindb.h"

#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QLocale>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>

// NUEVO
#include <algorithm>

// ─────────────────────────────────────────────────────────────
// StatCard
// ─────────────────────────────────────────────────────────────
StatCard::StatCard(const QString &icon, const QString &title,
                   const QString &value, const QString &subtitle,
                   const QString &accentColor, QWidget *parent)
    : QFrame(parent)
{
    setFixedHeight(120);
    setMinimumWidth(180);

    setStyleSheet(QString(R"(
        QFrame {
            background-color: #1A1D27;
            border: none;
            border-radius: 14px;
        }
    )"));

    auto *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(20);
    effect->setOffset(0, 4);
    effect->setColor(QColor(0, 0, 0, 60));
    setGraphicsEffect(effect);

    QHBoxLayout *root = new QHBoxLayout(this);
    root->setContentsMargins(20, 16, 20, 16);
    root->setSpacing(16);

    QLabel *iconLabel = new QLabel(icon);
    iconLabel->setFixedSize(48, 48);
    iconLabel->setAlignment(Qt::AlignCenter);

    iconLabel->setStyleSheet(QString(R"(
        background-color: %1;
        border-radius: 12px;
        font-size: 22px;
    )").arg(accentColor + "22"));

    QWidget *textW = new QWidget();
    textW->setStyleSheet("background: transparent;");

    QVBoxLayout *textL = new QVBoxLayout(textW);
    textL->setContentsMargins(0, 0, 0, 0);
    textL->setSpacing(2);

    QLabel *titleL = new QLabel(title);
    titleL->setStyleSheet(
        "color: #64748B;"
        "font-size: 11px;"
        "font-weight: 500;"
        "letter-spacing: 0.5px;"
        "background: transparent;"
        );

    QLabel *valueL = new QLabel(value);
    valueL->setStyleSheet(QString(
                              "color: %1;"
                              "font-size: 22px;"
                              "font-weight: 700;"
                              "background: transparent;"
                              ).arg(accentColor));

    QLabel *subL = new QLabel(subtitle);
    subL->setStyleSheet(
        "color: #475569;"
        "font-size: 11px;"
        "background: transparent;"
        );

    textL->addWidget(titleL);
    textL->addWidget(valueL);
    textL->addWidget(subL);

    root->addWidget(iconLabel);
    root->addWidget(textW);
    root->addStretch();
}

// ─────────────────────────────────────────────────────────────
// RecentExpenseRow
// ─────────────────────────────────────────────────────────────
RecentExpenseRow::RecentExpenseRow(const QString &icon,
                                   const QString &nombre,
                                   const QString &categoria,
                                   const QString &fecha,
                                   double monto,
                                   QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(58);

    setStyleSheet("background: transparent;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(12);

    QLabel *iconL = new QLabel(icon);
    iconL->setFixedSize(40, 40);
    iconL->setAlignment(Qt::AlignCenter);

    iconL->setStyleSheet(
        "background-color: #21253A;"
        "border-radius: 10px;"
        "font-size: 18px;"
        );

    QWidget *infoW = new QWidget();
    infoW->setStyleSheet("background: transparent;");

    QVBoxLayout *infoL = new QVBoxLayout(infoW);
    infoL->setContentsMargins(0, 0, 0, 0);
    infoL->setSpacing(2);

    QLabel *nameL = new QLabel(nombre);

    nameL->setStyleSheet(
        "color: #E2E8F0;"
        "font-size: 13px;"
        "font-weight: 600;"
        "background: transparent;"
        );

    QLabel *catL = new QLabel(
        QString("%1  ·  %2").arg(categoria, fecha)
        );

    catL->setStyleSheet(
        "color: #475569;"
        "font-size: 11px;"
        "background: transparent;"
        );

    infoL->addWidget(nameL);
    infoL->addWidget(catL);

    QLabel *montoL = new QLabel(
        QString("-$ %1").arg(monto, 0, 'f', 2)
        );

    montoL->setStyleSheet(
        "color: #F87171;"
        "font-size: 14px;"
        "font-weight: 700;"
        "background: transparent;"
        );

    layout->addWidget(iconL);
    layout->addWidget(infoW, 1);
    layout->addWidget(montoL);
}

// ─────────────────────────────────────────────────────────────
// DashboardPage
// ─────────────────────────────────────────────────────────────
DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background: transparent;");

    setupUI();
    refreshData();

    // Refresca automaticamente cuando cambian los tickets o llega sync del servidor
    connect(
        &DataManager::instance(),
        &DataManager::ticketsChanged,
        this,
        &DashboardPage::refreshData
        );

    connect(
        &DataManager::instance(),
        &DataManager::sincronizacionCompletada,
        this,
        &DashboardPage::refreshData
        );

    connect(
        &DataManager::instance(),
        &DataManager::suscripcionesChanged,
        this,
        &DashboardPage::refreshData
        );
}

void DashboardPage::setupUI()
{
    QScrollArea *scroll = new QScrollArea(this);

    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *content = new QWidget();
    content->setStyleSheet("background: transparent;");

    QVBoxLayout *mainL = new QVBoxLayout(content);
    mainL->setContentsMargins(32, 28, 32, 32);
    mainL->setSpacing(24);

    // ───────────────── HEADER ─────────────────

    QWidget *headerW = new QWidget();
    headerW->setStyleSheet("background: transparent;");

    QHBoxLayout *headerL = new QHBoxLayout(headerW);
    headerL->setContentsMargins(0,0,0,0);

    QWidget *greetW = new QWidget();
    greetW->setStyleSheet("background: transparent;");

    QVBoxLayout *greetL = new QVBoxLayout(greetW);
    greetL->setContentsMargins(0,0,0,0);
    greetL->setSpacing(2);

    QLabel *greetTitle = new QLabel("¡Hola! 👋");

    greetTitle->setStyleSheet(
        "color: #F1F5F9;"
        "font-size: 26px;"
        "font-weight: 700;"
        "background: transparent;"
        );

    QLabel *greetSub =
        new QLabel("Acá está el resumen de tus finanzas");

    greetSub->setStyleSheet(
        "color: #64748B;"
        "font-size: 13px;"
        "background: transparent;"
        );

    greetL->addWidget(greetTitle);
    greetL->addWidget(greetSub);

    QLocale spanish(QLocale::Spanish, QLocale::Argentina);

    QLabel *dateLabel = new QLabel(
        spanish.toString(
            QDate::currentDate(),
            "dddd, d 'de' MMMM"
            )
        );

    dateLabel->setStyleSheet(
        "color: #4ADE80;"
        "font-size: 12px;"
        "font-weight: 500;"
        "background-color: rgba(74,222,128,0.08);"
        "border: 1px solid rgba(74,222,128,0.2);"
        "border-radius: 20px;"
        "padding: 6px 14px;"
        );

    headerL->addWidget(greetW);
    headerL->addStretch();
    headerL->addWidget(dateLabel);

    mainL->addWidget(headerW);

    // ───────────────── STATS ─────────────────

    m_statsWidget = new QWidget();
    m_statsWidget->setStyleSheet("background: transparent;");

    m_statsLayout = new QHBoxLayout(m_statsWidget);
    m_statsLayout->setContentsMargins(0,0,0,0);
    m_statsLayout->setSpacing(16);

    mainL->addWidget(m_statsWidget);

    // ───────────────── BOTTOM ─────────────────

    QWidget *bottomW = new QWidget();
    bottomW->setStyleSheet("background: transparent;");

    QHBoxLayout *bottomL = new QHBoxLayout(bottomW);
    bottomL->setContentsMargins(0,0,0,0);
    bottomL->setSpacing(20);

    QWidget *leftW = new QWidget();
    leftW->setStyleSheet("background: transparent;");

    QVBoxLayout *leftL = new QVBoxLayout(leftW);
    leftL->setContentsMargins(0,0,0,0);
    leftL->setSpacing(12);

    buildRecentActivity(leftL);

    m_activityLayout = leftL;

    QWidget *rightW = new QWidget();
    rightW->setStyleSheet("background: transparent;");
    rightW->setFixedWidth(280);

    QVBoxLayout *rightL = new QVBoxLayout(rightW);
    rightL->setContentsMargins(0,0,0,0);
    rightL->setSpacing(16);

    buildQuickActions(rightL);
    buildSubsAlert(rightL);

    m_alertLayout = rightL;

    bottomL->addWidget(leftW, 1);
    bottomL->addWidget(rightW);

    mainL->addWidget(bottomW);
    mainL->addStretch();

    scroll->setWidget(content);

    QVBoxLayout *pageL = new QVBoxLayout(this);
    pageL->setContentsMargins(0,0,0,0);
    pageL->addWidget(scroll);
}

void DashboardPage::clearLayout(QLayout *layout)
{
    if (!layout)
        return;

    QLayoutItem *item;

    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (item->widget())
        {
            delete item->widget();
        }

        delete item;
    }
}

void DashboardPage::refreshData()
{
    clearLayout(m_statsLayout);

    QDate now = QDate::currentDate();

    int year = now.year();
    int month = now.month();

    // ─────────────────────────────────────────
    // NUEVO:
    // Obtener tickets desde SQLite sincronizado
    // ─────────────────────────────────────────

    QVector<Ticket> tickets =
        DataManager::instance().getTickets();

    double gastoMes = 0.0;
    int ticketCount = 0;

    for (const Ticket &t : tickets)
    {
        if (t.fecha.year() == year &&
            t.fecha.month() == month)
        {
            gastoMes += t.monto;
            ticketCount++;
        }
    }

    // CODIGO VIEJO (NO ELIMINADO)
    // double gastoMes =
    //     DataManager::instance().getGastoMes(year, month);

    // int ticketCount =
    //     DataManager::instance().getTicketCountMes(year, month);

    int subsActivas =
        DataManager::instance().getSuscripcionesActivas();

    int proximas = 0;

    for (const Suscripcion& s :
         DataManager::instance().getSuscripciones())
    {
        if (s.activa)
        {
            int days =
                QDate::currentDate().daysTo(
                    s.fechaVencimiento
                    );

            if (days >= 0 && days <= 7)
            {
                proximas++;
            }
        }
    }

    m_statsLayout->addWidget(
        new StatCard(
            "💰",
            "GASTO DEL MES",
            QString("$%1").arg(gastoMes, 0, 'f', 0),
            now.toString("MMMM yyyy"),
            "#4ADE80"
            )
        );

    m_statsLayout->addWidget(
        new StatCard(
            "🧾",
            "TICKETS",
            QString::number(ticketCount),
            "este mes",
            "#818CF8"
            )
        );

    m_statsLayout->addWidget(
        new StatCard(
            "🔄",
            "SUSCRIPCIONES",
            QString::number(subsActivas),
            "activas",
            "#38BDF8"
            )
        );

    m_statsLayout->addWidget(
        new StatCard(
            "⚠️",
            "POR VENCER",
            QString::number(proximas),
            "próx. 7 días",
            "#FBBF24"
            )
        );

    clearLayout(m_activityLayout);

    buildRecentActivity(m_activityLayout);
}

// ─────────────────────────────────────────────
// ACTIVIDAD RECIENTE
// ─────────────────────────────────────────────
void DashboardPage::buildRecentActivity(QVBoxLayout *layout)
{
    QWidget *titleW = new QWidget();
    titleW->setStyleSheet("background: transparent;");

    QHBoxLayout *titleL = new QHBoxLayout(titleW);
    titleL->setContentsMargins(0,0,0,0);

    QLabel *title =
        new QLabel("Actividad Reciente");

    title->setStyleSheet(
        "color: #F1F5F9;"
        "font-size: 16px;"
        "font-weight: 700;"
        "background: transparent;"
        );

    QPushButton *verTodo =
        new QPushButton("Ver todo →");

    verTodo->setCursor(Qt::PointingHandCursor);

    verTodo->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #4ADE80;
            border: none;
            font-size: 12px;
            font-weight: 600;
        }

        QPushButton:hover {
            color: #22C55E;
        }
    )");

    connect(
        verTodo,
        &QPushButton::clicked,
        this,
        &DashboardPage::navigateToTickets
        );

    titleL->addWidget(title);
    titleL->addStretch();
    titleL->addWidget(verTodo);

    layout->addWidget(titleW);

    // ───────────────── CARD ─────────────────

    QFrame *card = new QFrame();

    card->setStyleSheet(R"(
        QFrame {
            background-color: #1A1D27;
            border: 1px solid #2E3347;
            border-radius: 14px;
        }
    )");

    QVBoxLayout *cardL =
        new QVBoxLayout(card);

    cardL->setContentsMargins(0, 8, 0, 8);
    cardL->setSpacing(0);

    QVector<Ticket> tickets =
        DataManager::instance().getTickets();

    std::sort(
        tickets.begin(),
        tickets.end(),
        [](const Ticket& a, const Ticket& b)
        {
            return a.fecha > b.fecha;
        }
        );

    int maxItems = qMin(5, tickets.size());

    for (int i = 0; i < maxItems; ++i)
    {
        const Ticket& t = tickets[i];

        QString icon = "📋";

        QMap<QString,QString> icons = {
            {"Supermercado","🛒"},
            {"Restaurante","🍕"},
            {"Transporte","🚌"},
            {"Salud","💊"},
            {"Entretenimiento","🎬"},
            {"Servicios","⚡"}
        };

        if (icons.contains(t.categoria))
        {
            icon = icons[t.categoria];
        }

        auto *row = new RecentExpenseRow(
            icon,
            t.nombreLocal,
            t.categoria,
            t.fecha.toString("dd/MM/yyyy"),
            t.monto
            );

        cardL->addWidget(row);

        if (i < maxItems - 1)
        {
            QFrame *sep = new QFrame();

            sep->setFrameShape(QFrame::HLine);

            sep->setStyleSheet(
                "background-color: #1E2235;"
                "border: none;"
                "max-height: 1px;"
                "margin: 0 16px;"
                );

            cardL->addWidget(sep);
        }
    }

    if (tickets.isEmpty())
    {
        QLabel *empty = new QLabel(
            "No hay tickets aún. Subí tu primer ticket."
            );

        empty->setAlignment(Qt::AlignCenter);

        empty->setStyleSheet(
            "color: #64748B;"
            "padding: 20px;"
            );

        cardL->addWidget(empty);
    }

    layout->addWidget(card);
}
void DashboardPage::buildQuickActions(QVBoxLayout *layout)
{
    Q_UNUSED(layout);
}
void DashboardPage::buildSubsAlert(QVBoxLayout *layout)
{
    Q_UNUSED(layout);
}