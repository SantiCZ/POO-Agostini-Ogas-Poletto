#include "reportspage.h"
#include "datamanager.h"
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>
#include <QTimer>
#include <QLinearGradient>

CategoryBar::CategoryBar(const QString &icon, const QString &categoria,
                         double monto, double maximo, const QString &color,
                         QWidget *parent) : QWidget(parent)
{
    setFixedHeight(52);
    setStyleSheet("background: transparent;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(12);

    QLabel *iconL = new QLabel(icon);
    iconL->setFixedSize(36, 36);
    iconL->setAlignment(Qt::AlignCenter);
    iconL->setStyleSheet("background-color: #21253A; border-radius: 8px; font-size: 16px;");

    QWidget *infoW = new QWidget();
    infoW->setStyleSheet("background: transparent;");
    QVBoxLayout *infoL = new QVBoxLayout(infoW);
    infoL->setContentsMargins(0,0,0,0);
    infoL->setSpacing(5);

    QWidget *labelRow = new QWidget();
    labelRow->setStyleSheet("background: transparent;");
    QHBoxLayout *labelL = new QHBoxLayout(labelRow);
    labelL->setContentsMargins(0,0,0,0);

    QLabel *catL = new QLabel(categoria);
    catL->setStyleSheet("color: #E2E8F0; font-size: 12px; font-weight: 600; background: transparent;");
    QLabel *montoL = new QLabel(QString("$%1").arg(monto, 0, 'f', 0));
    montoL->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 700; background: transparent;").arg(color));

    labelL->addWidget(catL);
    labelL->addStretch();
    labelL->addWidget(montoL);

    QFrame *barBg = new QFrame();
    barBg->setFixedHeight(6);
    barBg->setStyleSheet("background-color: #21253A; border-radius: 3px;");

    int pct = (int)((monto / maximo) * 100.0);
    if (pct > 100) pct = 100;

    QFrame *barFill = new QFrame(barBg);
    barFill->setFixedHeight(6);
    barFill->setStyleSheet(QString("background-color: %1; border-radius: 3px;").arg(color));
    barFill->setFixedWidth(1);

    barBg->setProperty("fillPercent", pct);
    barBg->setProperty("fillColor", color);
    barBg->setProperty("fillWidget", QVariant::fromValue<QWidget*>(barFill));

    infoL->addWidget(labelRow);
    infoL->addWidget(barBg);

    layout->addWidget(iconL);
    layout->addWidget(infoW, 1);

    QTimer::singleShot(50, this, [barBg, barFill, pct]() {
        int w = (int)(barBg->width() * pct / 100.0);
        barFill->setFixedWidth(qMax(w, 6));
    });
}

BarChart::BarChart(QWidget *parent) : QWidget(parent) {
    setFixedHeight(200);
    setStyleSheet("background: transparent;");
}

void BarChart::setData(const QVector<QPair<QString,double>> &data, double maxVal) {
    m_data = data;
    m_maxVal = maxVal;
    update();
}

void BarChart::paintEvent(QPaintEvent *) {
    if (m_data.isEmpty()) return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int w = width();
    int h = height();
    int pad = 16;
    int barW = (w - pad * 2) / m_data.size() - 8;
    if (barW < 10) barW = 10;

    QStringList colors = {"#4ADE80","#818CF8","#38BDF8","#FBBF24","#FB923C","#F472B6"};

    for (int i = 0; i < m_data.size(); ++i) {
        double ratio = (m_maxVal > 0) ? (m_data[i].second / m_maxVal) : 0;
        int bh = (int)((h - 40) * ratio);
        int x  = pad + i * ((w - pad * 2) / m_data.size()) + 4;
        int y  = h - 30 - bh;

        QLinearGradient grad(x, y, x, y + bh);
        QColor c(colors[i % colors.size()]);
        grad.setColorAt(0, c);
        grad.setColorAt(1, c.darker(150));
        p.setBrush(grad);
        p.setPen(Qt::NoPen);
        QPainterPath path;
        path.addRoundedRect(x, y, barW, bh, 4, 4);
        p.drawPath(path);

        p.setPen(QColor(colors[i % colors.size()]));
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));
        p.drawText(x, y - 4, barW, 16, Qt::AlignCenter,
                   QString("$%1k").arg((int)(m_data[i].second / 1000)));

        p.setPen(QColor("#64748B"));
        p.setFont(QFont("Segoe UI", 8));
        p.drawText(x, h - 26, barW, 20, Qt::AlignCenter, m_data[i].first);
    }
}

