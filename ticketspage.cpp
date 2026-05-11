#include "ticketspage.h"
#include "uploadticketdialog.h"
#include "stylemanager.h"
#include "datamanager.h"
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <algorithm>

TicketCard::TicketCard(const Ticket &ticket, QWidget *parent) : QFrame(parent) {
    setFixedHeight(88);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(R"(
        QFrame {
            background-color: #1A1D27;
            border: 1px solid #2E3347;
            border-radius: 12px;
        }
        QFrame:hover {
            border: 1px solid #4ADE80;
            background-color: #1E2235;
        }
    )");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(14);

    QMap<QString,QString> catIcons = {
        {"Supermercado","🛒"}, {"Restaurante","🍕"}, {"Transporte","🚌"},
        {"Salud","💊"}, {"Entretenimiento","🎬"}, {"Servicios","⚡"},
        {"Ropa","👕"}, {"Tecnología","💻"}, {"Otro","📋"}
    };
    QString icon = catIcons.value(ticket.categoria, "📋");

    QLabel *iconL = new QLabel(icon);
    iconL->setFixedSize(48, 48);
    iconL->setAlignment(Qt::AlignCenter);
    iconL->setStyleSheet("background-color: #21253A; border-radius: 12px; font-size: 22px;");

    QWidget *infoW = new QWidget();
    infoW->setStyleSheet("background: transparent;");
    QVBoxLayout *infoL = new QVBoxLayout(infoW);
    infoL->setContentsMargins(0,0,0,0);
    infoL->setSpacing(3);

    QLabel *nameL = new QLabel(ticket.nombreLocal);
    nameL->setStyleSheet("color: #F1F5F9; font-size: 14px; font-weight: 600; background: transparent;");

    QWidget *metaW = new QWidget();
    metaW->setStyleSheet("background: transparent;");
    QHBoxLayout *metaL = new QHBoxLayout(metaW);
    metaL->setContentsMargins(0,0,0,0);
    metaL->setSpacing(8);

    QLabel *catBadge = new QLabel(ticket.categoria);
    catBadge->setStyleSheet(R"(
        color: #818CF8; background-color: rgba(129,140,248,0.12);
        border-radius: 6px; padding: 2px 8px;
        font-size: 11px; font-weight: 600;
    )");

    QLabel *fechaL = new QLabel(ticket.fecha.toString("dd/MM/yyyy"));
    fechaL->setStyleSheet("color: #475569; font-size: 11px; background: transparent;");

    QString iaText = ticket.procesadoPorIA ? "✨ IA" : "✏️ Manual";
    QString iaStyle = ticket.procesadoPorIA
                          ? "color: #4ADE80; background-color: rgba(74,222,128,0.1); border-radius: 6px; padding: 2px 8px; font-size: 11px; font-weight: 600;"
                          : "color: #64748B; background-color: rgba(100,116,139,0.1); border-radius: 6px; padding: 2px 8px; font-size: 11px;";
    QLabel *iaLabel = new QLabel(iaText);
    iaLabel->setStyleSheet(iaStyle);

    metaL->addWidget(catBadge);
    metaL->addWidget(fechaL);
    metaL->addWidget(iaLabel);
    metaL->addStretch();

    infoL->addWidget(nameL);
    infoL->addWidget(metaW);

    QWidget *rightW = new QWidget();
    rightW->setStyleSheet("background: transparent;");
    QVBoxLayout *rightL = new QVBoxLayout(rightW);
    rightL->setContentsMargins(0,0,0,0);
    rightL->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rightL->setSpacing(6);

    QLabel *montoL = new QLabel(QString("$%1").arg(ticket.monto, 0, 'f', 2));
    montoL->setStyleSheet("color: #F1F5F9; font-size: 16px; font-weight: 700; background: transparent;");
    montoL->setAlignment(Qt::AlignRight);

    QPushButton *delBtn = new QPushButton("🗑");
    delBtn->setFixedSize(28, 28);
    delBtn->setCursor(Qt::PointingHandCursor);
    delBtn->setStyleSheet(R"(
        QPushButton {
            background: transparent; border: none;
            font-size: 14px;
        }
        QPushButton:hover { background-color: rgba(248,113,113,0.1); border-radius: 6px; }
    )");
    connect(delBtn, &QPushButton::clicked, this, [this, id = ticket.id]() {
        emit deleteRequested(id);
    });

    rightL->addWidget(montoL);
    rightL->addWidget(delBtn, 0, Qt::AlignRight);

    layout->addWidget(iconL);
    layout->addWidget(infoW, 1);
    layout->addWidget(rightW);
}

TicketsPage::TicketsPage(QWidget *parent) : QWidget(parent) {
    setStyleSheet("background: transparent;");
    setupUI();
    refreshData();
    connect(m_searchEdit, &QLineEdit::textChanged, this, &TicketsPage::applyFilter);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TicketsPage::applyFilter);
}

