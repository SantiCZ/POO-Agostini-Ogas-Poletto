#pragma once

#include <QString>
#include <QDate>
#include <QList>
#include <QStringList>

/*
 * models.h
 * Define las estructuras de datos compartidas por toda la aplicacion.
 * Estas entidades funcionan como modelos simples: transportan informacion
 * entre la interfaz, la base SQLite y las respuestas del servidor.
 */

// ─────────────────────────────────────────────
// Movimiento base
// ─────────────────────────────────────────────

// Responsabilidad: modelo base para movimientos economicos.
// Herencia: Ticket y Suscripcion reutilizan estos atributos comunes.
// Clase abstracta: no es abstracta porque tambien sirve como
// contenedor simple de datos compartidos.
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

// Responsabilidad: gasto registrado manualmente o desde IA.
// Herencia: extiende MovimientoBase con datos propios de tickets.
struct Ticket : public MovimientoBase
{
    // Comercio o local asociado al comprobante.
    QString nombreLocal;

    // Detalle opcional de productos asociados al ticket.
    QList<ItemTicket> items;

    // Ruta local de la imagen seleccionada por el usuario.
    QString imagenPath;

    // Indica si los datos fueron completados por el servicio de IA.
    bool procesadoPorIA = false;
};

// ─────────────────────────────────────────────
// Suscripción
// ─────────────────────────────────────────────

// Responsabilidad: servicio recurrente con vencimiento y alerta.
// Herencia: extiende MovimientoBase con datos propios de suscripciones.
struct Suscripcion : public MovimientoBase
{
    // Nombre visible del servicio, por ejemplo Netflix o Spotify.
    QString nombreServicio;

    // Fecha en la que vence o se renueva el pago.
    QDate fechaVencimiento;

    // Dias previos al vencimiento para avisar al usuario.
    int diasAviso = 0;

    // Permite desactivar sin borrar definitivamente el registro.
    bool activa = true;

    // Nombre usado por la interfaz para elegir un icono representativo.
    QString iconoNombre;
};

// ─────────────────────────────────────────────
// Usuario
// ─────────────────────────────────────────────

// Usuario local utilizado para sesion y login offline.
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
// Alerta mostrada al usuario dentro de la aplicacion.
struct Notificacion
{
    QString titulo;

    QString mensaje;

    QDate fecha;

    bool leida = false;
};
