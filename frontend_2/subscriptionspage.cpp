#include "subscriptionspage.h"
#include "addsubscriptiondialog.h"
#include "datamanager.h"

#include <QScrollArea>
#include <QDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QDebug>

SubCard::SubCard(const Suscripcion &sub, QWidget *parent)
    : QFrame(parent)
{
    setFixedHeight(96);

    int diasRestantes = QDate::currentDate().daysTo(sub.fechaVencimiento);

    QString alertColor = "#2E3347";
    if (diasRestantes <= 3)  alertColor = "#F87171";
    else if (diasRestantes <= 7) alertColor = "#FBBF24";

    setStyleSheet(QString(R"(
        QFrame {
            background-color: #1A1D27;
            border: 1px solid %1;
            border-radius: 12px;
        }
        QFrame:hover {
            background-color: #1E2235;
        }
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
    infoL->setContentsMargins(0, 0, 0, 0);
    infoL->setSpacing(4);

    QWidget *nameRow = new QWidget();
    nameRow->setStyleSheet("background: transparent;");
    QHBoxLayout *nameRowL = new QHBoxLayout(nameRow);
    nameRowL->setContentsMargins(0, 0, 0, 0);
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
    if (diasRestantes < 0) {
        diasTxt = "¡Vencida!"; diasStyle = "color: #F87171;";
    } else if (diasRestantes == 0) {
        diasTxt = "Vence HOY"; diasStyle = "color: #F87171;";
    } else if (diasRestantes <= 7) {
        diasTxt = QString("Vence en %1 días").arg(diasRestantes);
        diasStyle = "color: #FBBF24;";
    } else {
        diasTxt = QString("Vence: %1").arg(sub.fechaVencimiento.toString("dd/MM/yyyy"));
        diasStyle = "color: #64748B;";
    }

    QLabel *diasL = new QLabel(diasTxt);
    diasL->setStyleSheet(diasStyle + "font-size: 12px; background: transparent; font-weight: 500;");

    QLabel *catL = new QLabel(sub.categoria);
    catL->setStyleSheet("color: #475569; font-size: 11px; background: transparent;");

    infoL->addWidget(nameRow);
    infoL->addWidget(diasL);
    infoL->addWidget(catL);

    // Panel derecho
    QWidget *rightW = new QWidget();
    rightW->setStyleSheet("background: transparent;");
    QVBoxLayout *rightL = new QVBoxLayout(rightW);
    rightL->setContentsMargins(0, 0, 0, 0);
    rightL->setSpacing(6);
    rightL->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *montoL = new QLabel(QString("$%1/mes").arg(sub.monto, 0, 'f', 0));
    montoL->setAlignment(Qt::AlignRight);
    montoL->setStyleSheet("color: #38BDF8; font-size: 15px; font-weight: 700; background: transparent;");

    QWidget *actionsW = new QWidget();
    actionsW->setStyleSheet("background: transparent;");
    QHBoxLayout *actL = new QHBoxLayout(actionsW);
    actL->setContentsMargins(0, 0, 0, 0);
    actL->setSpacing(4);

    QString btnBase = "QPushButton { background: transparent; border: none; font-size: 14px; }"
                      "QPushButton:hover { background-color: #21253A; border-radius: 6px; }";

    // Botón toggle (pausar/reanudar)
    QPushButton *toggleBtn = new QPushButton(sub.activa ? "⏸" : "▶");
    toggleBtn->setFixedSize(28, 28);
    toggleBtn->setCursor(Qt::PointingHandCursor);
    toggleBtn->setStyleSheet(btnBase);
    toggleBtn->setToolTip(sub.activa ? "Pausar suscripción" : "Reanudar suscripción");
    connect(toggleBtn, &QPushButton::clicked, this,
            [this, id = sub.id, activa = sub.activa]() {
                emit toggleRequested(id, !activa);
            });

    // Botón editar (más grande y con tooltip)
    QPushButton *editBtn = new QPushButton("✏");
    editBtn->setFixedSize(32, 32);  // Antes 28x28
    editBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setToolTip("Editar suscripción");
    editBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; font-size: 15px; }"
        "QPushButton:hover { background-color: rgba(56,189,248,0.12); border-radius: 8px; }"
        );
    connect(editBtn, &QPushButton::clicked, this,
            [this, id = sub.id]() {
                emit editRequested(id);
            });

    // Botón eliminar
    QPushButton *delBtn = new QPushButton("🗑");
    delBtn->setFixedSize(28, 28);
    delBtn->setCursor(Qt::PointingHandCursor);
    delBtn->setToolTip("Eliminar suscripción");
    delBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; font-size: 14px; }"
        "QPushButton:hover { background-color: rgba(248,113,113,0.1); border-radius: 6px; }"
        );
    connect(delBtn, &QPushButton::clicked, this,
            [this, id = sub.id]() {
                emit deleteRequested(id);
            });

    actL->addWidget(toggleBtn);
    actL->addWidget(editBtn);
    actL->addWidget(delBtn);

    rightL->addWidget(montoL);
    rightL->addWidget(actionsW, 0, Qt::AlignRight);

    layout->addWidget(iconL);
    layout->addWidget(infoW, 1);
    layout->addWidget(rightW);
}

