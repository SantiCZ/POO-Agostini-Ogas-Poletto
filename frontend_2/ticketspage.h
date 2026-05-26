#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QFrame>

#include "models.h"

class TicketCard : public QFrame {
    Q_OBJECT

public:
    explicit TicketCard(
        const Ticket &ticket,
        QWidget *parent = nullptr
        );

signals:
    void deleteRequested(int idRemoto);
};

class TicketsPage : public QWidget {
    Q_OBJECT

public:
    explicit TicketsPage(QWidget *parent = nullptr);

    void refreshData();

public slots:
    void onUploadClicked();

private slots:
    void applyFilter();

private:
    void setupUI();
    void addTicketCard(const Ticket &t);

private:
    QVBoxLayout *m_ticketListLayout = nullptr;
    QWidget *m_ticketListWidget = nullptr;

    QLabel *m_countLabel = nullptr;

    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_filterCombo = nullptr;
};