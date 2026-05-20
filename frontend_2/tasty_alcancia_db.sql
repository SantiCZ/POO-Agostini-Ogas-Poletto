-- ============================================================
--  alcancIA — Script de creación de base de datos
--  Motor: MySQL 8.0+
--  Generado para usar en phpMyAdmin / AdminDB
-- ============================================================

CREATE DATABASE IF NOT EXISTS tasty_alcancIA
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE tasty_alcancIA;

-- ------------------------------------------------------------
-- 1. USUARIOS
-- ------------------------------------------------------------
CREATE TABLE usuarios (
    id_usuario    INT           NOT NULL AUTO_INCREMENT,
    nombre        VARCHAR(100)  NOT NULL,
    email         VARCHAR(150)  NOT NULL,
    clave         VARCHAR(255)  NOT NULL,
    fecha_registro DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    actividad     TINYINT(1)   NOT NULL DEFAULT 1,

    PRIMARY KEY (id_usuario),
    UNIQUE KEY uq_usuarios_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


-- ------------------------------------------------------------
-- 2. CATEGORIAS
--    id_usuario NULL = categoría del sistema (compartida)
--    id_usuario con valor = categoría personalizada del usuario
-- ------------------------------------------------------------
CREATE TABLE categorias (
    id_categoria  INT           NOT NULL AUTO_INCREMENT,
    id_usuario    INT               NULL DEFAULT NULL,
    nombre        VARCHAR(80)   NOT NULL,
    icono         VARCHAR(50)       NULL,
    color         VARCHAR(7)        NULL,

    PRIMARY KEY (id_categoria),
    CONSTRAINT fk_categorias_usuario
        FOREIGN KEY (id_usuario) REFERENCES usuarios (id_usuario)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


-- ------------------------------------------------------------
-- 3. COMPROBANTES
-- ------------------------------------------------------------
CREATE TABLE comprobantes (
    id_comprobante INT          NOT NULL AUTO_INCREMENT,
    id_usuario     INT          NOT NULL,
    ruta_archivo   VARCHAR(500) NOT NULL,
    fecha_subida   DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    estado         ENUM('pendiente','procesado','error')
                               NOT NULL DEFAULT 'pendiente',

    PRIMARY KEY (id_comprobante),
    CONSTRAINT fk_comprobantes_usuario
        FOREIGN KEY (id_usuario) REFERENCES usuarios (id_usuario)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


-- ------------------------------------------------------------
-- 4. GASTOS
-- ------------------------------------------------------------
CREATE TABLE gastos (
    id_gasto       INT            NOT NULL AUTO_INCREMENT,
    id_usuario     INT            NOT NULL,
    id_comprobante INT                NULL DEFAULT NULL,
    id_categoria   INT            NOT NULL,
    comercio       VARCHAR(150)       NULL,
    monto          DECIMAL(12,2)  NOT NULL,
    fecha_gasto    DATE           NOT NULL,
    notas          TEXT               NULL,
    fecha_registro DATETIME       NOT NULL DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (id_gasto),
    CONSTRAINT fk_gastos_usuario
        FOREIGN KEY (id_usuario)     REFERENCES usuarios     (id_usuario)
        ON DELETE CASCADE  ON UPDATE CASCADE,
    CONSTRAINT fk_gastos_comprobante
        FOREIGN KEY (id_comprobante) REFERENCES comprobantes (id_comprobante)
        ON DELETE SET NULL ON UPDATE CASCADE,
    CONSTRAINT fk_gastos_categoria
        FOREIGN KEY (id_categoria)   REFERENCES categorias   (id_categoria)
        ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


-- ------------------------------------------------------------
-- 5. ITEMS_GASTO
-- ------------------------------------------------------------
CREATE TABLE items_gasto (
    id_item         INT            NOT NULL AUTO_INCREMENT,
    id_gasto        INT            NOT NULL,
    descripcion     VARCHAR(255)   NOT NULL,
    cantidad        DECIMAL(8,2)   NOT NULL DEFAULT 1,
    precio_unitario DECIMAL(10,2)  NOT NULL,
    subtotal        DECIMAL(10,2)  NOT NULL,

    PRIMARY KEY (id_item),
    CONSTRAINT fk_items_gasto
        FOREIGN KEY (id_gasto) REFERENCES gastos (id_gasto)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


-- ------------------------------------------------------------
-- 6. SUSCRIPCIONES
-- ------------------------------------------------------------
CREATE TABLE suscripciones (
    id_suscripcion INT           NOT NULL AUTO_INCREMENT,
    id_usuario     INT           NOT NULL,
    id_categoria   INT           NOT NULL,
    nombre         VARCHAR(150)  NOT NULL,
    monto          DECIMAL(10,2) NOT NULL,
    moneda         VARCHAR(10)   NOT NULL DEFAULT 'ARS',
    frecuencia     ENUM('diaria','semanal','mensual','anual')
                                 NOT NULL DEFAULT 'mensual',
    vencimiento    DATE          NOT NULL,
    alerta         INT           NOT NULL DEFAULT 3,
    actividad      TINYINT(1)   NOT NULL DEFAULT 1,
    notas          TEXT              NULL,

    PRIMARY KEY (id_suscripcion),
    CONSTRAINT fk_suscripciones_usuario
        FOREIGN KEY (id_usuario)   REFERENCES usuarios   (id_usuario)
        ON DELETE CASCADE  ON UPDATE CASCADE,
    CONSTRAINT fk_suscripciones_categoria
        FOREIGN KEY (id_categoria) REFERENCES categorias (id_categoria)
        ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


-- ------------------------------------------------------------
-- 7. NOTIFICACIONES
-- ------------------------------------------------------------
CREATE TABLE notificaciones (
    id_notificacion INT      NOT NULL AUTO_INCREMENT,
    id_usuario      INT      NOT NULL,
    tipo            ENUM('vencimiento','resumen','alerta')
                             NOT NULL,
    mensaje         TEXT     NOT NULL,
    leida           TINYINT(1) NOT NULL DEFAULT 0,
    fecha_creacion  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (id_notificacion),
    CONSTRAINT fk_notificaciones_usuario
        FOREIGN KEY (id_usuario) REFERENCES usuarios (id_usuario)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


-- ------------------------------------------------------------
-- ÍNDICES EXTRA (mejoran rendimiento en consultas frecuentes)
-- ------------------------------------------------------------
CREATE INDEX idx_gastos_fecha        ON gastos         (fecha_gasto);
CREATE INDEX idx_gastos_categoria    ON gastos         (id_categoria);
CREATE INDEX idx_suscripciones_venc  ON suscripciones  (vencimiento);
CREATE INDEX idx_notificaciones_leida ON notificaciones (leida);
CREATE INDEX idx_comprobantes_estado  ON comprobantes   (estado);

-- ============================================================
--  FIN DEL SCRIPT
-- ============================================================
