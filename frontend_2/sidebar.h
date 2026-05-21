#pragma once
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QVector>
#include <QFrame>

class SidebarButton : public QPushButton {
    Q_OBJECT
public:
    explicit SidebarButton(const QString &icon, const QString &text, QWidget *parent = nullptr);
    void setActive(bool active);
private:
    QString m_icon;
    QString m_text;
    void updateStyle(bool active);
};

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

    void setUserInfo(const QString &name, const QString &email);

signals:
    void pageChanged(int index);
    void logoutRequested();
    void editProfileRequested();
    void changePasswordRequested();

public slots:
    void setActivePage(int index);
    void setSyncStatus(bool online, int pending = 0);

private slots:
    void onProfileClicked();

private:
    void setupUI();

    QVBoxLayout             *m_layout          = nullptr;
    QButtonGroup            *m_btnGroup        = nullptr;
    QVector<SidebarButton*>  m_navButtons;
    int                      m_activeIndex     = 0;

    // Perfil clickeable
    QPushButton             *m_profileBtn      = nullptr;
    QLabel                  *m_userNameLabel   = nullptr;
    QLabel                  *m_userEmailLabel  = nullptr;

    // Sincronización
    QLabel                  *m_syncIcon        = nullptr;

    // Mantenido por compatibilidad con código existente que use m_logoutBtn
    QPushButton             *m_logoutBtn       = nullptr;
};