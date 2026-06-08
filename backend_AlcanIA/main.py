# =====================================================================
# API PRINCIPAL DE ALCANCIA - FASTAPI BACKEND
# =====================================================================
# Este archivo es el cerebro del servidor (VPS). Define todas las rutas
# y endpoints necesarios para la aplicación AlcancIA, incluyendo el 
# registro y login de usuarios, sincronización de base de datos local 
# (Qt SQLite) con MySQL, almacenamiento físico de imágenes/PDFs de 
# comprobantes, procesamiento inteligente de tickets usando el SDK de 
# OpenAI (GPT-4o-mini) y la persistencia de transacciones.
# =====================================================================

import os
import json
import base64
import uuid
import fitz  # PyMuPDF para procesamiento de PDFs
from fastapi import FastAPI, UploadFile, File, HTTPException, Body
from fastapi.responses import FileResponse
from openai import OpenAI
from dotenv import load_dotenv
import pymysql
from datetime import datetime
from pydantic import BaseModel

# Importamos funciones de seguridad. 
# NOTA: En la lógica actual de producción estas funciones están importadas 
# pero no se utilizan para autenticar las peticiones ni hashear en el backend, 
# ya que la clave viene pre-hacheada y la verificación se hace directo en SQL.
from security import encriptar_password, crear_token_acceso

# Cargamos el archivo de configuración .env
load_dotenv()

# ---------------------------------------------------------------------
# 1. CONFIGURACIONES E INICIALIZACIÓN
# ---------------------------------------------------------------------

# Cliente oficial de OpenAI configurado con la API Key del entorno
client = OpenAI(api_key=os.getenv("OPENAI_API_KEY"))

# Inicialización de la aplicación FastAPI
app = FastAPI(title="API de AlcancIA")


# ---------------------------------------------------------------------
# 2. CONEXIÓN A LA BASE DE DATOS (MYSQL)
# ---------------------------------------------------------------------

def get_db_connection():
    """
    Establece y retorna una conexión activa con el contenedor de MySQL
    utilizando PyMySQL y formateando los resultados como diccionarios.
    """
    return pymysql.connect(
        host=os.getenv("DB_HOST", "alcancIA_mysql"),
        user="root",                 # Usuario administrador por defecto
        password="RootPlebe2026.",   # Contraseña de MySQL en producción
        database="tasty_alcancIA",   # Nombre de la base de datos
        cursorclass=pymysql.cursors.DictCursor
    )


# ---------------------------------------------------------------------
# 3. ENDPOINTS BASE / GENERALES
# ---------------------------------------------------------------------

@app.get("/")
def bienvenida():
    """
    Ruta raíz para verificar la salud del servidor y constatar 
    que el backend está corriendo correctamente.
    """
    return {"mensaje": "Cerebro de AlcancIA actualizado (SDK v2) - Motor GPT"}


# ---------------------------------------------------------------------
# 4. GESTIÓN DE USUARIOS (REGISTRO Y ACCESO)
# ---------------------------------------------------------------------

# Modelo de Pydantic para validar los datos que envía la app Qt en el registro
class UsuarioRegistro(BaseModel):
    nombre: str
    email: str
    clave_hash: str  # Hash de la contraseña generado del lado del cliente (Qt / Mac)


@app.post("/api/v1/usuarios/registro")
async def registrar_usuario(user: UsuarioRegistro):
    """
    Registra un nuevo usuario en la base de datos MySQL de producción.
    Verifica que el nombre y el correo electrónico no existan previamente.
    """
    connection = None
    try:
        connection = get_db_connection()
        with connection.cursor() as cursor:
            # 1. Verificamos si el usuario o email ya existen en la base de datos
            cursor.execute(
                "SELECT id_usuario FROM usuarios WHERE nombre = %s OR email = %s", 
                (user.nombre, user.email)
            )
            if cursor.fetchone():
                return {"status": "error", "message": "El nombre de usuario o email ya están en uso."}

            # 2. Insertamos el nuevo registro con actividad = 1 (Activo)
            sql = """
                INSERT INTO usuarios (nombre, email, clave, fecha_registro, actividad)
                VALUES (%s, %s, %s, NOW(), 1)
            """
            cursor.execute(sql, (user.nombre, user.email, user.clave_hash))
            connection.commit()
            
            return {"status": "ok", "message": "Usuario creado exitosamente en producción."}

    except Exception as e:
        if connection:
            connection.rollback()
        print(f"DEBUG REGISTRO: {str(e)}")
        raise HTTPException(status_code=500, detail="Error al crear el usuario en la base de datos.")
    finally:
        if connection:
            connection.close()


