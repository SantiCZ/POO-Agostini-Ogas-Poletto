#pragma once

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QSqlDatabase>

#include "admindb.h"

/*
 * StatCard
 * Responsabilidad de clase:
 * Tarjeta reutilizable para mostrar un indicador numerico del dashboard.
 *
 * Herencia: hereda de QFrame para funcionar como tarjeta visual.
 */
class StatCard : public QFrame {
    Q_OBJECT

public:
    explicit StatCard(const QString &icon,
                      const QString &title,
                      const QString &value,
                      const QString &subtitle,
                      const QString &accentColor = "#4ADE80",
                      QWidget *parent = nullptr);
};

/*
 * RecentExpenseRow
 * Responsabilidad de clase:
 * Fila compacta para listar movimientos recientes dentro del dashboard.
 *
 * Herencia: hereda de QWidget para integrarse en layouts dinamicos.
 */
class RecentExpenseRow : public QWidget {
    Q_OBJECT

public:
    explicit RecentExpenseRow(const QString &icon,
                              const QString &nombre,
                              const QString &categoria,
                              const QString &fecha,
                              double monto,
                              QWidget *parent = nullptr);
};

/*
 * DashboardPage
 * Responsabilidad de clase:
 * Pantalla inicial con resumen mensual, actividad reciente, accesos rapidos
 * y alertas de suscripciones proximas a vencer.
 *
 * SIGNAL:
 * navigateToTickets() y navigateToSubs() avisan a MainWidget que debe cambiar
 * de pagina cuando el usuario usa acciones rapidas.
 */
class DashboardPage : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);

    // Recarga datos desde SQLite y reconstruye los bloques dinamicos.
    void refreshData();

signals:
    // Navegacion solicitada desde acciones rapidas.
    void navigateToTickets();
    void navigateToSubs();

private:
    // Construccion de secciones visuales del dashboard.
    void setupUI();
    void buildStatsRow(QHBoxLayout *layout);
    void buildRecentActivity(QVBoxLayout *layout);
    void buildQuickActions(QVBoxLayout *layout);
    void buildSubsAlert(QVBoxLayout *layout);

    // Libera widgets dinamicos antes de reconstruir una seccion.
    void clearLayout(QLayout *layout);

    // Acceso a la conexion SQLite compartida.
    QSqlDatabase getDatabase();

    // Consultas de resumen usadas por las tarjetas principales.
    double obtenerGastoMesDB();
    int obtenerCantidadTicketsDB();
    int obtenerSuscripcionesActivasDB();
    int obtenerSuscripcionesPorVencerDB();

private:
    // Atributos importantes: layouts y widgets se guardan como miembros
    // para poder limpiar y reconstruir secciones durante refreshData().
    QHBoxLayout *m_statsLayout = nullptr;

    QVBoxLayout *m_activityLayout = nullptr;
    QVBoxLayout *m_alertLayout = nullptr;

    QWidget *m_statsWidget = nullptr;
    QWidget *m_activityWidget = nullptr;
    QWidget *m_alertWidget = nullptr;

    adminDB m_dbManager;
};
