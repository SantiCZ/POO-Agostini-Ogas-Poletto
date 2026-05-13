#include "addsubscriptiondialog.h"
#include "models.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
//
AddSubscriptionDialog::AddSubscriptionDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Nueva Suscripción");
    setModal(true);
    setFixedSize(480, 500);
    setStyleSheet("QDialog { background-color: #0F1117; border: 1px solid #2E3347; border-radius: 16px; }");
    setupUI();
}

void AddSubscriptionDialog::setupUI() {
    QVBoxLayout *mainL = new QVBoxLayout(this);
    mainL->setContentsMargins(28, 24, 28, 24);
    mainL->setSpacing(20);

    // Título
    QLabel *title = new QLabel("🔄  Nueva Suscripción");
    title->setStyleSheet("color: #F1F5F9; font-size: 20px; font-weight: 700; background: transparent;");
    QLabel *sub = new QLabel("Registrá un servicio recurrente y recibí recordatorios");
    sub->setStyleSheet("color: #64748B; font-size: 12px; background: transparent;");
    mainL->addWidget(title);
    mainL->addWidget(sub);

    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color: #2E3347; border: none;");
    mainL->addWidget(sep);

    QString inputStyle = R"(
        QLineEdit, QComboBox, QDoubleSpinBox, QSpinBox, QDateEdit {
            background-color: #21253A;
            color: #F1F5F9;
            border: 1px solid #2E3347;
            border-radius: 8px;
            padding: 9px 12px;
            font-size: 13px;
        }
        QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QSpinBox:focus, QDateEdit:focus {
            border: 1px solid #38BDF8;
        }
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
        l->setStyleSheet("color: #64748B; font-size: 12px; background: transparent; margin-bottom: 2px;");
        return l;
    };

    // Formulario
    QWidget *formW = new QWidget();
    formW->setStyleSheet("background: transparent;");
    QGridLayout *formL = new QGridLayout(formW);
    formL->setContentsMargins(0,0,0,0);
    formL->setSpacing(10);
    formL->setVerticalSpacing(6);

    m_nombreEdit = new QLineEdit();
    m_nombreEdit->setPlaceholderText("ej: Netflix, Spotify, Gimnasio...");
    m_nombreEdit->setStyleSheet(inputStyle);

    m_montoSpin = new QDoubleSpinBox();
    m_montoSpin->setPrefix("$ ");
    m_montoSpin->setMaximum(9999999.99);
    m_montoSpin->setDecimals(2);
    m_montoSpin->setStyleSheet(inputStyle);

    m_fechaEdit = new QDateEdit(QDate::currentDate().addMonths(1));
    m_fechaEdit->setCalendarPopup(true);
    m_fechaEdit->setStyleSheet(inputStyle);

    m_diasSpin = new QSpinBox();
    m_diasSpin->setRange(1, 30);
    m_diasSpin->setValue(5);
    m_diasSpin->setSuffix(" días antes");
    m_diasSpin->setStyleSheet(inputStyle);

    m_catCombo = new QComboBox();
    for (auto &c : CATEGORIAS) m_catCombo->addItem(c);
    m_catCombo->setCurrentText("Entretenimiento");
    m_catCombo->setStyleSheet(inputStyle);

    formL->addWidget(makeLabel("Nombre del Servicio"),    0, 0, 1, 2);
    formL->addWidget(m_nombreEdit,                        1, 0, 1, 2);
    formL->addWidget(makeLabel("Monto Mensual"),           2, 0);
    formL->addWidget(makeLabel("Categoría"),               2, 1);
    formL->addWidget(m_montoSpin,                         3, 0);
    formL->addWidget(m_catCombo,                          3, 1);
    formL->addWidget(makeLabel("Próx. Vencimiento"),       4, 0);
    formL->addWidget(makeLabel("Avisar con"),              4, 1);
    formL->addWidget(m_fechaEdit,                         5, 0);
    formL->addWidget(m_diasSpin,                          5, 1);

    mainL->addWidget(formW);

    // Info aviso
    QFrame *infoCard = new QFrame();
    infoCard->setStyleSheet(R"(
        QFrame {
            background-color: rgba(56, 189, 248, 0.06);
            border: 1px solid rgba(56,189,248,0.2);
            border-radius: 10px;
        }
    )");
    QHBoxLayout *infoL = new QHBoxLayout(infoCard);
    infoL->setContentsMargins(14, 10, 14, 10);
    QLabel *infoTxt = new QLabel("💡  Recibirás una notificación cuando la suscripción esté por vencer.");
    infoTxt->setStyleSheet("color: #38BDF8; font-size: 12px; background: transparent;");
    infoTxt->setWordWrap(true);
    infoL->addWidget(infoTxt);
    mainL->addWidget(infoCard);

    mainL->addStretch();

    // Botones
    QWidget *btnW = new QWidget();
    btnW->setStyleSheet("background: transparent;");
    QHBoxLayout *btnL = new QHBoxLayout(btnW);
    btnL->setContentsMargins(0,0,0,0);
    btnL->setSpacing(12);

    m_cancelBtn = new QPushButton("Cancelar");
    m_cancelBtn->setFixedHeight(40);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setStyleSheet(R"(
        QPushButton { background: transparent; color: #64748B; border: 1px solid #2E3347; border-radius: 8px; font-size: 13px; }
        QPushButton:hover { color: #94A3B8; border-color: #475569; }
    )");
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    m_saveBtn = new QPushButton("💾  Guardar");
    m_saveBtn->setFixedHeight(40);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setStyleSheet(R"(
        QPushButton { background-color: #38BDF8; color: #0F1117; border: none; border-radius: 8px; font-size: 13px; font-weight: 700; }
        QPushButton:hover { background-color: #0EA5E9; }
    )");
    connect(m_saveBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnL->addWidget(m_cancelBtn, 1);
    btnL->addWidget(m_saveBtn, 2);
    mainL->addWidget(btnW);
}

QString AddSubscriptionDialog::nombreServicio() const { return m_nombreEdit->text(); }
double  AddSubscriptionDialog::monto()           const { return m_montoSpin->value(); }
QDate   AddSubscriptionDialog::fechaVencimiento() const { return m_fechaEdit->date(); }
int     AddSubscriptionDialog::diasAviso()        const { return m_diasSpin->value(); }
QString AddSubscriptionDialog::categoria()        const { return m_catCombo->currentText(); }
QString AddSubscriptionDialog::iconoNombre()      const { return m_nombreEdit->text().toLower().simplified(); }
