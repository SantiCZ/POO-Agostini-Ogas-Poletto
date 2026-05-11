#include "stylemanager.h"

QString StyleManager::appStyleSheet() {
    return QString(R"(
        QMainWindow, QDialog {
            background-color: #0F1117;
            color: #F1F5F9;
        }
        QWidget {
            background-color: transparent;
            color: #F1F5F9;
            font-family: 'Segoe UI', 'SF Pro Display', sans-serif;
            font-size: 13px;
        }
        QScrollArea {
            border: none;
            background: transparent;
        }
        QScrollBar:vertical {
            background: #1A1D27;
            width: 6px;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background: #2E3347;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical:hover {
            background: #4ADE80;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        QScrollBar:horizontal {
            background: #1A1D27;
            height: 6px;
            border-radius: 3px;
        }
        QScrollBar::handle:horizontal {
            background: #2E3347;
            border-radius: 3px;
        }
        QToolTip {
            background-color: #21253A;
            color: #F1F5F9;
            border: 1px solid #2E3347;
            border-radius: 6px;
            padding: 4px 8px;
        }
        QMessageBox {
            background-color: #1A1D27;
        }
        QMessageBox QLabel {
            color: #F1F5F9;
        }
        QMessageBox QPushButton {
            background-color: #4ADE80;
            color: #0F1117;
            border: none;
            border-radius: 6px;
            padding: 6px 16px;
            font-weight: bold;
        }
        QMessageBox QPushButton:hover {
            background-color: #22C55E;
        }
    )");
}

QString StyleManager::cardStyle(const QString &accent) {
    return QString(R"(
        QFrame {
            background-color: #1A1D27;
            border: 1px solid #2E3347;
            border-radius: 12px;
        }
        QFrame:hover {
            border: 1px solid %1;
        }
    )").arg(accent);
}

QString StyleManager::primaryButtonStyle() {
    return QString(R"(
        QPushButton {
            background-color: #4ADE80;
            color: #0F1117;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-weight: 700;
            font-size: 13px;
            letter-spacing: 0.3px;
        }
        QPushButton:hover {
            background-color: #22C55E;
        }
        QPushButton:pressed {
            background-color: #16A34A;
        }
        QPushButton:disabled {
            background-color: #2E3347;
            color: #64748B;
        }
    )");
}

QString StyleManager::secondaryButtonStyle() {
    return QString(R"(
        QPushButton {
            background-color: transparent;
            color: #4ADE80;
            border: 1px solid #4ADE80;
            border-radius: 8px;
            padding: 9px 20px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: rgba(74, 222, 128, 0.1);
        }
        QPushButton:pressed {
            background-color: rgba(74, 222, 128, 0.2);
        }
    )");
}

QString StyleManager::dangerButtonStyle() {
    return QString(R"(
        QPushButton {
            background-color: transparent;
            color: #F87171;
            border: 1px solid #F87171;
            border-radius: 8px;
            padding: 9px 20px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: rgba(248, 113, 113, 0.1);
        }
    )");
}

QString StyleManager::inputStyle() {
    return QString(R"(
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QTextEdit {
            background-color: #21253A;
            color: #F1F5F9;
            border: 1px solid #2E3347;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 13px;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus,
        QDateEdit:focus, QTextEdit:focus {
            border: 1px solid #4ADE80;
            outline: none;
        }
        QLineEdit::placeholder {
            color: #64748B;
        }
        QComboBox::drop-down {
            border: none;
            width: 24px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid #94A3B8;
        }
        QComboBox QAbstractItemView {
            background-color: #21253A;
            color: #F1F5F9;
            border: 1px solid #2E3347;
            selection-background-color: #2E3347;
            border-radius: 8px;
            padding: 4px;
        }
        QSpinBox::up-button, QSpinBox::down-button,
        QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
            background: #2E3347;
            border: none;
            border-radius: 3px;
        }
        QDateEdit::drop-down {
            border: none;
            width: 24px;
        }
        QCalendarWidget {
            background-color: #21253A;
            color: #F1F5F9;
        }
    )");
}

QString StyleManager::sidebarStyle() {
    return QString(R"(
        QWidget#sidebar {
            background-color: #13161F;
            border-right: 1px solid #2E3347;
        }
    )");
}
