//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBMaximizer and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================
#pragma once

//==============================================================================
// INCLUDES
//==============================================================================

#include <juce_gui_basics/juce_gui_basics.h>
#include "DarkThemeColors.h"
#include <vector>

//==============================================================================
// CLASE PRINCIPAL DEL CREDITS WINDOW
//==============================================================================

/**
 * CreditsWindow para mostrar información de créditos con multiidioma
 * 
 * Ventana de créditos especializada que muestra información del plugin, 
 * desarrolladores, tecnologías y bibliografía. Características principales:
 * - Sistema bilingüe (español/inglés) con botón de cambio de idioma
 * - Links clickeables que abren en navegador externo
 * - Imagen de fondo del diagrama con transparencia ajustable
 * - Escalado dinámico de fuente y elementos según tamaño de ventana
 * - Detección de hover en links con cambio de cursor
 * - Formato de terminal monocromático para estética retro
 */
class CreditsWindow : public juce::Component
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CreditsWindow(const juce::String& pluginFormat = "");
    ~CreditsWindow() override;
    
    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    
    //==========================================================================
    // MÉTODOS PÚBLICOS DE ACCESO
    //==========================================================================
    
    bool getIsEnglish() const { return isEnglish; }  // Obtener estado del idioma actual
    
    //==========================================================================
    // CALLBACKS
    //==========================================================================
    
    std::function<void()> onClose;  // Callback cuando se cierra la ventana
    
private:
    //==========================================================================
    // ESTRUCTURA PARA MANEJO DE LINKS
    //==========================================================================
    
    struct LinkInfo
    {
        juce::String text;                    // Texto del link
        juce::String url;                     // URL de destino
        juce::Rectangle<int> bounds;          // Área clickeable del link
    };
    
    //==========================================================================
    // VARIABLES DE CONTENIDO Y UI
    //==========================================================================
    
    juce::String fullText;                    // Texto completo de créditos generado
    juce::Image backgroundImage;              // Imagen de fondo del diagrama
    std::vector<LinkInfo> links;              // Lista de links clickeables
    juce::TextButton langButton;              // Botón de cambio de idioma
    
    //==========================================================================
    // VARIABLES DE ESTADO Y CONFIGURACIÓN
    //==========================================================================
    
    bool isEnglish = false;                   // Estado del idioma (false = español, true = inglés)
    int hoveredLinkIndex = -1;                // Índice del link actualmente bajo hover
    juce::String pluginFormat;               // Formato del plugin (VST3, AU, AAX)
    
    //==========================================================================
    // MÉTODOS PRIVADOS DE UTILIDAD
    //==========================================================================
    
    void buildFullCreditsText();             // Construir texto según idioma actual
    void buildFullCreditsTextSpanish();     // Generar texto en español
    void buildFullCreditsTextEnglish();     // Generar texto en inglés
    void toggleLanguage();                   // Cambiar entre español e inglés
    juce::Colour getTerminalColour() const;  // Obtener color del texto (terminal style)
    juce::Rectangle<int> getLinkBounds() const;  // Calcular bounds de links (legacy)
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreditsWindow)
};

//==============================================================================
// OVERLAY FLOTANTE PARA CREDITS WINDOW
//==============================================================================

/**
 * CreditsOverlay para ventana flotante de créditos con fondo translúcido
 * 
 * Overlay que contiene una CreditsWindow y maneja la lógica de ventana modal.
 * Características principales:
 * - Fondo translúcido para overlay modal
 * - Click fuera de la ventana para cerrar (opcional, gestionado por CreditsWindow)
 * - Delegación de configuración al CreditsWindow interno
 * - Acceso directo a la ventana de créditos para obtener estado del idioma
 */
class CreditsOverlay : public juce::Component
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CreditsOverlay(const juce::String& pluginFormat = "");
    
    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    //==========================================================================
    // MÉTODOS PÚBLICOS DE ACCESO
    //==========================================================================
    
    CreditsWindow& getCreditsWindow() { return creditsWindow; }  // Acceso a ventana interna
    
    //==========================================================================
    // CALLBACKS
    //==========================================================================
    
    std::function<void()> onClose;  // Callback para cerrar overlay
    
private:
    //==========================================================================
    // COMPONENTES DE UI
    //==========================================================================
    
    CreditsWindow creditsWindow;  // Ventana de créditos interna
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreditsOverlay)
};