void TicketsPage::setupUI() {
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

    QLabel *title = new QLabel("🧾  Mis Tickets");
    title->setStyleSheet("color: #F1F5F9; font-size: 24px; font-weight: 700; background: transparent;");
    m_countLabel = new QLabel("0 registros");
    m_countLabel->setStyleSheet("color: #64748B; font-size: 13px; background: transparent;");
    titleL->addWidget(title);
    titleL->addWidget(m_countLabel);

    QPushButton *uploadBtn = new QPushButton("📷  Subir Ticket");
    uploadBtn->setFixedHeight(42);
    uploadBtn->setCursor(Qt::PointingHandCursor);
    uploadBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #4ADE80, stop:1 #22C55E);
            color: #0F1117;
            border: none;
            border-radius: 10px;
            padding: 0 20px;
            font-size: 13px;
            font-weight: 700;
        }
        QPushButton:hover { background: #22C55E; }
    )");
    connect(uploadBtn, &QPushButton::clicked, this, &TicketsPage::onUploadClicked);

    headerL->addWidget(titleW);
    headerL->addStretch();
    headerL->addWidget(uploadBtn);
    mainL->addWidget(headerW);

    // Filtros
    QWidget *filtersW = new QWidget();
    filtersW->setStyleSheet("background: transparent;");
    QHBoxLayout *filtersL = new QHBoxLayout(filtersW);
    filtersL->setContentsMargins(0,0,0,0);
    filtersL->setSpacing(12);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("🔍  Buscar ticket...");
    m_searchEdit->setFixedHeight(40);
    m_searchEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #1A1D27;
            color: #F1F5F9;
            border: 1px solid #2E3347;
            border-radius: 10px;
            padding: 0 14px;
            font-size: 13px;
        }
        QLineEdit:focus { border: 1px solid #4ADE80; }
    )");

    m_filterCombo = new QComboBox();
    m_filterCombo->addItem("Todas las categorías");
    for (auto &c : CATEGORIAS) m_filterCombo->addItem(c);
    m_filterCombo->setFixedHeight(40);
    m_filterCombo->setFixedWidth(200);
    m_filterCombo->setStyleSheet(R"(
        QComboBox {
            background-color: #1A1D27;
            color: #F1F5F9;
            border: 1px solid #2E3347;
            border-radius: 10px;
            padding: 0 12px;
            font-size: 13px;
        }
        QComboBox:focus { border: 1px solid #4ADE80; }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox QAbstractItemView {
            background-color: #1A1D27;
            color: #F1F5F9;
            border: 1px solid #2E3347;
            selection-background-color: #2E3347;
        }
    )");

    filtersL->addWidget(m_searchEdit, 1);
    filtersL->addWidget(m_filterCombo);
    mainL->addWidget(filtersW);

    // Lista
    m_ticketListWidget = new QWidget();
    m_ticketListWidget->setStyleSheet("background: transparent;");
    m_ticketListLayout = new QVBoxLayout(m_ticketListWidget);
    m_ticketListLayout->setContentsMargins(0,0,0,0);
    m_ticketListLayout->setSpacing(10);

    mainL->addWidget(m_ticketListWidget);
    mainL->addStretch();

    scroll->setWidget(content);
    QVBoxLayout *pageL = new QVBoxLayout(this);
    pageL->setContentsMargins(0,0,0,0);
    pageL->addWidget(scroll);
}

void TicketsPage::refreshData() {
    applyFilter();
}

void TicketsPage::applyFilter() {
    QString filter = m_filterCombo->currentText();
    QString search = m_searchEdit->text();
    QVector<Ticket> tickets = DataManager::instance().getTickets(filter, search);
    // Limpiar layout
    QLayoutItem *item;
    while ((item = m_ticketListLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    for (const Ticket& t : tickets) {
        addTicketCard(t);
    }
    m_countLabel->setText(QString("%1 registros").arg(tickets.size()));
}

void TicketsPage::addTicketCard(const Ticket &t) {
    auto *card = new TicketCard(t, this);
    connect(card, &TicketCard::deleteRequested, this, [this](int id) {
        if (DataManager::instance().removeTicket(id)) {
            refreshData();
        }
    });
    m_ticketListLayout->addWidget(card);
}

void TicketsPage::onUploadClicked() {
    UploadTicketDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Ticket t;
        t.id = 0;
        t.monto = dlg.monto();
        t.fecha = dlg.fecha();
        t.categoria = dlg.categoria();
        t.nombreLocal = dlg.nombreLocal().isEmpty() ? "Sin nombre" : dlg.nombreLocal();
        t.imagenPath = dlg.imagenPath();
        t.procesadoPorIA = !dlg.imagenPath().isEmpty();
        t.descripcion = "";
        if (DataManager::instance().addTicket(t)) {
            refreshData();
        }
    }
}