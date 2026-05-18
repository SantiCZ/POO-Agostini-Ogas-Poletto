#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include "models.h"

class SubCard : public QFrame {
    Q_OBJECT
public:
    SubCard(const Suscripcion &sub, QWidget *parent = nullptr);

signals:
    void toggleRequested(int id, bool activa);
    void deleteRequested(int id);
};

class SubscriptionsPage : public QWidget {
    Q_OBJECT
public:
    explicit SubscriptionsPage(QWidget *parent = nullptr);
    void refreshData();

public slots:
    void onAddClicked();

private:
    void setupUI();
    void addSubCard(const Suscripcion &s);

    QVBoxLayout  *m_subListLayout = nullptr;
    QWidget      *m_subListWidget = nullptr;
    QLabel       *m_totalLabel    = nullptr;
    QLabel       *m_countLabel    = nullptr;
};