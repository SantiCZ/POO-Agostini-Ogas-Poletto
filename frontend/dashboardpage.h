#pragma once
#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>

class StatCard : public QFrame {
    Q_OBJECT
public:
    StatCard(const QString &icon, const QString &title,
             const QString &value, const QString &subtitle,
             const QString &accentColor = "#4ADE80",
             QWidget *parent = nullptr);
};

class RecentExpenseRow : public QWidget {
    Q_OBJECT
public:
    RecentExpenseRow(const QString &icon, const QString &nombre,
                     const QString &categoria, const QString &fecha,
                     double monto, QWidget *parent = nullptr);
};

class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(QWidget *parent = nullptr);
    void refreshData();

signals:
    void navigateToTickets();
    void navigateToSubs();

private:
    void setupUI();
    void buildStatsRow(QHBoxLayout *layout);
    void buildRecentActivity(QVBoxLayout *layout);
    void buildQuickActions(QVBoxLayout *layout);
    void buildSubsAlert(QVBoxLayout *layout);
    void clearLayout(QLayout *layout);

    QHBoxLayout *m_statsLayout = nullptr;
    QVBoxLayout *m_activityLayout = nullptr;
    QVBoxLayout *m_alertLayout = nullptr;
    QWidget *m_statsWidget = nullptr;
    QWidget *m_activityWidget = nullptr;
    QWidget *m_alertWidget = nullptr;
};