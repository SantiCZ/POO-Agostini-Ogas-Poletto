# POO-Agostini-Ogas-Poletto
proyecto grupal correspondiente a la materia POO
# 💸 SmartExpense & Subs Manager

**Dominio:** fin-ansias.com.ar  

---

## 🧾 1. Resumen del Proyecto

**SmartExpense & Subs Manager** es una plataforma de gestión financiera personal desarrollada en C++.  

Su objetivo principal es eliminar el desorden de los comprobantes físicos (tickets) y evitar el olvido de gastos fijos como suscripciones o servicios.

La aplicación utiliza Inteligencia Artificial para **leer, interpretar y clasificar automáticamente los gastos**, centralizando toda la información financiera del usuario en un único panel de control claro y estructurado.

---

## ⚙️ 2. Funcionalidades Principales

La aplicación funciona como un **contador virtual inteligente y proactivo**.

### 📥 Carga Omnicanal de Gastos
- Ingreso manual de gastos
- Subida de tickets escaneados (imágenes)
- Carga de facturas digitales (PDF)

### 🤖 Procesamiento y OCR Inteligente
- Extracción automática de:
  - Comercio
  - Fecha
  - Monto total
  - Lista de productos
- Eliminación de la carga manual de datos

### 🏷️ Categorización Dinámica
- Clasificación automática en categorías como:
  - Alimentación
  - Ocio
  - Transporte
- Aprendizaje basado en contexto (IA)

### 🔁 Gestión de Suscripciones
- Registro de servicios recurrentes (Netflix, gimnasio, luz, etc.)
- Alertas antes del vencimiento
- Prevención de pagos innecesarios o atrasos

---

## 🧠 3. Arquitectura Orientada a Objetos

El sistema está diseñado con una arquitectura robusta basada en **Programación Orientada a Objetos (POO)**, aplicando herencia, encapsulamiento y reutilización de código.

### 📦 Clase Base: `DocumentoFinanciero`
Contiene:
- Monto
- Fecha de emisión
- Método de pago
- Moneda  

Incluye métodos virtuales comunes a todas las transacciones.

---

### 🧾 Subclase: `GastoTicket`
Hereda de `DocumentoFinanciero` y agrega:
- Comercio
- Sucursal
- Lista de ítems comprados  

Métodos específicos:
- `desglosarItems()`

---

### 🔄 Subclase: `ServicioRecurrente`
Hereda de `DocumentoFinanciero` y agrega:
- Periodicidad
- Fecha de próximo vencimiento
- Estado del pago  

Permite gestionar pagos automáticos y recordatorios.

---

### 🤖 Clase: `MotorIA` (Patrón Singleton)
- Instancia única en todo el sistema
- Responsable de:
  - Construcción de datos en formato JSON
  - Comunicación con APIs externas
  - Procesamiento de respuestas de IA

---

## 🛠️ 4. Stack Tecnológico

El proyecto utiliza un enfoque híbrido moderno:

### 🖥️ Frontend / Motor de Aplicación
- **IDE:** Qt Creator  
- **Framework:** Qt 6 (con archivos `.ui`)  
- **Compilador:** MinGW 64-bit  
- **Networking:** QNetworkAccessManager  

---

### 💻 Backend / Lógica
- **Editor:** Visual Studio Code  
- **Lenguaje:** C++  
- **Asistente IA:** Gemini Code Assist  

---

### 🤖 Inteligencia Artificial
- **Motor:** API de Gemini 1.5 Pro  
- **Funcionalidad:**
  - Procesamiento de tickets
  - Extracción de datos
  - Generación de JSON estructurado

---

### ☁️ Infraestructura (Servidor VPS)

- **Sistema Operativo:** Ubuntu 24.04 LTS  
- **Hardware:**
  - 6 vCPU
  - 12 GB RAM
  - 100 GB NVMe
  - 300 Mbit/s de ancho de banda  
- **Extras:**
  - Sistema de snapshots (respaldo)
  - Backend intermediario entre app y API de IA
  - Almacenamiento de base de datos

---

 activa con IA  

---
