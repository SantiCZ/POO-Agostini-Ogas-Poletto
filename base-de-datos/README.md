# Arquitectura de la base de datos — AlcancIA

## Idea general de como funciona el flujo de la base de datos 

* **MySQL en VPS** como base de datos principal centralizada.
* **SQLite local** como almacenamiento liviano y caché en cada computadora del usuario.

```txt
                ┌──────────────────────┐
                │     Aplicación Qt    │
                └──────────┬───────────┘
                           │
              ┌────────────┴────────────┐
              │                         │
              ▼                         ▼

     ┌────────────────┐       ┌──────────────────┐
     │ SQLite Local   │       │ VPS + MySQL      │
     │ (caché local)  │       │ (base principal) │
     └────────────────┘       └──────────────────┘
```

---

# Función de Cada Base de Datos

---

# 1. MySQL en VPS (Fuente de verdad)

La base de datos MySQL alojada en el VPS almacena toda la información completa del sistema y representa la fuente oficial de datos.

## Responsabilidades

* Usuarios completos
* Categorías
* Gastos históricos
* Comprobantes
* Items detallados
* Suscripciones
* Notificaciones
* Información compartida
* Sincronización entre dispositivos

## Administración

La administración se realiza mediante:

* phpMyAdmin
* conexión SSH
* API/backend del VPS

# 2. SQLite Local (Caché)

La aplicación Qt incorpora una base SQLite local almacenada en la computadora del usuario. Esta base NO reemplaza a MySQL, sino que funciona como:

* caché local,
* almacenamiento temporal,
* respaldo liviano,
* optimización de consultas frecuentes.

## Objetivo del SQLite Local

El almacenamiento local permite:

* acelerar el acceso a información frecuente,
* disminuir consultas repetitivas al VPS,
* mantener información mínima persistente,
* optimizar tiempos de carga,
* reducir tráfico hacia el servidor.

---

# Estructura Local de SQLite

La base SQLite NO almacena toda la información del sistema. Solo guarda una copia mínima necesaria para funcionamiento local.

## Tablas locales

| Tabla            | Función                    |
| ---------------- | -------------------------- |
| `usuario_sesion` | Mantener sesión local      |
| `categorias`     | Caché de categorías        |
| `gastos`         | Últimos gastos del usuario |
| `suscripciones`  | Suscripciones activas      |
| `notificaciones` | Notificaciones pendientes  |

---

# Información NO almacenada localmente

Las siguientes tablas permanecen exclusivamente en MySQL:

| Tabla              | Motivo               |
| ------------------ | -------------------- |
| `comprobantes`     | Archivos pesados     |
| `items_gasto`      | Datos muy detallados |
| historial completo | Reduce tamaño local  |

---
# Flujo General

```txt
1. Qt solicita información al VPS
2. MySQL responde los datos
3. Qt actualiza SQLite local
4. SQLite mantiene una copia parcial optimizada
5. Consultas frecuentes pueden resolverse localmente
```

---

# Control de Sincronización

Las tablas locales poseen campos especiales:

```sql
sincronizado INTEGER DEFAULT 0
```

| Valor | Significado                 |
| ----- | --------------------------- |
| `0`   | Pendiente de sincronización |
| `1`   | Sincronizado con VPS        |

También se utiliza:

```sql
accion_pendiente
```

Para indicar:

* crear
* editar
* eliminar

---