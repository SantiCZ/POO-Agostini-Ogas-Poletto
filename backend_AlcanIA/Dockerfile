# =====================================================================
# DOCKERFILE - CONSTRUCCIÓN DEL CONTENEDOR FASTAPI
# =====================================================================
# Este archivo define los pasos para compilar e inicializar la imagen
# Docker que ejecuta nuestra API. Se basa en una imagen optimizada
# para FastAPI que ya viene con Uvicorn y Gunicorn preinstalados.
# =====================================================================

# 1. IMAGEN BASE
# Usamos la imagen oficial optimizada para FastAPI y Python 3.11
FROM tiangolo/uvicorn-gunicorn-fastapi:python3.11

# 2. INSTALACIÓN DE DEPENDENCIAS (Aprovechando la caché de Docker)
# Copiamos primero el archivo requirements.txt antes de copiar todo el código.
# Esto previene que se vuelvan a instalar todas las librerías cada vez que
# modifiques una sola línea de código en tu main.py.
COPY ./requirements.txt /app/requirements.txt

# Instalamos y actualizamos las dependencias en la imagen sin usar almacenamiento temporal
RUN pip install --no-cache-dir --upgrade -r /app/requirements.txt

# 3. COPIA DE CÓDIGO FUENTE
# Copiamos todo el contenido del directorio actual (incluyendo main.py y security.py)
# al directorio /app del contenedor.
COPY . /app