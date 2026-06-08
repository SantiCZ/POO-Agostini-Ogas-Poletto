# 💸 AlcancIA - Sistema Inteligente de Gestión de Gastos y Suscripciones

**AlcancIA** es una solución de software integral de finanzas personales diseñada para simplificar el seguimiento de consumos diarios, el control de suscripciones y la digitalización automática de comprobantes. El sistema combina una aplicación de escritorio nativa de alto rendimiento con un backend inteligente alojado en un servidor en la nube (VPS).

El proyecto está estructurado bajo los estándares de la **Programación Orientada a Objetos (POO)** y utiliza inteligencia artificial para procesar tickets de compra, extrayendo de manera automática los ítems adquiridos, montos, categorías y comercios a partir de imágenes o archivos PDF.

---

## 🛠️ 1. Arquitectura General del Sistema

El sistema opera bajo un modelo cliente-servidor con soporte de sincronización y cache en modo offline (desconectado). 

```mermaid
graph TD
    subgraph Cliente_Escritorio [Cliente de Escritorio (Local)]
        QtApp[Aplicación Qt 6 C++]
        DBLocal[(SQLite tasty_alcancia.db)]
        QtApp <-->|Lectura/Escritura Local| DBLocal
    end

    subgraph Servidor_VPS [Servidor VPS en la Nube (Ubuntu 24.04)]
        FastAPI[FastAPI Backend - Python 3.11]
        DBMySQL[(Base de Datos - MySQL 8.0)]
        PMA[Administración - phpMyAdmin]
        Storage[Almacenamiento Físico - uploads/tickets/]
        
        FastAPI <-->|Consultas SQL| DBMySQL
        PMA -.->|Administración| DBMySQL
        FastAPI --->|Guarda Comprobantes| Storage
    end

    subgraph Servicios_Externos [Inteligencia Artificial]
        OpenAI[API de OpenAI - GPT-4o-mini]
    end

    QtApp <-->|Consultas y Sincronización REST| FastAPI
    FastAPI <-->|Análisis de Imágenes/PDFs| OpenAI
```

> [!NOTE]
> **Modo Offline Resiliente:** La aplicación de escritorio almacena toda la información localmente en una base de datos SQLite. Si el servidor VPS no está accesible, el usuario puede seguir registrando gastos y editando suscripciones. Al conectarse o iniciar sesión de nuevo, los cambios locales pendientes se sincronizan automáticamente con la base de datos MySQL en la nube.

---

## 🦾 2. Funcionalidades Principales

1. **Ingreso Omnicanal:** Carga de gastos manuales rápidos o subida directa de comprobantes (imágenes JPG/PNG y documentos digitales PDF).
2. **Procesamiento con IA:** Extracción automatizada de datos clave sin necesidad de OCRs tradicionales estructurados:
   - Nombre del comercio y categoría sugerida de gasto.
   - Fecha exacta de la compra.
   - Monto total y desglose completo de productos (ítems, cantidad, precio unitario y subtotal).
3. **Control de Suscripciones:** Registro de servicios recurrentes con fecha de vencimiento y configuración de días de aviso para alertas preventivas de pago.
4. **Notificaciones Inteligentes:** Panel interactivo que avisa sobre próximos vencimientos o acciones necesarias del usuario.
5. **Reportes Gráficos Dinámicos:** Visualización estadística mensual y distribución de los gastos según categorías.

---

## 🧠 3. Diseño de Software y Principios de POO (C++)

La aplicación de escritorio está estructurada en el lenguaje C++ bajo el framework Qt 6. Las clases principales residen en el archivo [models.h](./AlcancIA/models.h).

### 📐 Jerarquía de Modelos y Herencia
Para representar los documentos contables y los movimientos económicos, el sistema implementa una jerarquía basada en herencia:

* **Clase Base: `MovimientoBase`**
  Encapsula los datos esenciales compartidos por cualquier transacción financiera:
  - `id` (Identificador único).
  - `monto` (Valor monetario).
  - `fecha` (Fecha de registro/emisión).
  - `descripcion` (Notas complementarias).

* **Clase Derivada: `Ticket` (Gasto)**
  Hereda las propiedades de `MovimientoBase` y extiende su funcionalidad para modelar consumos desglosados:
  - `nombreLocal` (Comercio).
  - `categoria` (Clasificación).
  - `imagenPath` (Ruta física del comprobante).
  - `procesadoPorIA` (Flag de origen).
  - `items` (Lista dinámica de tipo `QList<ItemTicket>`).

* **Clase Derivada: `Suscripcion` (Servicio Recurrente)**
  Hereda las propiedades de `MovimientoBase` y se extiende con lógica de recurrencia y alertas:
  - `nombreServicio` (Ej: Netflix, Spotify, Luz).
  - `fechaVencimiento` (Fecha de renovación).
  - `diasAviso` (Anticipación para recordatorio).
  - `activa` (Estado lógico de la suscripción).
  - `iconoNombre` (Identificador visual).

### 🧩 Encapsulamiento y Gestión de Datos
* **`adminDB` ([admindb.h](./AlcancIA/admindb.h) / [admindb.cpp](./AlcancIA/admindb.cpp)):** Clase responsable de encapsular el acceso de bajo nivel a la base de datos SQLite local. Realiza la lectura de registros de la UI, la inserción offline y prepara las colas de sincronización local.
* **`DataManager` ([datamanager.h](./AlcancIA/datamanager.h) / [datamanager.cpp](./AlcancIA/datamanager.cpp)):** Clase controladora central que maneja la comunicación en red. Implementa el patrón cliente-servidor mediante llamadas asíncronas HTTP con `QNetworkAccessManager`, traduciendo los modelos de C++ a payloads JSON para el backend.

