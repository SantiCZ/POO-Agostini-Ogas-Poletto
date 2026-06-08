# =====================================================================
# SYSTEMA DE SEGURIDAD - ALCANCIA
# =====================================================================
# Este archivo contiene las funciones necesarias para garantizar la 
# seguridad del backend, incluyendo el hashing de contraseñas con bcrypt 
# y la generación de tokens JWT (JSON Web Tokens) para sesiones.
# =====================================================================

import bcrypt
import os
from datetime import datetime, timedelta
from jose import jwt
from dotenv import load_dotenv

# ---------------------------------------------------------------------
# 1. CARGA DE CONFIGURACIÓN
# ---------------------------------------------------------------------
# Cargamos las variables de entorno del archivo .env para acceder de
# forma segura a las claves secretas y parámetros del sistema.
# ---------------------------------------------------------------------
load_dotenv()

# Clave secreta para firmar los tokens JWT. Se lee del archivo .env;
# si no existe, se utiliza un valor por defecto seguro para desarrollo.
SECRET_KEY = os.getenv("SECRET_KEY", "clave_temporal_de_desarrollo_muy_larga_123") 

# Algoritmo de encriptación estándar para firmas JWT.
ALGORITHM = "HS256"

# Tiempo de validez establecido para cada token de acceso (en minutos).
ACCESS_TOKEN_EXPIRE_MINUTES = 30


# ---------------------------------------------------------------------
# 2. FUNCIONES DE CONTRASEÑAS (HASHING Y VERIFICACIÓN)
# ---------------------------------------------------------------------

def encriptar_password(password: str) -> str:
    """
    Convierte una contraseña en texto plano en un hash seguro no-reversible
    utilizando el algoritmo bcrypt con generación dinámica de sal (salt).
    
    Args:
        password (str): Contraseña en texto plano ingresada por el usuario.
        
    Returns:
        str: El hash resultante codificado en formato UTF-8 listo para base de datos.
    """
    # Convertimos la contraseña de texto plano (string) a representación binaria (bytes)
    password_bytes = password.encode('utf-8')
    
    # Generamos una sal única y aleatoria
    salt = bcrypt.gensalt()
    
    # Generamos el hash combinando la contraseña binaria y la sal
    hash_resultado = bcrypt.hashpw(password_bytes, salt)
    
    # Retornamos el hash decodificado en string para su almacenamiento
    return hash_resultado.decode('utf-8')


def verificar_password(password_plana: str, password_encriptada: str) -> bool:
    """
    Compara una contraseña ingresada en texto plano contra un hash
    guardado previamente en la base de datos para validar si coinciden.
    
    Args:
        password_plana (str): Contraseña que se desea verificar.
        password_encriptada (str): Hash seguro guardado en la base de datos.
        
    Returns:
        bool: True si la contraseña coincide con el hash, False en caso contrario.
    """
    # bcrypt extrae internamente la sal del hash guardado y realiza la comparación segura
    return bcrypt.checkpw(
        password_plana.encode('utf-8'), 
        password_encriptada.encode('utf-8')
    )


# ---------------------------------------------------------------------
# 3. GESTIÓN DE TOKENS DE SESIÓN (JWT)
# ---------------------------------------------------------------------

def crear_token_acceso(data: dict) -> str:
    """
    Genera un token JWT (JSON Web Token) firmado con la clave del servidor
    que sirve como credencial temporal para peticiones autenticadas.
    
    Args:
        data (dict): Diccionario con los datos del payload (ej: id_usuario).
        
    Returns:
        str: Token JWT firmado.
    """
    # Hacemos una copia del diccionario para evitar mutar el original
    a_encriptar = data.copy()
    
    # Definimos la fecha y hora exacta en la que expirará este token (UTC)
    expiracion = datetime.utcnow() + timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)
    
    # Agregamos la clave estándar 'exp' requerida por el estándar JWT para controlar la validez
    a_encriptar.update({"exp": expiracion})
    
    # Firmamos digitalmente el payload usando la SECRET_KEY y el algoritmo HS256
    return jwt.encode(a_encriptar, SECRET_KEY, algorithm=ALGORITHM)