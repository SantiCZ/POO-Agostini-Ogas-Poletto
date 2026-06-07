#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QVector>
#include <QFrame>

/*
 * SidebarButton
 * Responsabilidad de clase:
 * Boton de navegacion con icono, texto y estado activo.
 *
 * Herencia:
 * Hereda de QPushButton para reutilizar comportamiento de boton y personalizar
 * su estado visual.
 */
class SidebarButton : public QPushButton {
    Q_OBJECT
public:
    explicit SidebarButton(const QString &icon, const QString &text, QWidget *parent = nullptr);

    // Cambia el estilo visual segun la pagina seleccionada.
    void setActive(bool active);

private:
    QString m_icon;
    QString m_text;

    void updateStyle(bool active);
};

/*
 * Sidebar
 * Responsabilidad de clase:
 * Menu lateral de navegacion.
 * Emite senales hacia MainWidget para cambiar de pagina, cerrar sesion o
 * abrir el dialogo de carga rapida de tickets.
 *
 * SIGNAL/SLOT:
 * pageChanged(), logoutRequested() y uploadTicketRequested() comunican eventos
 * de usuario a MainWidget sin acoplar ambas clases.
 *
 * Eventos de mouse:
 * No se sobrescribe mousePressEvent(); se aprovecha el evento de mouse ya
 * implementado por QPushButton y se responde mediante clicked().
 */
class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

    // Actualiza los datos visibles del usuario autenticado.
    void setUserInfo(const QString &name, const QString &email);

signals:
    void pageChanged(int index);
    void logoutRequested();
    void uploadTicketRequested();

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

    QPushButton             *m_profileBtn      = nullptr;
    QLabel                  *m_userNameLabel   = nullptr;
    QLabel                  *m_userEmailLabel  = nullptr;

    QLabel                  *m_syncIcon        = nullptr;

    // Se conserva como miembro de clase para conectar y estilizar el logout.
    QPushButton             *m_logoutBtn       = nullptr;
};