---

## ☁️ 4. Infraestructura y Lógica del Backend (FastAPI / VPS)

El backend, ubicado en la carpeta [backend_AlcanIA](./backend_AlcanIA), actúa como puente de seguridad, persistencia distribuida y procesamiento de inteligencia artificial.

### 🌐 Endpoints expuestos por la API:
* **Autenticación:**
  - `POST /api/v1/usuarios/registro`: Crea la cuenta de usuario en MySQL.
  - `POST /api/v1/usuarios/login`: Verifica accesos de manera segura.
* **Procesamiento de Tickets e Inteligencia Artificial:**
  - `POST /api/v1/tickets/analizar`: Recibe el archivo subido, lo guarda en el disco del VPS, detecta si es un PDF y en tal caso renderiza su primera página a JPEG en memoria usando **PyMuPDF (`fitz`)**. Posteriormente, envía los bytes en Base64 a OpenAI (GPT-4o-mini) bajo un estricto prompt contable que devuelve los datos estructurados en formato JSON.
  - `POST /api/v1/tickets/guardar`: Inserta el comprobante, el gasto general, los productos desglosados, las suscripciones sugeridas y las notificaciones correspondientes en una única transacción atómica en MySQL.
* **Sincronización:**
  - `GET /api/v1/usuarios/{id_usuario}/sync`: Descarga todas las categorías, gastos históricos, suscripciones activas y notificaciones pendientes para actualizar el SQLite del cliente.
  - `POST /api/v1/suscripciones/sync`: Sincroniza en lote todas las modificaciones, creaciones o eliminaciones lógicas de suscripciones realizadas por el usuario en modo offline.

> [!WARNING]
> **Seguridad y Hashing:** El backend recibe e interactúa con claves previamente encriptadas del lado del cliente. Sin embargo, en el módulo [security.py](./backend_AlcanIA/security.py) se encuentra implementada toda la lógica para hachear contraseñas mediante `bcrypt` y generar tokens `JWT` para futuras integraciones de autenticación por Token de portador (Bearer).

---

## 📂 5. Estructura del Repositorio

La carpeta principal del proyecto se divide en dos componentes independientes:

### 🖥️ Carpeta Frontend C++: `/AlcancIA`
* `main.cpp`: Punto de partida que inicializa la aplicación Qt y coordina la conexión local.
* `models.h`: Definición de la estructura de datos (clases y colecciones del modelo POO).
* `admindb.h` / `admindb.cpp`: Gestor de consultas SQLite local.
* `datamanager.h` / `datamanager.cpp`: Control de red y sincronización con el VPS.
* `logindialog.h` / `logindialog.cpp`: Pantalla de inicio de sesión y validación de red.
* `dashboardpage.cpp`, `reportspage.cpp`, `subscriptionspage.cpp`, `ticketspage.cpp`: Controladores de la interfaz gráfica del usuario.
* `uploadticketdialog.cpp`: Ventana que maneja la carga de archivos al servidor.
* `stylemanager.h` / `stylemanager.cpp`: Encargado de centralizar la hoja de estilos visual (CSS/QSS) de la interfaz de usuario.
* `frontend_2.pro`: Configuración de compilación del proyecto Qt Creator.

### ☁️ Carpeta Servidor VPS: `/backend_AlcanIA`
* `main.py`: Código principal en Python con los endpoints y lógica del servidor FastAPI.
* `security.py`: Librería local para hashing con bcrypt y generación de tokens JWT.
* `Dockerfile`: Instrucciones de construcción de la imagen contenedora de la API.
* `docker-compose.yml`: Orquestador de la infraestructura (MySQL, phpMyAdmin y FastAPI).
* `requirements.txt`: Dependencias del servidor de producción.
* `.env`: Variables de entorno para tokens, credenciales de MySQL y OpenAI API Keys.

---

## 🚀 6. Guía de Ejecución y Despliegue

### Requisitos del Cliente (Escritorio)
- **Qt Creator** con Qt 6 instalado (Compilador recomendado: MinGW 64-bit).
- Dependencia del driver SQLite integrado en el SDK de Qt.

**Instrucciones de ejecución:**
1. Abre Qt Creator.
2. Carga el archivo de configuración de proyecto `frontend_2.pro` ubicado en `/AlcancIA`.
3. Compila y ejecuta el proyecto.

### Requisitos del Servidor (VPS / Local)
- **Docker** y **Docker Compose** instalados.
- Un archivo `.env` configurado en la raíz de la carpeta `backend_AlcanIA`.

**Instrucciones de ejecución:**
1. Navega a la carpeta del backend:
   ```bash
   cd backend_AlcanIA
   ```
2. Levanta los contenedores en segundo plano:
   ```bash
   docker compose up -d --build
   ```
3. Verifica que los servicios estén corriendo:
   ```bash
   docker compose ps
   ```
   * La API de FastAPI estará disponible en el puerto `8000`.
   * El panel web de phpMyAdmin estará disponible en el puerto `8080`.

---

## 👤 7. Datos Académicos y Autoría
* **Proyecto:** Integrador AlcancIA
* **Materia:** Programación Orientada a Objetos (POO)
* **Docentes:** Equipo de Cátedra de POO
* **Universidad:** Universidad Blas Pascal (UBP)
* **Autores:** 
  - Agostini Santiago
  - Ogass Avril
  - Poletto Lorenzo