ReportsPage::ReportsPage(QWidget *parent) : QWidget(parent) {
    setStyleSheet("background: transparent;");
    setupUI();
    refreshData();
}

void ReportsPage::setupUI() {
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *content = new QWidget();
    content->setStyleSheet("background: transparent;");
    QVBoxLayout *mainL = new QVBoxLayout(content);
    mainL->setContentsMargins(32, 28, 32, 32);
    mainL->setSpacing(24);

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

    QLabel *title = new QLabel("📊  Reportes");
    title->setStyleSheet("color: #F1F5F9; font-size: 24px; font-weight: 700; background: transparent;");
    QLabel *sub = new QLabel("Resumen financiero");
    sub->setStyleSheet("color: #64748B; font-size: 13px; background: transparent;");
    titleL->addWidget(title);
    titleL->addWidget(sub);

    QPushButton *exportBtn = new QPushButton("📥  Exportar");
    exportBtn->setFixedHeight(38);
    exportBtn->setCursor(Qt::PointingHandCursor);
    exportBtn->setStyleSheet(R"(
        QPushButton {
            background: transparent; color: #94A3B8;
            border: 1px solid #2E3347; border-radius: 8px;
            padding: 0 16px; font-size: 12px;
        }
        QPushButton:hover { border-color: #4ADE80; color: #4ADE80; }
    )");

    headerL->addWidget(titleW);
    headerL->addStretch();
    headerL->addWidget(exportBtn);
    mainL->addWidget(headerW);

    // Summary cards
    QWidget *cardsW = new QWidget();
    cardsW->setStyleSheet("background: transparent;");
    m_summaryLayout = new QHBoxLayout(cardsW);
    m_summaryLayout->setContentsMargins(0,0,0,0);
    m_summaryLayout->setSpacing(14);
    mainL->addWidget(cardsW);

    // Mid section
    QWidget *midW = new QWidget();
    midW->setStyleSheet("background: transparent;");
    QHBoxLayout *midL = new QHBoxLayout(midW);
    midL->setContentsMargins(0,0,0,0);
    midL->setSpacing(20);

    QWidget *chartW = new QWidget();
    chartW->setStyleSheet("background: transparent;");
    m_chartLayout = new QVBoxLayout(chartW);
    m_chartLayout->setContentsMargins(0,0,0,0);
    m_chartLayout->setSpacing(12);

    QWidget *breakW = new QWidget();
    breakW->setStyleSheet("background: transparent;");
    breakW->setFixedWidth(300);
    m_breakdownLayout = new QVBoxLayout(breakW);
    m_breakdownLayout->setContentsMargins(0,0,0,0);
    m_breakdownLayout->setSpacing(12);

    midL->addWidget(chartW, 1);
    midL->addWidget(breakW);
    mainL->addWidget(midW);
    mainL->addStretch();

    scroll->setWidget(content);
    QVBoxLayout *pageL = new QVBoxLayout(this);
    pageL->setContentsMargins(0,0,0,0);
    pageL->addWidget(scroll);
}

void ReportsPage::clearLayout(QLayout *layout) {
    if (!layout) return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
}

void ReportsPage::refreshData() {
    clearLayout(m_summaryLayout);
    clearLayout(m_chartLayout);
    clearLayout(m_breakdownLayout);

    QDate now = QDate::currentDate();
    int year = now.year();
    int month = now.month();
    double gastoMes = DataManager::instance().getGastoMes(year, month);
    double gastoMesAnterior = DataManager::instance().getGastoMes(year, month-1);
    int ticketCount = DataManager::instance().getTicketCountMes(year, month);
    double promedio = (ticketCount > 0) ? gastoMes / ticketCount : 0;

    // Summary cards
    struct Card { QString icon, label, val, sub, color; };
    QVector<Card> cards = {
                           {"💸", "TOTAL GASTADO",  QString("$%1").arg(gastoMes, 0, 'f', 0), "este mes",    "#4ADE80"},
                           {"📈", "MES ANTERIOR",   QString("$%1").arg(gastoMesAnterior, 0, 'f', 0), "",     "#F472B6"},
                           {"🧾", "TICKETS",        QString::number(ticketCount), "registrados",  "#818CF8"},
                           {"⌀",  "GASTO PROMEDIO", QString("$%1").arg(promedio, 0, 'f', 2), "por ticket",   "#38BDF8"},
                           };
    for (auto &c : cards) {
        QFrame *card = new QFrame();
        card->setStyleSheet("QFrame { background-color: #1A1D27; border: 1px solid #2E3347; border-radius: 12px; }");
        card->setFixedHeight(90);
        QVBoxLayout *l = new QVBoxLayout(card);
        l->setContentsMargins(16, 12, 16, 12);
        l->setSpacing(3);
        QLabel *iconL = new QLabel(c.icon + "  " + c.label);
        iconL->setStyleSheet("color: #64748B; font-size: 11px; font-weight: 600; background: transparent;");
        QLabel *valL = new QLabel(c.val);
        valL->setStyleSheet(QString("color: %1; font-size: 20px; font-weight: 700; background: transparent;").arg(c.color));
        QLabel *subL = new QLabel(c.sub);
        subL->setStyleSheet("color: #475569; font-size: 11px; background: transparent;");
        l->addWidget(iconL);
        l->addWidget(valL);
        l->addWidget(subL);
        m_summaryLayout->addWidget(card);
    }

    // Bar chart (gastos por semana)
    auto semanas = DataManager::instance().getGastosPorSemana(year, month);
    double maxSem = 0;
    for (auto &p : semanas) if (p.second > maxSem) maxSem = p.second;
    maxSem = qMax(maxSem, 1.0);
    QLabel *chartTitle = new QLabel("Gastos por Semana");
    chartTitle->setStyleSheet("color: #F1F5F9; font-size: 16px; font-weight: 700; background: transparent;");
    m_chartLayout->addWidget(chartTitle);
    QFrame *card = new QFrame();
    card->setStyleSheet("QFrame { background-color: #1A1D27; border: 1px solid #2E3347; border-radius: 14px; }");
    QVBoxLayout *cardL = new QVBoxLayout(card);
    cardL->setContentsMargins(20, 16, 20, 16);
    BarChart *chart = new BarChart();
    chart->setData(semanas, maxSem);
    cardL->addWidget(chart);
    m_chartLayout->addWidget(card);

    // Category breakdown
    QLabel *breakTitle = new QLabel("Por Categoría");
    breakTitle->setStyleSheet("color: #F1F5F9; font-size: 16px; font-weight: 700; background: transparent;");
    m_breakdownLayout->addWidget(breakTitle);
    QFrame *breakCard = new QFrame();
    breakCard->setStyleSheet("QFrame { background-color: #1A1D27; border: 1px solid #2E3347; border-radius: 14px; }");
    QVBoxLayout *breakCardL = new QVBoxLayout(breakCard);
    breakCardL->setContentsMargins(16, 16, 16, 16);
    breakCardL->setSpacing(6);

    auto cats = DataManager::instance().getGastosPorCategoria(year, month);
    double maxCat = 0;
    for (auto &p : cats) if (p.second > maxCat) maxCat = p.second;
    maxCat = qMax(maxCat, 1.0);

    QMap<QString,QString> icons = {
        {"Supermercado","🛒"},{"Restaurante","🍕"},{"Transporte","🚌"},
        {"Salud","💊"},{"Entretenimiento","🎬"},{"Servicios","⚡"},
        {"Ropa","👕"},{"Tecnología","💻"},{"Otro","📋"}
    };
    QVector<std::tuple<QString,QString,double,QString>> ordered;
    for (auto &p : cats) {
        QString icon = icons.value(p.first, "📋");
        QString color = "#38BDF8";
        if (p.first == "Supermercado") color = "#4ADE80";
        else if (p.first == "Servicios") color = "#38BDF8";
        else if (p.first == "Transporte") color = "#818CF8";
        else if (p.first == "Restaurante") color = "#FBBF24";
        else if (p.first == "Salud") color = "#F472B6";
        else color = "#FB923C";
        ordered.append({icon, p.first, p.second, color});
    }
    std::sort(ordered.begin(), ordered.end(), [](const auto &a, const auto &b) {
        return std::get<2>(a) > std::get<2>(b);
    });
    for (auto &[icon, cat, monto, color] : ordered) {
        breakCardL->addWidget(new CategoryBar(icon, cat, monto, maxCat, color));
    }
    if (ordered.isEmpty()) {
        QLabel *empty = new QLabel("Sin datos de gastos");
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color: #64748B; padding: 20px;");
        breakCardL->addWidget(empty);
    }
    m_breakdownLayout->addWidget(breakCard);
    m_breakdownLayout->addStretch();
}