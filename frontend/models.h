#pragma once
#include <QString>
#include <QDate>
#include <QList>

// ─── Movimiento base ───────────────────────────────────────────────
struct MovimientoBase {
    int     id;
    double  monto;
    QDate   fecha;
    QString categoria;
    QString descripcion;
};

// ─── Ítem de ticket ───────────────────────────────────────────────
struct ItemTicket {
    QString nombre;
    int     cantidad;
    double  precioUnitario;
};

// ─── Ticket (hereda conceptualmente de MovimientoBase) ────────────
struct Ticket : public MovimientoBase {
    QString         nombreLocal;
    QList<ItemTicket> items;
    QString         imagenPath;
    bool            procesadoPorIA;
};

// ─── Suscripción ──────────────────────────────────────────────────
struct Suscripcion : public MovimientoBase {
    QString nombreServicio;
    QDate   fechaVencimiento;
    int     diasAviso;          // avisar X días antes
    bool    activa;
    QString iconoNombre;        // ej: "netflix", "spotify"
};

// ─── Categorías predefinidas ──────────────────────────────────────
static const QStringList CATEGORIAS = {
    "Supermercado", "Restaurante", "Transporte", "Salud",
    "Entretenimiento", "Servicios", "Ropa", "Tecnología", "Otro"
};
