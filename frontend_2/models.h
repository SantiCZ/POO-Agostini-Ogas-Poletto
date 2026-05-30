#pragma once

#include <QString>
#include <QDate>
#include <QList>
#include <QStringList>

// ─────────────────────────────────────────────
// Movimiento base
// ─────────────────────────────────────────────

struct MovimientoBase {
    int     id = -1;
    double  monto = 0;
    QDate   fecha;
    QString categoria;
    QString descripcion;
};

// ─────────────────────────────────────────────
// Ítem de ticket
// ─────────────────────────────────────────────

struct ItemTicket
{
    QString nombre;

    int cantidad = 0;

    double precioUnitario = 0.0;
};

// ─────────────────────────────────────────────
// Ticket
// ─────────────────────────────────────────────

struct Ticket : public MovimientoBase
{
    QString nombreLocal;

    QList<ItemTicket> items;

    QString imagenPath;

    bool procesadoPorIA = false;
};

// ─────────────────────────────────────────────
// Suscripción
// ─────────────────────────────────────────────

struct Suscripcion : public MovimientoBase
{
    QString nombreServicio;

    QDate fechaVencimiento;

    int diasAviso = 0;

    bool activa = true;

    QString iconoNombre;
};

// ─────────────────────────────────────────────
// Usuario
// ─────────────────────────────────────────────

struct User
{
    int id = -1;

    QString username;

    QString passwordHash;
};

// ─────────────────────────────────────────────
// Categorías
// ─────────────────────────────────────────────

static const QStringList CATEGORIAS =
    {
        "Supermercado",
        "Restaurante",
        "Transporte",
        "Salud",
        "Entretenimiento",
        "Servicios",
        "Ropa",
        "Tecnología",
        "Otro"
};
struct Notificacion
{
    QString titulo;

    QString mensaje;

    QDate fecha;

    bool leida = false;
};