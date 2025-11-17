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

//==============================================================================
// LOOK AND FEEL PERSONALIZADO PARA SCROLLBARS
//==============================================================================

/**
 * CustomScrollBarLookAndFeel para scrollbars con color dinámico
 * 
 * LookAndFeel personalizado que permite cambiar el color de los scrollbars
 * dinámicamente según el contexto (halo color de las ventanas de código).
 * Características principales:
 * - Color de scrollbar configurable en tiempo real
 * - Estilo moderno con esquinas redondeadas
 * - Estados visuales para hover y mouse down
 * - Integración perfecta con el tema oscuro del plugin
 */
class CustomScrollBarLookAndFeel : public juce::LookAndFeel_V4
{
public:
    //==========================================================================
    // MÉTODOS PÚBLICOS DE CONFIGURACIÓN
    //==========================================================================
    
    void setScrollBarColour(const juce::Colour& colour)
    {
        scrollBarColour = colour;
    }
    
    //==========================================================================
    // OVERRIDE DE LOOKANDFEEL
    //==========================================================================
    
    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar,
                      int x, int y, int width, int height,
                      bool isScrollbarVertical,
                      int thumbStartPosition,
                      int thumbSize,
                      bool isMouseOver,
                      bool isMouseDown) override
    {
        // Background translúcido
        g.fillAll(juce::Colours::black.withAlpha(0.3f));
        
        // Track del scrollbar
        juce::Rectangle<int> trackRect;
        if (isScrollbarVertical)
            trackRect = juce::Rectangle<int>(x, y, width, height);
        else
            trackRect = juce::Rectangle<int>(x, y, width, height);
            
        g.setColour(scrollBarColour.withAlpha(0.2f));
        g.fillRect(trackRect);
        
        // Thumb con estados de interacción
        if (thumbSize > 0)
        {
            juce::Rectangle<int> thumbRect;
            if (isScrollbarVertical)
                thumbRect = juce::Rectangle<int>(x + 2, thumbStartPosition + 2, width - 4, thumbSize - 4);
            else
                thumbRect = juce::Rectangle<int>(thumbStartPosition + 2, y + 2, thumbSize - 4, height - 4);
            
            g.setColour(scrollBarColour.withAlpha(isMouseDown ? 0.9f : (isMouseOver ? 0.8f : 0.7f)));
            g.fillRoundedRectangle(thumbRect.toFloat(), 2.0f);
        }
    }
    
private:
    //==========================================================================
    // VARIABLES DE ESTADO
    //==========================================================================
    
    juce::Colour scrollBarColour = DarkTheme::accent;  // Color dinámico del scrollbar
};

//==============================================================================
// CLASE PRINCIPAL DEL CODE WINDOW
//==============================================================================

/**
 * CodeWindow para visualización de código con terminal estilo Matrix
 * 
 * Ventana de código especializada que muestra contenido de código fuente con
 * estilo visual de terminal. Características principales:
 * - Editor de texto de solo lectura con scroll personalizado
 * - Colores dinámicos basados en halo color configurable
 * - Botón de copy to clipboard integrado
 * - Fuente monoespaciada optimizada para código
 * - Timer para feedback visual del botón copy
 * - Integración completa con DarkTheme del plugin
 */
class CodeWindow : public juce::Component,
                   private juce::Timer
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CodeWindow();
    ~CodeWindow() override;
    
    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    //==========================================================================
    // MÉTODOS PÚBLICOS DE CONFIGURACIÓN
    //==========================================================================
    
    void setCode(const juce::String& code, const juce::String& title);  // Establecer código a mostrar
    void setHaloColour(const juce::Colour& colour);                     // Color del halo dinámico
    juce::Colour getHaloColour() const { return haloColour; }          // Obtener color del halo
    
    //==========================================================================
    // CALLBACKS
    //==========================================================================
    
    std::function<void()> onClose;  // Callback cuando se cierra la ventana
    
private:
    //==========================================================================
    // OVERRIDE DE TIMER
    //==========================================================================
    
    void timerCallback() override;  // Timer callback para restaurar texto del botón
    
    //==========================================================================
    // MÉTODOS PRIVADOS DE UTILIDAD
    //==========================================================================
    
    juce::Colour getCodeColour() const;  // Obtener color del código según halo
    void copyToClipboard();             // Copiar código al clipboard
    
    //==========================================================================
    // VARIABLES DE CONTENIDO
    //==========================================================================
    
    juce::String codeText;       // Texto del código a mostrar
    juce::String windowTitle;    // Título de la ventana
    
    //==========================================================================
    // COMPONENTES DE UI
    //==========================================================================
    
    juce::TextEditor codeEditor;                        // Editor de texto con scroll
    juce::TextButton copyButton{"Copy Code"};           // Botón para copiar código
    CustomScrollBarLookAndFeel scrollBarLookAndFeel;    // LookAndFeel personalizado para scrollbars
    
    //==========================================================================
    // VARIABLES DE ESTADO
    //==========================================================================
    
    juce::Colour haloColour = DarkTheme::accent;  // Color del halo dinámico
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CodeWindow)
};

//==============================================================================
// OVERLAY FLOTANTE PARA CODE WINDOW
//==============================================================================

/**
 * CodeOverlay para ventana flotante de código con fondo translúcido
 * 
 * Overlay que contiene una CodeWindow y maneja la lógica de ventana modal.
 * Características principales:
 * - Fondo translúcido para overlay modal
 * - Click fuera de la ventana para cerrar
 * - Delegación de configuración al CodeWindow interno
 * - Integración seamless con el sistema de colores dinámicos
 */
class CodeOverlay : public juce::Component
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CodeOverlay();
    
    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    //==========================================================================
    // MÉTODOS PÚBLICOS DE CONFIGURACIÓN
    //==========================================================================
    
    void setCode(const juce::String& code, const juce::String& title);  // Establecer código a mostrar
    void setHaloColour(const juce::Colour& colour);                     // Color del halo dinámico
    
    //==========================================================================
    // CALLBACKS
    //==========================================================================
    
    std::function<void()> onClose;  // Callback para cerrar overlay
    
private:
    //==========================================================================
    // COMPONENTES DE UI
    //==========================================================================
    
    CodeWindow codeWindow;  // Ventana de código interna
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CodeOverlay)
};