SubscriptionsPage::SubscriptionsPage(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background: transparent;");
    setupUI();
    refreshData();

    connect(&DataManager::instance(), &DataManager::suscripcionesChanged,
            this, &SubscriptionsPage::refreshData);
    connect(&DataManager::instance(), &DataManager::sincronizacionCompletada,
            this, &SubscriptionsPage::refreshData);
}

void SubscriptionsPage::setupUI()
{
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
    headerL->setContentsMargins(0, 0, 0, 0);

    QWidget *titleW = new QWidget();
    titleW->setStyleSheet("background: transparent;");
    QVBoxLayout *titleL = new QVBoxLayout(titleW);
    titleL->setContentsMargins(0, 0, 0, 0);
    titleL->setSpacing(2);

    QLabel *title = new QLabel("🔄  Suscripciones");
    title->setStyleSheet("color: #F1F5F9; font-size: 24px; font-weight: 700; background: transparent;");

    QWidget *statsRow = new QWidget();
    statsRow->setStyleSheet("background: transparent;");
    QHBoxLayout *statsL = new QHBoxLayout(statsRow);
    statsL->setContentsMargins(0, 0, 0, 0);
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
    connect(addBtn, &QPushButton::clicked, this, &SubscriptionsPage::onAddClicked);

    headerL->addWidget(titleW);
    headerL->addStretch();
    headerL->addWidget(addBtn);

    mainL->addWidget(headerW);

    // Lista
    m_subListWidget = new QWidget();
    m_subListWidget->setStyleSheet("background: transparent;");
    m_subListLayout = new QVBoxLayout(m_subListWidget);
    m_subListLayout->setContentsMargins(0, 0, 0, 0);
    m_subListLayout->setSpacing(10);

    mainL->addWidget(m_subListWidget);
    mainL->addStretch();

    scroll->setWidget(content);

    QVBoxLayout *pageL = new QVBoxLayout(this);
    pageL->setContentsMargins(0, 0, 0, 0);
    pageL->addWidget(scroll);
}

