# 💰 alcancIA - Gestión de Gastos Inteligente (Servidor VPS)

**alcancIA** es una plataforma diseñada para ayudar a los usuarios a realizar un seguimiento inteligente de sus gastos, suscripciones y finanzas personales utilizando herramientas de backend modernas, seguras y escalables desplegadas en un entorno de producción dedicado (VPS).

---

## 🚀 Estado del Proyecto: Etapa de Servidor y Producción (VPS)

El backend de **alcancIA** se encuentra desplegado y configurado en producción utilizando una arquitectura de microservicios contenerizados.

### 🛠️ Stack Tecnológico en el VPS
* **Servidor de Alojamiento:** VPS (Virtual Private Server) con **Ubuntu 24.04 LTS** administrado vía SSH.
* **Orquestación y Contenerización:** **Docker** y **Docker Compose** para asegurar entornos aislados y portables.
* **Motor Backend:** **FastAPI (Python 3.11)** por su alto rendimiento y generación automática de documentación interactiva.
* **Motor de Base de Datos:** **MySQL 8.0** para almacenamiento transaccional robusto y relacional.
* **Administración Visual:** **phpMyAdmin** para monitoreo y administración web directa del motor SQL.
* **Motor de Inteligencia Artificial:** Integración con la API de **OpenAI (GPT-4o-mini)** para extracción avanzada de tickets.

---

## 📦 Estructura del Servidor y Explicación de Archivos

Cada archivo en este repositorio cumple una función específica dentro de la arquitectura de la aplicación en el VPS:

### 🐍 Código de la Aplicación
* 📄 **[main.py](./main.py): El Cerebro del Servidor**
  * Es el punto de entrada de la aplicación FastAPI.
  * Define los endpoints REST que consume la aplicación cliente (Qt / C++).
  * Gestiona las conexiones transaccionales a MySQL (`get_db_connection`).
  * Administra el procesamiento de imágenes y la conversión de archivos PDF a imágenes legibles usando **PyMuPDF (`fitz`)**.
  * Envía datos procesados a la API de OpenAI y gestiona la persistencia unificada (gastos, ítems desglosados, suscripciones y notificaciones).
  * Expone las rutas de sincronización bidireccional de la base de datos local SQLite de los usuarios con el servidor MySQL central.

* 🔐 **[security.py](./security.py): Lógica de Criptografía**
  * Contiene las utilidades del servidor para seguridad de datos.
  * `encriptar_password`: Hachea contraseñas en texto plano usando el algoritmo industrial **bcrypt** con generación de sal (salt) aleatoria.
  * `verificar_password`: Compara claves en texto plano contra hashes guardados de forma segura.
  * `crear_token_acceso`: Genera y firma tokens de sesión **JWT (JSON Web Tokens)** con tiempo de expiración determinado.

### 🐳 Configuración de Despliegue (Docker)
* 🐳 **[Dockerfile](./Dockerfile): Receta de Construcción**
  * Define los pasos para construir la imagen del contenedor del backend.
  * Instala dependencias y prepara el entorno Python 3.11 sobre una imagen optimizada para servidores FastAPI con Uvicorn y Gunicorn.

* 🐙 **[docker-compose.yml](./docker-compose.yml): Orquestador de Servicios**
  * Define los contenedores que se levantan simultáneamente en el VPS:
    1. `db` (MySQL en puerto `3306`)
    2. `phpmyadmin` (Administrador de base de datos en puerto `8080`)
    3. `api_alcancia` (FastAPI en puerto `8000`)
    4. `api_kanban` (API externa/TP compartido en puerto `8001`)
  * Gestiona la creación del volumen persistente `db_data` para evitar pérdidas de datos en MySQL y carga el archivo de configuración `.env`.

### ⚙️ Configuraciones y Dependencias
* 📝 **[requirements.txt](./requirements.txt): Librerías Requeridas**
  * Listado detallado de todas las dependencias del servidor (FastAPI, PyMySQL, OpenAI, PyMuPDF, passlib, python-jose, etc.) necesarias para que el sistema funcione.
* 🔑 **[.env](./.env): Variables de Entorno**
  * Archivo de configuración local del VPS que almacena variables sensibles: contraseñas de MySQL, clave secreta del JWT y la API Key privada de OpenAI.
* 📁 **`uploads/` / `uploads/tickets/`: Almacén Físico**
  * Carpeta ubicada en el sistema de archivos del VPS donde se guardan de forma permanente e indexada las fotos y PDFs de los tickets de los usuarios para su posterior descarga o visualización en la aplicación.

---

## 🛠️ Comandos Útiles para el VPS

Una vez dentro de la consola del VPS vía SSH, los comandos comunes para administrar estos archivos y contenedores son:

```bash
# Levantar todos los servicios en segundo plano (daemon)
docker compose up -d --build

# Ver el estado y los logs en tiempo real de la API
docker compose logs -f api_alcancia

# Detener los servicios del servidor
docker compose down

# Entrar a la terminal interactiva del contenedor de base de datos MySQL
docker exec -it alcancIA_mysql mysql -u root -p
```

---

## 👤 Equipo de Desarrollo (POO)
* **Estudiantes:** [Agostini Santiago, Ogass Avril y Poletto Lorenzo]
* **Materia:** Programación Orientada a Objetos
* **Universidad:** Universidad Blas Pascal (UBP)