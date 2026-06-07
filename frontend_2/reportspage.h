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

/*
 * CategoryBar
 * Barra horizontal que representa el gasto de una categoria respecto al maximo.
 */
class CategoryBar : public QWidget {
    Q_OBJECT
public:
    CategoryBar(const QString &icon, const QString &categoria,
                double monto, double maximo, const QString &color,
                QWidget *parent = nullptr);
};

/*
 * BarChart
 * Responsabilidad de clase:
 * Grafico simple dibujado con QPainter para comparar gastos por semana.
 *
 * Herencia:
 * Hereda de QWidget para poder participar en layouts Qt y redibujarse.
 *
 * Polimorfismo y paintEvent:
 * Sobrescribe paintEvent(), metodo virtual de QWidget. Qt lo llama cuando el
 * widget necesita repintarse; esta clase aprovecha ese polimorfismo para
 * dibujar barras propias en lugar de usar un componente grafico externo.
 */
class BarChart : public QWidget {
    Q_OBJECT
public:
    explicit BarChart(QWidget *parent = nullptr);

    // Recibe los datos agregados y el valor maximo para escalar las barras.
    void setData(const QVector<QPair<QString,double>> &data, double maxVal);

protected:
    // paintEvent: renderiza el grafico manualmente dentro del widget.
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QPair<QString,double>> m_data;
    double m_maxVal = 1.0;
};

/*
 * ReportsPage
 * Pantalla de reportes mensuales.
 * Combina tarjetas resumen, grafico semanal y desglose por categorias.
 */
class ReportsPage : public QWidget {
    Q_OBJECT
public:
    explicit ReportsPage(QWidget *parent = nullptr);

    // Recarga las estadisticas visibles desde DataManager.
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
