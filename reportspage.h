#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>

class CategoryBar : public QWidget {
    Q_OBJECT
public:
    CategoryBar(const QString &icon, const QString &categoria,
                double monto, double maximo, const QString &color,
                QWidget *parent = nullptr);
};

class BarChart : public QWidget {
    Q_OBJECT
public:
    explicit BarChart(QWidget *parent = nullptr);
    void setData(const QVector<QPair<QString,double>> &data, double maxVal);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QPair<QString,double>> m_data;
    double m_maxVal = 1.0;
};

class ReportsPage : public QWidget {
    Q_OBJECT
public:
    explicit ReportsPage(QWidget *parent = nullptr);
    void refreshData();

private:
    void setupUI();
    void buildSummaryCards(QHBoxLayout *layout);
    void buildBarChart(QVBoxLayout *layout);
    void buildCategoryBreakdown(QVBoxLayout *layout);
    void clearLayout(QLayout *layout);

    QHBoxLayout *m_summaryLayout = nullptr;
    QVBoxLayout *m_chartLayout = nullptr;
    QVBoxLayout *m_breakdownLayout = nullptr;
};