#include <QApplication>
#include <QFont>
#include "mainwindow.h"
#include "logindialog.h"
#include "datamanager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFont font("Segoe UI", 10);
    font.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(font);

    // Cargar datos existentes
    DataManager::instance().loadFromFile();

    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window;
    window.show();

    return app.exec();
}