# Modelo de Pydantic para validar los datos que envía la app Qt en el login
class UsuarioLogin(BaseModel):
    email: str
    clave_hash: str  # Hash de la contraseña enviado desde el cliente para validación directa


@app.post("/api/v1/usuarios/login")
async def login_usuario(user: UsuarioLogin):
    """
    Autentica a un usuario comprobando directamente el email y el hash
    de la contraseña almacenada en la base de datos.
    """
    connection = None
    try:
        connection = get_db_connection()
        with connection.cursor() as cursor:
            # Buscamos al usuario por email y clave encriptada (comparación directa de hashes)
            sql = """
                SELECT id_usuario, nombre, email, actividad 
                FROM usuarios 
                WHERE email = %s AND clave = %s
            """
            cursor.execute(sql, (user.email, user.clave_hash))
            usuario = cursor.fetchone()

            # Si la consulta no trae registros, las credenciales son inválidas
            if not usuario:
                raise HTTPException(status_code=401, detail="Credenciales incorrectas")

            # Si el usuario existe pero está desactivado administrativamente
            if usuario['actividad'] == 0:
                raise HTTPException(status_code=403, detail="El usuario está desactivado")

            # Retornamos confirmación exitosa con la información básica del usuario
            return {
                "status": "ok",
                "message": "Login exitoso",
                "usuario": {
                    "id_usuario": usuario["id_usuario"],
                    "nombre": usuario["nombre"],
                    "email": usuario["email"]
                }
            }

    except HTTPException:
        # Relanzamos las excepciones HTTP propias (401, 403) sin modificarlas
        raise
    except Exception as e:
        print(f"DEBUG LOGIN: {str(e)}")
        raise HTTPException(status_code=500, detail="Error interno al procesar el login")
    finally:
        if connection:
            connection.close()


# ---------------------------------------------------------------------
# 5. MOTOR DE INTELIGENCIA ARTIFICIAL (PROCESAMIENTO DE COMPROBANTES)
# ---------------------------------------------------------------------

