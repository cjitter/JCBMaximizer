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
#include <juce_graphics/juce_graphics.h>
#include "../Windows/DarkThemeColors.h"
#include "CustomSlider.h"

//==============================================================================
// CLASE PRINCIPAL DEL TRIM SLIDER
//==============================================================================

/**
 * TrimSlider especializado para controles de trim de input/output
 * 
 * Diseñado específicamente para sliders verticales de trim que se
 * superponen sobre los medidores de nivel. Características:
 * - Estilo vertical linear con indicador visual central en 0 dB
 * - Rango típico de -12dB a +12dB con incrementos de 0.1dB
 * - Doble-click para reset a 0.0dB
 * - Cursor especializado up/down resize
 * - Callback para click derecho (reset u otras acciones)
 * - Estilo visual integrado con DarkTheme
 */



class TrimSlider : public CustomSlider
{
public:
    //==========================================================================
    // CONSTRUCTOR Y CONFIGURACIÓN INICIAL
    //==========================================================================
    
    TrimSlider()
    {
        // Configuración básica del slider vertical
        setSliderStyle(juce::Slider::LinearVertical);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
        
        // Configuración de rango y valores
        setRange(-12.0, 12.0, 0.1);
        setValue(0.0);
        setDoubleClickReturnValue(true, 0.0);
        
        // Configuración de popup y colores
        setPopupDisplayEnabled(false, false, nullptr);
        setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::thumbColourId, juce::Colours::white.withAlpha(0.9f));  // Blanco consistente
        
        // Configuración del TextBox integrado
        setTextBoxIsEditable(true);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        //setTextValueSuffix(" dB");
        
    }
    
    //==========================================================================
    // OVERRIDE DE RENDERING PERSONALIZADO
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto sliderPos = getPositionOfValue(getValue());
        
        // Dibujar línea central en 0 dB como referencia visual
        float centerY = getPositionOfValue(0.0);
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawHorizontalLine(centerY, bounds.getX() + 2, bounds.getRight() - 2);
        
        // Dibujar indicador actual - Blanco para consistencia visual
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        auto thumbBounds = juce::Rectangle<float>(bounds.getWidth() - 8, 4).withCentre(juce::Point<float>(bounds.getCentreX(), sliderPos));
        g.fillRoundedRectangle(thumbBounds, 2.0f);
        
        // Nota: El valor se muestra en el TextBox integrado de JUCE
    }
    
    //==========================================================================
    // OVERRIDES DE EVENTOS DE MOUSE
    //==========================================================================
    
    void mouseEnter(const juce::MouseEvent&) override
    {
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    }
    
    void mouseExit(const juce::MouseEvent&) override
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        if (event.mods.isPopupMenu())
        {
            // Manejar click derecho - típicamente para reset u opciones
            if (onRightClick)
                onRightClick();
        }
        else
        {
            // Comportamiento normal del slider para click izquierdo
            CustomSlider::mouseDown(event);
        }
    }
    
    
    //==========================================================================
    // CALLBACK PARA CLICK DERECHO
    //==========================================================================
    
    /**
     * Callback que se ejecuta cuando se hace click derecho sobre el slider
     * Típicamente usado para reset a valor por defecto o menú contextual
     */
    std::function<void()> onRightClick;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrimSlider)
};

