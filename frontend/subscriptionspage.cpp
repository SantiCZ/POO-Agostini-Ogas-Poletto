#include "subscriptionspage.h"
#include "addsubscriptiondialog.h"
#include "datamanager.h"
#include <QScrollArea>
#include <algorithm>

SubCard::SubCard(const Suscripcion &sub, QWidget *parent) : QFrame(parent) {
    setFixedHeight(96);
    int diasRestantes = QDate::currentDate().daysTo(sub.fechaVencimiento);
    QString alertColor = "#2E3347";
    if (diasRestantes <= 3) alertColor = "#F87171";
    else if (diasRestantes <= 7) alertColor = "#FBBF24";

    setStyleSheet(QString(R"(
        QFrame {
            background-color: #1A1D27;
            border: 1px solid %1;
            border-radius: 12px;
        }
        QFrame:hover { background-color: #1E2235; }
    )").arg(alertColor));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(14);

    QMap<QString, QString> icons = {
        {"netflix",  "🎬"}, {"spotify",  "🎵"}, {"disney",   "🏰"},
        {"amazon",   "📦"}, {"hbo",      "📺"}, {"apple",    "🍎"},
        {"gimnasio", "💪"}, {"gym",      "💪"}, {"luz",      "⚡"},
        {"epec",     "⚡"}, {"gas",      "🔥"}, {"internet", "🌐"},
        {"otro",     "🔄"}
    };
    QString icon = "🔄";
    for (auto it = icons.begin(); it != icons.end(); ++it) {
        if (sub.iconoNombre.toLower().contains(it.key())) {
            icon = it.value();
            break;
        }
    }

    QLabel *iconL = new QLabel(icon);
    iconL->setFixedSize(52, 52);
    iconL->setAlignment(Qt::AlignCenter);
    iconL->setStyleSheet("background-color: #21253A; border-radius: 13px; font-size: 24px;");

    QWidget *infoW = new QWidget();
    infoW->setStyleSheet("background: transparent;");
    QVBoxLayout *infoL = new QVBoxLayout(infoW);
    infoL->setContentsMargins(0,0,0,0);
    infoL->setSpacing(4);

    QWidget *nameRow = new QWidget();
    nameRow->setStyleSheet("background: transparent;");
    QHBoxLayout *nameRowL = new QHBoxLayout(nameRow);
    nameRowL->setContentsMargins(0,0,0,0);
    nameRowL->setSpacing(8);

    QLabel *nameL = new QLabel(sub.nombreServicio);
    nameL->setStyleSheet("color: #F1F5F9; font-size: 14px; font-weight: 700; background: transparent;");
    nameRowL->addWidget(nameL);
    if (!sub.activa) {
        QLabel *pauseBadge = new QLabel("Pausada");
        pauseBadge->setStyleSheet("color: #64748B; background-color: #21253A; border-radius: 6px; padding: 2px 8px; font-size: 11px;");
        nameRowL->addWidget(pauseBadge);
    }
    nameRowL->addStretch();

    QString diasTxt, diasStyle;
    if (diasRestantes < 0) { diasTxt = "¡Vencida!"; diasStyle = "color: #F87171;"; }
    else if (diasRestantes == 0) { diasTxt = "Vence HOY"; diasStyle = "color: #F87171;"; }
    else if (diasRestantes <= 7) { diasTxt = QString("Vence en %1 días").arg(diasRestantes); diasStyle = "color: #FBBF24;"; }
    else { diasTxt = QString("Vence: %1").arg(sub.fechaVencimiento.toString("dd/MM/yyyy")); diasStyle = "color: #64748B;"; }

    QLabel *diasL = new QLabel(diasTxt);
    diasL->setStyleSheet(diasStyle + "font-size: 12px; background: transparent; font-weight: 500;");
    QLabel *catL = new QLabel(sub.categoria);
    catL->setStyleSheet("color: #475569; font-size: 11px; background: transparent;");

    infoL->addWidget(nameRow);
    infoL->addWidget(diasL);
    infoL->addWidget(catL);

    QWidget *rightW = new QWidget();
    rightW->setStyleSheet("background: transparent;");
    QVBoxLayout *rightL = new QVBoxLayout(rightW);
    rightL->setContentsMargins(0,0,0,0);
    rightL->setSpacing(6);
    rightL->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *montoL = new QLabel(QString("$%1/mes").arg(sub.monto, 0, 'f', 0));
    montoL->setAlignment(Qt::AlignRight);
    montoL->setStyleSheet("color: #38BDF8; font-size: 15px; font-weight: 700; background: transparent;");

    QWidget *actionsW = new QWidget();
    actionsW->setStyleSheet("background: transparent;");
    QHBoxLayout *actL = new QHBoxLayout(actionsW);
    actL->setContentsMargins(0,0,0,0);
    actL->setSpacing(4);

    QPushButton *toggleBtn = new QPushButton(sub.activa ? "⏸" : "▶");
    toggleBtn->setFixedSize(28, 28);
    toggleBtn->setCursor(Qt::PointingHandCursor);
    toggleBtn->setStyleSheet("QPushButton { background: transparent; border: none; font-size: 14px; }"
                             "QPushButton:hover { background-color: #21253A; border-radius: 6px; }");
    connect(toggleBtn, &QPushButton::clicked, this, [this, id = sub.id, activa = sub.activa]() {
        emit toggleRequested(id, !activa);
    });

    QPushButton *delBtn = new QPushButton("🗑");
    delBtn->setFixedSize(28, 28);
    delBtn->setCursor(Qt::PointingHandCursor);
    delBtn->setStyleSheet("QPushButton { background: transparent; border: none; font-size: 14px; }"
                          "QPushButton:hover { background-color: rgba(248,113,113,0.1); border-radius: 6px; }");
    connect(delBtn, &QPushButton::clicked, this, [this, id = sub.id]() { emit deleteRequested(id); });

    actL->addWidget(toggleBtn);
    actL->addWidget(delBtn);
    rightL->addWidget(montoL);
    rightL->addWidget(actionsW, 0, Qt::AlignRight);

    layout->addWidget(iconL);
    layout->addWidget(infoW, 1);
    layout->addWidget(rightW);
}

SubscriptionsPage::SubscriptionsPage(QWidget *parent) : QWidget(parent) {
    setStyleSheet("background: transparent;");
    setupUI();
    refreshData();
}

void SubscriptionsPage::setupUI() {
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *content = new QWidget();
    content->setStyleSheet("background: transparent;");
    QVBoxLayout *mainL = new QVBoxLayout(content);
    mainL->setContentsMargins(32, 28, 32, 32);
    mainL->setSpacing(20);

    // Header
    QWidget *headerW = new QWidget();
    headerW->setStyleSheet("background: transparent;");
    QHBoxLayout *headerL = new QHBoxLayout(headerW);
    headerL->setContentsMargins(0,0,0,0);

    QWidget *titleW = new QWidget();
    titleW->setStyleSheet("background: transparent;");
    QVBoxLayout *titleL = new QVBoxLayout(titleW);
    titleL->setContentsMargins(0,0,0,0);
    titleL->setSpacing(2);

    QLabel *title = new QLabel("🔄  Suscripciones");
    title->setStyleSheet("color: #F1F5F9; font-size: 24px; font-weight: 700; background: transparent;");

    QWidget *statsRow = new QWidget();
    statsRow->setStyleSheet("background: transparent;");
    QHBoxLayout *statsL = new QHBoxLayout(statsRow);
    statsL->setContentsMargins(0,0,0,0);
    statsL->setSpacing(16);

    m_countLabel = new QLabel("0 suscripciones");
    m_countLabel->setStyleSheet("color: #64748B; font-size: 13px; background: transparent;");
    m_totalLabel = new QLabel("Total: $0/mes");
    m_totalLabel->setStyleSheet("color: #38BDF8; font-size: 13px; font-weight: 600; background: transparent;");

    statsL->addWidget(m_countLabel);
    statsL->addWidget(m_totalLabel);
    titleL->addWidget(title);
    titleL->addWidget(statsRow);

    QPushButton *addBtn = new QPushButton("➕  Nueva Suscripción");
    addBtn->setFixedHeight(42);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #38BDF8, stop:1 #0EA5E9);
            color: #0F1117;
            border: none;
            border-radius: 10px;
            padding: 0 20px;
            font-size: 13px;
            font-weight: 700;
        }
        QPushButton:hover { background: #0EA5E9; }
    )");
    connect(addBtn, &QPushButton::clicked, this, &SubscriptionsPage::onAddClicked);

    headerL->addWidget(titleW);
    headerL->addStretch();
    headerL->addWidget(addBtn);
    mainL->addWidget(headerW);

    // Lista
    m_subListWidget = new QWidget();
    m_subListWidget->setStyleSheet("background: transparent;");
    m_subListLayout = new QVBoxLayout(m_subListWidget);
    m_subListLayout->setContentsMargins(0,0,0,0);
    m_subListLayout->setSpacing(10);

    mainL->addWidget(m_subListWidget);
    mainL->addStretch();

    scroll->setWidget(content);
    QVBoxLayout *pageL = new QVBoxLayout(this);
    pageL->setContentsMargins(0,0,0,0);
    pageL->addWidget(scroll);
}

void SubscriptionsPage::refreshData() {
    // Limpiar layout
    QLayoutItem *item;
    while ((item = m_subListLayout->takeAt(0))) {
        delete item->widget();
        delete item;
    }
    QVector<Suscripcion> subs = DataManager::instance().getSuscripciones();
    double total = 0;
    for (const Suscripcion& s : subs) {
        if (s.activa) total += s.monto;
        addSubCard(s);
    }
    m_countLabel->setText(QString("%1 suscripciones").arg(subs.size()));
    m_totalLabel->setText(QString("Total activas: $%1/mes").arg(total, 0, 'f', 0));
}

void SubscriptionsPage::addSubCard(const Suscripcion &s) {
    auto *card = new SubCard(s, this);
    connect(card, &SubCard::toggleRequested, this, [this](int id, bool activa) {
        if (DataManager::instance().updateSuscripcionEstado(id, activa)) {
            refreshData();
        }
    });
    connect(card, &SubCard::deleteRequested, this, [this](int id) {
        if (DataManager::instance().removeSuscripcion(id)) {
            refreshData();
        }
    });
    m_subListLayout->addWidget(card);
}

void SubscriptionsPage::onAddClicked() {
    AddSubscriptionDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Suscripcion s;
        s.id = 0;
        s.monto = dlg.monto();
        s.fecha = QDate::currentDate();
        s.categoria = dlg.categoria();
        s.descripcion = "";
        s.nombreServicio = dlg.nombreServicio().isEmpty() ? "Sin nombre" : dlg.nombreServicio();
        s.fechaVencimiento = dlg.fechaVencimiento();
        s.diasAviso = dlg.diasAviso();
        s.activa = true;
        s.iconoNombre = dlg.iconoNombre();
        if (DataManager::instance().addSuscripcion(s)) {
            refreshData();
        }
    }
}