void SubscriptionsPage::refreshData()
{
    while (QLayoutItem *item = m_subListLayout->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    const QVector<Suscripcion> subs = DataManager::instance().getSuscripciones();

    double total = 0;
    for (const Suscripcion &s : subs) {
        if (s.activa) total += s.monto;
        addSubCard(s);
    }

    m_countLabel->setText(QString("%1 suscripciones").arg(subs.size()));
    m_totalLabel->setText(QString("Total activas: $%1/mes").arg(total, 0, 'f', 0));
}

void SubscriptionsPage::addSubCard(const Suscripcion &s)
{
    auto *card = new SubCard(s, this);

    connect(card, &SubCard::toggleRequested, this,
            [](int id, bool activa) {
                DataManager::instance().updateSuscripcionEstado(id, activa);
            });

    connect(card, &SubCard::deleteRequested, this,
            [](int id) {
                DataManager::instance().removeSuscripcion(id);
            });

    connect(card, &SubCard::editRequested, this, &SubscriptionsPage::onEditClicked);

    m_subListLayout->addWidget(card);
}

void SubscriptionsPage::onAddClicked()
{
    AddSubscriptionDialog dlg(this);
    dlg.exec();
}

void SubscriptionsPage::onEditClicked(int id)
{
    const QVector<Suscripcion> subs = DataManager::instance().getSuscripciones();
    Suscripcion actual;
    bool encontrada = false;
    for (const Suscripcion &s : subs) {
        if (s.id == id) { actual = s; encontrada = true; break; }
    }
    if (!encontrada) return;

    QDialog dlg(this);
    dlg.setWindowTitle("Editar Suscripción");
    dlg.setModal(true);
    dlg.setFixedSize(480, 460);
    dlg.setStyleSheet(R"(
        QDialog { background-color: #0F1117; border: 1px solid #2E3347; border-radius: 16px; }
        QLabel  { background: transparent; }
    )");

    QString inputStyle = R"(
        QLineEdit, QComboBox, QDoubleSpinBox, QSpinBox, QDateEdit {
            background-color: #21253A; color: #F1F5F9;
            border: 1px solid #2E3347; border-radius: 8px;
            padding: 9px 12px; font-size: 13px;
        }
        QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus,
        QSpinBox:focus, QDateEdit:focus { border: 1px solid #38BDF8; }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox QAbstractItemView {
            background-color: #21253A; color: #F1F5F9;
            border: 1px solid #2E3347; selection-background-color: #2E3347;
        }
        QDoubleSpinBox::up-button, QDoubleSpinBox::down-button,
        QSpinBox::up-button, QSpinBox::down-button {
            background: #2E3347; border: none; border-radius: 3px;
        }
        QDateEdit::drop-down { border: none; width: 24px; }
    )";

    auto makeLabel = [](const QString &txt) {
        QLabel *l = new QLabel(txt);
        l->setStyleSheet("color: #64748B; font-size: 12px; margin-bottom: 2px;");
        return l;
    };

    QVBoxLayout *mainL = new QVBoxLayout(&dlg);
    mainL->setContentsMargins(28, 24, 28, 24);
    mainL->setSpacing(20);

    QLabel *titleLbl = new QLabel("✏  Editar Suscripción");
    titleLbl->setStyleSheet("color: #F1F5F9; font-size: 20px; font-weight: 700;");
    QLabel *subLbl = new QLabel("Modificá los datos del servicio");
    subLbl->setStyleSheet("color: #64748B; font-size: 12px;");
    mainL->addWidget(titleLbl);
    mainL->addWidget(subLbl);

    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color: #2E3347; border: none;");
    mainL->addWidget(sep);

    QWidget *formW = new QWidget();
    formW->setStyleSheet("background: transparent;");
    QGridLayout *formL = new QGridLayout(formW);
    formL->setContentsMargins(0, 0, 0, 0);
    formL->setSpacing(10);
    formL->setVerticalSpacing(6);

    auto *nombreEdit = new QLineEdit(actual.nombreServicio);
    nombreEdit->setStyleSheet(inputStyle);

    auto *montoSpin = new QDoubleSpinBox();
    montoSpin->setPrefix("$ ");
    montoSpin->setMaximum(9999999.99);
    montoSpin->setDecimals(2);
    montoSpin->setValue(actual.monto);
    montoSpin->setStyleSheet(inputStyle);

    auto *fechaEdit = new QDateEdit(actual.fechaVencimiento);
    fechaEdit->setCalendarPopup(true);
    fechaEdit->setStyleSheet(inputStyle);

    auto *diasSpin = new QSpinBox();
    diasSpin->setRange(1, 30);
    diasSpin->setValue(actual.diasAviso);
    diasSpin->setSuffix(" días antes");
    diasSpin->setStyleSheet(inputStyle);

    auto *catCombo = new QComboBox();
    for (auto &c : CATEGORIAS) catCombo->addItem(c);
    catCombo->setCurrentText(actual.categoria);
    catCombo->setStyleSheet(inputStyle);

    formL->addWidget(makeLabel("Nombre del Servicio"), 0, 0, 1, 2);
    formL->addWidget(nombreEdit,                       1, 0, 1, 2);
    formL->addWidget(makeLabel("Monto Mensual"),        2, 0);
    formL->addWidget(makeLabel("Categoría"),            2, 1);
    formL->addWidget(montoSpin,                        3, 0);
    formL->addWidget(catCombo,                         3, 1);
    formL->addWidget(makeLabel("Próx. Vencimiento"),    4, 0);
    formL->addWidget(makeLabel("Avisar con"),           4, 1);
    formL->addWidget(fechaEdit,                        5, 0);
    formL->addWidget(diasSpin,                         5, 1);
    mainL->addWidget(formW);

    mainL->addStretch();

    QWidget *btnW = new QWidget();
    btnW->setStyleSheet("background: transparent;");
    QHBoxLayout *btnL = new QHBoxLayout(btnW);
    btnL->setContentsMargins(0, 0, 0, 0);
    btnL->setSpacing(12);

    QPushButton *cancelBtn = new QPushButton("Cancelar");
    cancelBtn->setFixedHeight(40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: transparent; color: #64748B; border: 1px solid #2E3347;
                      border-radius: 8px; font-size: 13px; }
        QPushButton:hover { color: #94A3B8; border-color: #475569; }
    )");
    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    QPushButton *saveBtn = new QPushButton("💾  Guardar cambios");
    saveBtn->setFixedHeight(40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(R"(
        QPushButton { background-color: #38BDF8; color: #0F1117; border: none;
                      border-radius: 8px; font-size: 13px; font-weight: 700; }
        QPushButton:hover { background-color: #0EA5E9; }
    )");
    QObject::connect(saveBtn, &QPushButton::clicked, &dlg, [&]() {
        if (nombreEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, "Atención", "El nombre no puede estar vacío.");
            return;
        }
        dlg.accept();
    });

    btnL->addWidget(cancelBtn, 1);
    btnL->addWidget(saveBtn, 2);
    mainL->addWidget(btnW);

    if (dlg.exec() != QDialog::Accepted) return;

    Suscripcion editada  = actual;
    editada.nombreServicio  = nombreEdit->text().trimmed();
    editada.monto           = montoSpin->value();
    editada.fechaVencimiento = fechaEdit->date();
    editada.diasAviso       = diasSpin->value();
    editada.categoria       = catCombo->currentText();
    editada.iconoNombre     = editada.nombreServicio.toLower().simplified();

    if (!DataManager::instance().updateSuscripcion(editada)) {
        QMessageBox::critical(this, "Error", "No se pudo guardar el cambio.");
        return;
    }
}
