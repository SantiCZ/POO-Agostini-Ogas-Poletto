#pragma once
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QVector>

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

signals:
    void pageChanged(int index);

public slots:
    void setActivePage(int index);

private:
    void setupUI();

    QVBoxLayout   *m_layout       = nullptr;
    QLabel        *m_logoLabel    = nullptr;
    QLabel        *m_userLabel    = nullptr;
    QButtonGroup  *m_btnGroup     = nullptr;
    QVector<SidebarButton*> m_navButtons;
    int            m_activeIndex  = 0;
};
