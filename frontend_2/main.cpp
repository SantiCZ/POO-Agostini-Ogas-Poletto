#include <QApplication>
#include <QFont>
#include <QMessageBox>
#include "mainwidget.h"
#include "logindialog.h"
#include "datamanager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFont font("Segoe UI", 10);
    font.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(font);

    app.setApplicationName("AlcancIA");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("AlcancIA Team");

    // Cargar datos existentes
    DataManager::instance().loadFromFile();
    // Crear usuario demo si no hay ninguno
    DataManager::instance().migrateUsers();

    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    // Si el usuario se acaba de registrar, mostrar mensaje de bienvenida
    if (login.wasRegistered()) {
        QMessageBox msg;
        msg.setWindowTitle("Bienvenido a AlcancIA");
        msg.setText(QString("Cuenta creada exitosamente.\n\nHola, %1! Ya podés empezar.").arg(login.getUsername()));
        msg.setStyleSheet(R"(
            QMessageBox { background-color: #1A1D27; }
            QMessageBox QLabel { color: #F1F5F9; font-size: 13px; }
            QMessageBox QPushButton {
                background-color: #4ADE80; color: #0F1117;
                border: none; border-radius: 6px;
                padding: 6px 20px; font-weight: 700;
            }
            QMessageBox QPushButton:hover { background-color: #22C55E; }
        )");
        msg.exec();
    }

    MainWidget window;
    window.setWindowTitle(QString("AlcancIA - %1").arg(login.getUsername()));
    window.show();

    return app.exec();
}