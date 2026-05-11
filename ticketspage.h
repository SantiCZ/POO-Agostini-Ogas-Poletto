#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QFrame>
#include "models.h"

class TicketCard : public QFrame {
    Q_OBJECT
public:
    TicketCard(const Ticket &ticket, QWidget *parent = nullptr);

signals:
    void deleteRequested(int id);
};

class TicketsPage : public QWidget {
    Q_OBJECT
public:
    explicit TicketsPage(QWidget *parent = nullptr);
    void refreshData();

public slots:
    void onUploadClicked();

private:
    void setupUI();
    void addTicketCard(const Ticket &t);
    void applyFilter();

    QVBoxLayout  *m_ticketListLayout = nullptr;
    QWidget      *m_ticketListWidget = nullptr;
    QLabel       *m_countLabel       = nullptr;
    QLineEdit    *m_searchEdit       = nullptr;
    QComboBox    *m_filterCombo      = nullptr;
};