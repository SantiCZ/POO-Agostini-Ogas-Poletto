#include "ticketspage.h"

#include "uploadticketdialog.h"
#include "stylemanager.h"
#include "datamanager.h"

#include <QGraphicsDropShadowEffect>
#include <QMessageBox>

TicketCard::TicketCard(
    const Ticket &ticket,
    QWidget *parent
    ) : QFrame(parent)
{
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

    QHBoxLayout *layout =
        new QHBoxLayout(this);

    layout->setContentsMargins(16,0,16,0);

    QMap<QString, QString> catIcons =
        {
            {"Supermercado","🛒"},
            {"Restaurante","🍕"},
            {"Transporte","🚌"},
            {"Salud","💊"},
            {"Entretenimiento","🎬"},
            {"Servicios","⚡"},
            {"Ropa","👕"},
            {"Tecnología","💻"},
            {"Otro","📋"}
        };

    QString icon =
        catIcons.value(
            ticket.categoria,
            "📋"
            );

    QLabel *iconL =
        new QLabel(icon);

    iconL->setFixedSize(48,48);

    iconL->setAlignment(Qt::AlignCenter);

    iconL->setStyleSheet(
        "background-color: #21253A;"
        "border-radius: 12px;"
        "font-size: 22px;"
        );

    QLabel *nameL =
        new QLabel(ticket.nombreLocal);

    nameL->setStyleSheet(
        "color: #F1F5F9;"
        "font-size: 14px;"
        "font-weight: 600;"
        );

    QLabel *catL =
        new QLabel(ticket.categoria);

    catL->setStyleSheet(
        "color: #818CF8;"
        "font-size: 11px;"
        );

    QLabel *fechaL =
        new QLabel(
            ticket.fecha.toString(
                "dd/MM/yyyy"
                )
            );

    fechaL->setStyleSheet(
        "color: #64748B;"
        "font-size: 11px;"
        );

    QLabel *montoL =
        new QLabel(
            QString("$%1")
                .arg(ticket.monto,0,'f',2)
            );

    montoL->setStyleSheet(
        "color: #F1F5F9;"
        "font-size: 16px;"
        "font-weight: 700;"
        );

    QPushButton *delBtn =
        new QPushButton("🗑");

    delBtn->setFixedSize(28,28);

    connect(
        delBtn,
        &QPushButton::clicked,
        this,
        [this, ticket]()
        {
            emit deleteRequested(ticket.id);
        }
        );

    QVBoxLayout *infoL =
        new QVBoxLayout();

    infoL->addWidget(nameL);
    infoL->addWidget(catL);
    infoL->addWidget(fechaL);

    QVBoxLayout *rightL =
        new QVBoxLayout();

    rightL->addWidget(montoL);
    rightL->addWidget(delBtn);

    layout->addWidget(iconL);
    layout->addLayout(infoL);
    layout->addStretch();
    layout->addLayout(rightL);
}

TicketsPage::TicketsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();

    refreshData();

    connect(
        m_searchEdit,
        &QLineEdit::textChanged,
        this,
        &TicketsPage::applyFilter
        );

    connect(
        m_filterCombo,
        QOverload<int>::of(
            &QComboBox::currentIndexChanged
            ),
        this,
        &TicketsPage::applyFilter
        );

    connect(
        &DataManager::instance(),
        &DataManager::ticketsChanged,
        this,
        &TicketsPage::refreshData
        );
}

void TicketsPage::setupUI()
{
    QVBoxLayout *mainL =
        new QVBoxLayout(this);

    m_searchEdit =
        new QLineEdit();

    m_filterCombo =
        new QComboBox();

    m_filterCombo->addItem(
        "Todas las categorías"
        );

    for (const QString &c : CATEGORIAS)
    {
        m_filterCombo->addItem(c);
    }

    m_countLabel =
        new QLabel("0 registros");

    m_ticketListWidget =
        new QWidget();

    m_ticketListLayout =
        new QVBoxLayout(
            m_ticketListWidget
            );

    mainL->addWidget(m_searchEdit);
    mainL->addWidget(m_filterCombo);
    mainL->addWidget(m_countLabel);
    mainL->addWidget(m_ticketListWidget);
}

void TicketsPage::refreshData()
{
    applyFilter();
}

void TicketsPage::applyFilter()
{
    QString categoria =
        m_filterCombo->currentText();

    QString busqueda =
        m_searchEdit->text();

    const QVector<Ticket> tickets =
        DataManager::instance()
            .getTickets(
                categoria,
                busqueda
                );

    QLayoutItem *item;

    while (
        (item =
         m_ticketListLayout->takeAt(0))
        != nullptr
        )
    {
        delete item->widget();
        delete item;
    }

    for (const Ticket &t : tickets)
    {
        addTicketCard(t);
    }

    m_countLabel->setText(
        QString("%1 registros")
            .arg(tickets.size())
        );
}

void TicketsPage::addTicketCard(
    const Ticket &t
    )
{
    TicketCard *card =
        new TicketCard(t);

    connect(
        card,
        &TicketCard::deleteRequested,
        this,
        [this](int id)
        {
            if (
                !DataManager::instance()
                     .removeTicket(id)
                )
            {
                QMessageBox::warning(
                    this,
                    "Error",
                    "No se pudo eliminar"
                    );

                return;
            }

            refreshData();
        }
        );

    m_ticketListLayout
        ->addWidget(card);
}

void TicketsPage::onUploadClicked()
{
    UploadTicketDialog dlg(this);

    if (
        dlg.exec()
        == QDialog::Accepted
        )
    {
        refreshData();
    }
}
