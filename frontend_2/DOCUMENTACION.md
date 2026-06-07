# Documentacion tecnica por criterios

## 1. Comentario de responsabilidad de la clase

Cada clase principal tiene un comentario de responsabilidad en su archivo `.h`.
La idea es que, antes de leer los metodos, se entienda que papel cumple dentro
del sistema.

- `DataManager`: coordina datos, red, SQLite y senales de actualizacion.
- `adminDB`: encapsula SQLite y las consultas SQL.
- `MainWidget`: coordina la ventana principal, paginas, notificaciones y cierre.
- `LoginDialog`: maneja autenticacion y registro.
- `DashboardPage`: muestra resumen financiero y accesos rapidos.
- `TicketsPage`: lista, filtra, carga y elimina tickets.
- `SubscriptionsPage`: gestiona suscripciones recurrentes.
- `ReportsPage`: muestra reportes y graficos.
- `Sidebar`: maneja navegacion lateral y logout.
- `StyleManager`: concentra estilos QSS y paleta visual.

## 2. Justificar atributos importantes

Se documentaron atributos que sostienen estado relevante:

- `DataManager::networkManager`: se mantiene como miembro porque las peticiones
  HTTP son asincronicas y deben vivir mientras la app procesa respuestas.
- `DataManager::estadoActual`: permite informar a la interfaz si la app espera,
  envia una foto, sincroniza, tuvo exito o fallo.
- `DataManager::m_db`: es el punto de acceso a SQLite desde la capa de datos.
- `DataManager::m_notificaciones`: evita reconstruir notificaciones desde cero
  cada vez que cambia la UI.
- `adminDB::CONNECTION_NAME`: asegura una conexion SQLite compartida.
- `adminDB::db`: guarda la conexion local concreta.
- `UploadTicketDialog::m_iaJsonResult`: conserva la respuesta IA hasta convertir
  JSON en modelos C++.

## 3. Justificar herencia

El proyecto aplica herencia en dos niveles.

Herencia de dominio:

- `Ticket : public MovimientoBase`
- `Suscripcion : public MovimientoBase`

Ambas clases reutilizan `id`, `monto`, `fecha`, `categoria` y `descripcion`,
pero agregan atributos propios.

Herencia con Qt:

- `MainWidget`, `DashboardPage`, `TicketsPage`, `SubscriptionsPage`,
  `ReportsPage`, `Sidebar` heredan de `QWidget`.
- `LoginDialog`, `UploadTicketDialog`, `AddSubscriptionDialog` heredan de
  `QDialog`.
- `TicketCard`, `SubCard`, `StatCard` heredan de `QFrame`.
- `SidebarButton` hereda de `QPushButton`.
- `DataManager` y `adminDB` heredan de `QObject`.

Esta herencia permite integrar widgets en layouts, usar eventos de Qt y conectar
signals/slots.

## 4. Justificar clases abstractas

Actualmente el proyecto no define clases abstractas propias con metodos puros
(`= 0`).

Se dejo explicado en comentarios que clases como `DataManager`, `adminDB` y
`MovimientoBase` no son abstractas porque tienen uso concreto:

- `DataManager` se instancia como Singleton.
- `adminDB` implementa operaciones reales de SQLite.
- `MovimientoBase` actua como modelo base simple para compartir atributos.

Si el proyecto creciera, podria tener sentido una clase abstracta como
`Repository` o `Exporter`, pero hoy no hace falta.

## 5. Justificar polimorfismo

El polimorfismo aparece al sobrescribir metodos virtuales de Qt:

- `AddSubscriptionDialog::accept()`
- `UploadTicketDialog::accept()`
- `MainWidget::closeEvent()`
- `BarChart::paintEvent()`

Qt llama esos metodos mediante punteros o referencias a clases base
(`QDialog`, `QWidget`). Cada clase hija redefine el comportamiento segun su
responsabilidad.

## 6. Explicar los SIGNAL y SLOT

El proyecto usa signals/slots para desacoplar clases.

Ejemplos:

- `DataManager` emite `ticketsChanged()` y `suscripcionesChanged()` para avisar
  que las pantallas deben refrescarse.
- `DataManager` emite `loginExitoso()` o `loginFallido()` luego de una respuesta
  de red.
- `Sidebar` emite `pageChanged()`, `logoutRequested()` y
  `uploadTicketRequested()` para que `MainWidget` decida que hacer.
- `QNetworkAccessManager::finished` se conecta al slot
  `DataManager::onRespuestaRecibida()`.

Esto permite manejar red, botones y actualizaciones sin que las clases queden
fuertemente acopladas.

## 7. Explicar paintEvent

`BarChart::paintEvent()` en `reportspage.cpp` dibuja el grafico de barras
manualmente con `QPainter`.

Qt llama automaticamente a `paintEvent()` cuando el widget necesita repintarse.
El metodo calcula dimensiones, proporcion de cada barra, colores y etiquetas.
Esto evita depender de una libreria externa para un grafico simple.

## 8. Explicar eventos de mouse

No se sobrescriben eventos como `mousePressEvent()` porque el proyecto aprovecha
los widgets de Qt.

Ejemplos:

- Los botones emiten `clicked()` cuando reciben un click de mouse.
- `Sidebar::onProfileClicked()` responde al click del boton de perfil.
- `profileInner->setAttribute(Qt::WA_TransparentForMouseEvents)` permite que
  los labels internos no capturen el click y que el evento llegue al boton padre.

## 9. Explicar SQLite

SQLite es la base local de la aplicacion.

`adminDB`:

- abre o crea el archivo `tasty_alcancia.db`;
- crea tablas;
- usa `QSqlQuery` para ejecutar SQL;
- centraliza inserciones y actualizaciones;
- usa una conexion compartida mediante `CONNECTION_NAME`;
- sincroniza JSON recibido del servidor hacia tablas locales.

`DataManager` usa `adminDB` para consultar y guardar tickets, suscripciones,
usuarios y notificaciones sin duplicar SQL en la interfaz.

## 10. Explicar validaciones

Las validaciones principales estan en los dialogos:

- `AddSubscriptionDialog::accept()` verifica que el nombre de servicio no este
  vacio.
- `UploadTicketDialog::accept()` verifica comercio, monto mayor a cero y sesion
  activa.
- `LoginDialog` valida campos de login/registro, incluyendo formato de email y
  coincidencia de contrasenas.

Cuando una validacion falla, se muestra un mensaje y el dialogo no se cierra.

## 11. Explicar exportaciones

Actualmente no hay exportaciones activas de reportes a archivo.

En `ReportsPage` se documento que el boton de exportar fue eliminado. Esto es
importante porque evita presentar como implementada una funcion inexistente.

Si se agregara exportacion en el futuro, podria implementarse como:

- CSV para datos tabulares;
- PDF para reportes visuales;
- imagen para graficos.

Esa funcionalidad deberia quedar encapsulada en una clase propia, por ejemplo
`ReportExporter`, para no mezclar UI con escritura de archivos.
