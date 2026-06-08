#pragma once

#include <QString>

/*
 * StyleManager
 * Responsabilidad de clase:
 * Clase utilitaria que concentra los estilos QSS de la aplicacion.
 * Evita duplicar cadenas de estilos en cada pantalla y mantiene una paleta
 * visual consistente.
 *
 * Clases abstractas:
 * No es abstracta; es una clase utilitaria con metodos static y constantes.
 */
class StyleManager {
public:
    // Hojas de estilo reutilizables por tipo de componente.
    static QString appStyleSheet();
    static QString cardStyle(const QString &accent = "#4ADE80");
    static QString primaryButtonStyle();
    static QString secondaryButtonStyle();
    static QString dangerButtonStyle();
    static QString inputStyle();
    static QString sidebarStyle();
    static QString logoutButtonStyle();  // nuevo metodo

    // Paleta de colores base de la interfaz.
    static constexpr const char* BG_DARK    = "#0F1117";
    static constexpr const char* BG_CARD    = "#1A1D27";
    static constexpr const char* BG_SURFACE = "#21253A";
    static constexpr const char* ACCENT     = "#4ADE80";
    static constexpr const char* ACCENT2    = "#818CF8";
    static constexpr const char* TEXT_PRI   = "#F1F5F9";
    static constexpr const char* TEXT_SEC   = "#94A3B8";
    static constexpr const char* BORDER     = "#2E3347";
    static constexpr const char* DANGER     = "#F87171";
    static constexpr const char* WARNING    = "#FBBF24";
};