@app.post("/api/v1/tickets/analizar")
async def analizar_ticket(file: UploadFile = File(...)):
    """
    Recibe un archivo (imagen o PDF), lo guarda físicamente en el VPS y
    utiliza el modelo GPT-4o-mini de OpenAI para extraer de forma estructurada
    los datos contables del comprobante (comercio, monto, artículos, etc.).
    """
    try:
        # 1. Creamos la estructura de directorios en el VPS si no existe
        DIRECTORIO_TICKETS = "uploads/tickets"
        os.makedirs(DIRECTORIO_TICKETS, exist_ok=True)

        # 2. Generamos un nombre aleatorio único para evitar colisiones en disco
        extension = file.filename.split('.')[-1] if '.' in file.filename else 'jpg'
        nombre_unico = f"ticket_{uuid.uuid4().hex[:8]}.{extension}"
        ruta_vps = os.path.join(DIRECTORIO_TICKETS, nombre_unico)

        image_data = await file.read()
        
        # Guardamos el archivo original físicamente en el disco
        with open(ruta_vps, "wb") as buffer:
            buffer.write(image_data)

        # ─── CONVERSIÓN DE PDF A IMAGEN PARA LA IA ───
        # GPT no analiza archivos binarios PDF directamente. Si es un PDF, renderizamos 
        # la primera página a JPEG en memoria usando PyMuPDF (fitz) para enviarlo como imagen.
        if file.filename.lower().endswith('.pdf') or file.content_type == "application/pdf":
            doc = fitz.open(stream=image_data, filetype="pdf")
            page = doc.load_page(0)          # Obtenemos la primera página
            pix = page.get_pixmap(dpi=150)   # Renderizamos a matriz de píxeles
            image_data_for_ai = pix.tobytes("jpg")  # Convertimos la página renderizada a bytes JPG
            doc.close()
        else:
            # Si ya es una imagen (.png, .jpg, etc.), la enviamos directamente
            image_data_for_ai = image_data
        # ─────────────────────────────────────────────

        # Convertimos la imagen a Base64 para que la API de OpenAI pueda interpretarla
        base64_image = base64.b64encode(image_data_for_ai).decode('utf-8')
        
        # Prompt de sistema que fuerza una respuesta en formato JSON estructurado rígido
        system_prompt = """
        Eres un asistente experto en extracción de datos contables de tickets y facturas.
        Debes analizar la imagen proporcionada y devolver ÚNICAMENTE un objeto JSON válido con la siguiente estructura estricta, sin texto adicional ni bloques de código (Markdown):

        {
          "comprobante": { "ruta_archivo": "string", "estado": "procesado" },
          "gasto": {
            "comercio": "string",
            "monto": float,
            "fecha_gasto": "YYYY-MM-DD",
            "categoria_sugerida": "string",
            "notas": "string"
          },
          "items_gasto": [ { "descripcion": "string", "cantidad": float, "precio_unitario": float, "subtotal": float } ],
          "suscripcion": null,
          "notificacion": { "tipo": "string", "mensaje": "string" }
        }

        Reglas: 
        1. Las fechas deben estar en formato ISO 8601 (YYYY-MM-DD).
        2. Para el campo "categoria_sugerida", DEBES elegir estrictamente UNA de estas opciones: "Supermercado", "Restaurante", "Transporte", "Salud", "Entretenimiento", "Servicios", "Ropa", "Tecnología", "Otro".
        """

        # Petición al modelo de OpenAI
        response = client.chat.completions.create(
            model="gpt-4o-mini",
            response_format={ "type": "json_object" }, # Fuerza salida en formato JSON
            temperature=0.0, # Temperatura 0 para una extracción precisa e idéntica
            messages=[
                {"role": "system", "content": system_prompt},
                {
                    "role": "user",
                    "content": [
                        {"type": "text", "text": "Analiza este ticket y devuelve el JSON estructurado respetando las reglas."},
                        {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{base64_image}"}}
                    ]
                }
            ]
        )

        texto = response.choices[0].message.content
        respuesta_json = json.loads(texto)

        # 3. Inyectamos la ruta del VPS correspondiente a este archivo guardado
        if "comprobante" not in respuesta_json:
            respuesta_json["comprobante"] = {}
        
        respuesta_json["comprobante"]["ruta_archivo"] = ruta_vps

        return respuesta_json

    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error en el Motor IA: {str(e)}")


# ---------------------------------------------------------------------
# 6. PERSISTENCIA DE GASTOS Y COMPROBANTES (TRANSACCIONAL EN MYSQL)
# ---------------------------------------------------------------------

@app.post("/api/v1/tickets/guardar")
async def guardar_ticket(payload: dict = Body(...)):
    """
    Recibe la estructura JSON extraída (y potencialmente editada por el usuario)
    y la inserta de forma transaccional en la base de datos (tablas: comprobantes, 
    gastos, items_gasto, suscripciones y notificaciones).
    """
    connection = None
    try:
        connection = get_db_connection()
        with connection.cursor() as cursor:
            
            # --- VALIDACIÓN OBLIGATORIA DEL ID_USUARIO ---
            id_usuario = payload.get("id_usuario")
            if not id_usuario:
                raise HTTPException(status_code=400, detail="id_usuario requerido")
            # ---------------------------------------------

            ahora = datetime.now()

            # 1. Insertamos en la tabla de 'comprobantes'
            comp_data = payload.get("comprobante", {})
            sql_comprobante = """
                INSERT INTO comprobantes (id_usuario, ruta_archivo, fecha_subida, estado)
                VALUES (%s, %s, %s, %s)
            """
            cursor.execute(sql_comprobante, (
                id_usuario, 
                comp_data.get("ruta_archivo", "tickets/desconocido.jpg"), 
                ahora, 
                comp_data.get("estado", "procesado")
            ))
            id_comprobante = cursor.lastrowid # Obtenemos la ID autogenerada del comprobante

            # Helper para mapear la categoría de texto a la ID correspondiente de la tabla
            gasto_data = payload.get("gasto", {})
            cat_nombre = gasto_data.get("categoria_sugerida", "Otro")
            cursor.execute("SELECT id_categoria FROM categorias WHERE nombre = %s LIMIT 1", (cat_nombre,))
            cat_row = cursor.fetchone()
            id_categoria = cat_row["id_categoria"] if cat_row else 1 # ID 1 para "Otro" por defecto

            # 2. Insertamos en la tabla de 'gastos' relacionándolo con el comprobante y categoría
            sql_gasto = """
                INSERT INTO gastos (id_usuario, id_comprobante, id_categoria, comercio, monto, fecha_gasto, notas, fecha_registro)
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
            """
            cursor.execute(sql_gasto, (
                id_usuario,
                id_comprobante,
                id_categoria,
                gasto_data.get("comercio", "Sin nombre"),
                gasto_data.get("monto", 0.0),
                gasto_data.get("fecha_gasto", ahora.date()),
                gasto_data.get("notas", "Insertado desde la app de escritorio"),
                ahora
            ))
            id_gasto = cursor.lastrowid # Obtenemos la ID autogenerada del gasto para los ítems

            # 3. Insertamos cada producto del desglose en la tabla 'items_gasto'
            items = payload.get("items_gasto", [])
            sql_item = """
                INSERT INTO items_gasto (id_gasto, descripcion, cantidad, precio_unitario, subtotal)
                VALUES (%s, %s, %s, %s, %s)
            """
            for item in items:
                cursor.execute(sql_item, (
                    id_gasto,
                    item.get("descripcion", "Producto"),
                    item.get("cantidad", 1.0),
                    item.get("precio_unitario", 0.0),
                    item.get("subtotal", 0.0)
                ))

            # 4. Opcional: Insertar suscripción si el ticket describe un servicio recurrente
            sub_data = payload.get("suscripcion")
            if sub_data and sub_data is not None:
                sql_sub = """
                    INSERT INTO suscripciones (id_usuario, nombre, monto, moneda, frecuencia, vencimiento, alerta, actividad, notas)
                    VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
                """
                # NOTA: Se eliminó id_categoria de esta inserción para coincidir con el diseño de la tabla
                cursor.execute(sql_sub, (
                    id_usuario,
                    sub_data.get("nombre", "Servicio"),
                    sub_data.get("monto", 0.0),
                    sub_data.get("moneda", "ARS"),
                    sub_data.get("frecuencia", "mensual"),
                    sub_data.get("vencimiento", ahora.date()),
                    sub_data.get("alerta_dias", 5),
                    1, # Activa por defecto
                    sub_data.get("notas", "")
                ))

            # 5. Opcional: Insertar notificaciones/alertas sugeridas
            notif_data = payload.get("notificacion")
            if notif_data:
                try:
                    tipo_seguro = "info" 
                    sql_notif = """
                        INSERT INTO notificaciones (id_usuario, tipo, mensaje, leida, fecha_creacion)
                        VALUES (%s, %s, %s, %s, %s)
                    """
                    cursor.execute(sql_notif, (
                        id_usuario,
                        tipo_seguro,
                        notif_data.get("mensaje", "Gasto registrado"),
                        0, # No leída por defecto
                        ahora
                    ))
                except Exception as e_notif:
                    print(f"DEBUG NOTIFICACION: No se pudo guardar la alerta -> {str(e_notif)}")

        # Confirmamos la transacción (Atomicidad)
        connection.commit()
        return {"status": "ok", "message": "Estructura contable persistida en MySQL exitosamente.", "id_gasto": id_gasto}

    except Exception as e:
        print(f"DEBUG ERROR: {str(e)}") 
        if connection:
            connection.rollback() # Ante cualquier fallo, deshacemos todos los inserts parciales
        raise HTTPException(status_code=500, detail=f"Error en persistencia MySQL: {str(e)}")
    finally:
        if connection:
            connection.close()


# ---------------------------------------------------------------------
# 7. SINCRONIZACIÓN Y DESCARGAS
# ---------------------------------------------------------------------

@app.get("/api/v1/usuarios/{id_usuario}/sync")
async def sincronizar_usuario(id_usuario: int):
    """
    Ruta para la sincronización completa del cliente local (Qt SQLite). 
    Descarga categorías, gastos, suscripciones activas y notificaciones
    pendientes de leer correspondientes a un usuario en específico.
    """
    connection = None
    try:
        connection = get_db_connection()
        with connection.cursor() as cursor:
            
            # 1. Validamos la existencia del usuario
            cursor.execute("""
                SELECT id_usuario, nombre, email, actividad
                FROM usuarios
                WHERE id_usuario = %s
            """, (id_usuario,))
            usuario = cursor.fetchone()

            if usuario is None:
                raise HTTPException(status_code=404, detail="Usuario no encontrado")

            # 2. Obtenemos categorías (las globales del sistema y las personalizadas del usuario)
            cursor.execute("""
                SELECT id_categoria, id_usuario, nombre, icono, color
                FROM categorias
                WHERE id_usuario IS NULL OR id_usuario = %s
            """, (id_usuario,))
            categorias = cursor.fetchall()

            # 3. Obtenemos el histórico de gastos
            cursor.execute("""
                SELECT id_gasto, id_usuario, id_categoria, comercio, monto, fecha_gasto, notas
                FROM gastos
                WHERE id_usuario = %s
            """, (id_usuario,))
            gastos = cursor.fetchall()

            # 4. Obtenemos las suscripciones activas (actividad = 1)
            cursor.execute("""
                SELECT id_suscripcion, id_usuario, nombre, monto,
                       moneda, frecuencia, vencimiento, alerta, actividad, notas
                FROM suscripciones
                WHERE id_usuario = %s AND actividad = 1
            """, (id_usuario,))
            suscripciones = cursor.fetchall()

            # 5. Obtenemos notificaciones no leídas (leida = 0)
            cursor.execute("""
                SELECT id_notificacion, id_usuario, tipo, mensaje, leida, fecha_creacion
                FROM notificaciones
                WHERE id_usuario = %s AND leida = 0
            """, (id_usuario,))
            notificaciones = cursor.fetchall()

        # Retorna el consolidado de datos estructurado en formato JSON
        return {
            "usuario": usuario,
            "categorias": categorias,
            "gastos": gastos,
            "suscripciones": suscripciones,
            "notificaciones": notificaciones
        }

    except Exception as e:
        print(f"DEBUG ERROR SYNC: {str(e)}")
        if connection:
            connection.rollback()
        raise HTTPException(status_code=500, detail=f"Error al sincronizar: {str(e)}")
    finally:
        if connection:
            connection.close()


@app.get("/api/v1/usuarios/{id_usuario}/comprobantes/{id_comprobante}")
async def obtener_comprobante(id_usuario: int, id_comprobante: int):
    """
    Busca la ruta del comprobante (ticket) en la base de datos y retorna 
    el archivo binario físico correspondiente para ser visualizado en la app.
    Requiere id_usuario para seguridad y control de acceso.
    """
    connection = None
    try:
        connection = get_db_connection()
        with connection.cursor() as cursor:
            # 1. Buscamos la ruta del archivo en MySQL vinculada al usuario
            sql = """
                SELECT ruta_archivo 
                FROM comprobantes 
                WHERE id_comprobante = %s AND id_usuario = %s
            """
            cursor.execute(sql, (id_comprobante, id_usuario))
            comprobante = cursor.fetchone()

            if not comprobante:
                raise HTTPException(status_code=404, detail="Comprobante no encontrado o acceso denegado")

            ruta_fisica = comprobante["ruta_archivo"]

            # 2. Verificamos la existencia real del archivo físico en el disco del servidor VPS
            if not os.path.exists(ruta_fisica):
                raise HTTPException(status_code=404, detail="El archivo físico se borró o no existe en el servidor")

            # 3. Retornamos el archivo de forma nativa a través de FastAPI
            return FileResponse(ruta_fisica)

    except HTTPException:
        raise
    except Exception as e:
        print(f"DEBUG GET COMPROBANTE: {str(e)}")
        raise HTTPException(status_code=500, detail="Error interno al buscar la imagen")
    finally:
        if connection:
            connection.close()


# ---------------------------------------------------------------------
# 8. GESTIÓN INDEPENDIENTE DE SUSCRIPCIONES
# ---------------------------------------------------------------------

@app.post("/api/v1/suscripciones/guardar")
async def guardar_suscripcion(payload: dict = Body(...)):
    """
    Endpoint para crear o guardar manualmente una suscripción recurrente 
    desde la interfaz sin procesar comprobantes.
    """
    connection = None
    try:
        # Validación obligatoria de que pertenezca a un usuario
        id_usuario = payload.get("id_usuario")
        if not id_usuario:
            raise HTTPException(status_code=400, detail="id_usuario requerido")

        connection = get_db_connection()
        with connection.cursor() as cursor:
            sql = """
                INSERT INTO suscripciones
                (id_usuario, nombre, monto, moneda, frecuencia, vencimiento, alerta, actividad, notas)
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
            """
            # NOTA: Se eliminó el campo id_categoria de esta consulta para coincidir con la estructura actual.
            cursor.execute(sql, (
                id_usuario,
                payload.get("nombre", "Nueva Suscripción"),
                payload.get("monto", 0.0),
                payload.get("moneda", "ARS"),
                payload.get("frecuencia", "mensual"),
                payload.get("vencimiento"),
                payload.get("alerta", 3),
                1, # actividad activa
                payload.get("notas", "")
            ))

        connection.commit()
        return {"status": "ok", "message": "Suscripción guardada"}

    except KeyError as e:
        raise HTTPException(status_code=400, detail=f"Campo obligatorio faltante: {str(e)}")
    except Exception as e:
        if connection: 
            connection.rollback()
        raise HTTPException(status_code=500, detail=str(e))
    finally:
        if connection: 
            connection.close()
        

@app.post("/api/v1/suscripciones/sync")
async def sync_suscripciones_pendientes(payload: dict = Body(...)):
    """
    Recibe una lista de acciones de suscripciones realizadas en local sin conexión 
    (crear, editar, eliminar) y las replica por lotes en la base de datos de producción.
    """
    connection = None
    try:
        id_usuario = payload.get("id_usuario")
        if not id_usuario:
            raise HTTPException(status_code=400, detail="id_usuario requerido")
        
        suscripciones = payload.get("suscripciones", [])
        if not suscripciones:
            return {"status": "ok", "message": "No hay suscripciones pendientes para sincronizar"}

        connection = get_db_connection()
        with connection.cursor() as cursor:
            for sub in suscripciones:
                accion = sub.get("accion_pendiente")
                
                if accion == "crear":
                    # NOTA DE ATENCIÓN: Esta consulta de creación incluye id_categoria (o id_categoria_remota).
                    # Si tu tabla de base de datos final eliminó esta columna, este fragmento podría fallar.
                    sql_crear = """
                        INSERT INTO suscripciones
                        (id_usuario, id_categoria, nombre, monto, moneda, frecuencia, vencimiento, alerta, actividad, notas)
                        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
                    """
                    cursor.execute(sql_crear, (
                        id_usuario,
                        sub.get("id_categoria_remota", 1), # Default a 1 (ej: Otros) si no viene
                        sub.get("nombre", "Sin nombre"),
                        sub.get("monto", 0.0),
                        sub.get("moneda", "ARS"),
                        sub.get("frecuencia", "mensual"),
                        sub.get("vencimiento"),
                        sub.get("alerta", 3),
                        1, # Actividad = 1 (Activo)
                        sub.get("notas", "")
                    ))
                
                elif accion == "editar":
                    id_suscripcion_remota = sub.get("id_suscripcion_remota")
                    if id_suscripcion_remota:
                        sql_editar = """
                            UPDATE suscripciones
                            SET nombre = %s, monto = %s, vencimiento = %s, alerta = %s
                            WHERE id_suscripcion = %s AND id_usuario = %s
                        """
                        cursor.execute(sql_editar, (
                            sub.get("nombre"),
                            sub.get("monto"),
                            sub.get("vencimiento"),
                            sub.get("alerta"),
                            id_suscripcion_remota,
                            id_usuario
                        ))

                elif accion == "eliminar":
                    id_suscripcion_remota = sub.get("id_suscripcion_remota")
                    if id_suscripcion_remota:
                        # Eliminación lógica (soft delete): no borramos, marcamos inactiva (actividad = 0)
                        sql_eliminar = """
                            UPDATE suscripciones
                            SET actividad = 0
                            WHERE id_suscripcion = %s AND id_usuario = %s
                        """
                        cursor.execute(sql_eliminar, (id_suscripcion_remota, id_usuario))

        # Confirmamos los cambios en la base de datos
        connection.commit()
        return {"status": "ok", "message": "Suscripciones sincronizadas exitosamente"}

    except Exception as e:
        if connection:
            connection.rollback()
        print(f"DEBUG SYNC POST: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Error al sincronizar cambios: {str(e)}")
    finally:
        if connection:
            connection.close()