//==============================================================================
// INCLUDES
//==============================================================================
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
// CLASE PRINCIPAL DEL CUSTOM TOOLTIP
//==============================================================================

/**
 * CustomTooltip para visualización de ayuda contextual en tiempo real
 * 
 * Sistema de tooltips personalizado que muestra texto de ayuda para componentes.
 * Características principales:
 * - Detección automática de componentes con TooltipClient interface
 * - Verificación de ventana padre para evitar tooltips cruzados
 * - Escala ajustable para diferentes tamaños de display
 * - Timer optimizado para performance (8Hz refresh rate)
 * - Fondo semitransparente sutil con texto centrado
 * 
 * Basado en tooltip_component por François Becker
 * https://github.com/francoisbecker/fb-utils/blob/master/include/fbu/tooltip_component.hpp
 * 
 * MIT License - Copyright (c) 2017 François Becker
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
class CustomTooltip : public juce::Component,
                      private juce::Timer
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CustomTooltip()
    {
        setInterceptsMouseClicks(false, false);  // No interceptar mouse events
        startTimerHz(8);                         // Verificar 8 veces por segundo
    }
    
    ~CustomTooltip() override
    {
        stopTimer();
    }
    
    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        if (currentText.isEmpty())
            return;
            
        auto bounds = getLocalBounds().toFloat();
        
        // Fondo semitransparente sutil
        g.setColour(juce::Colours::black.withAlpha(0.1f));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Texto blanco escalado
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(14.0f * scaleFactor)));
        g.drawFittedText(currentText, getLocalBounds().reduced(8), 
                         juce::Justification::centred, 3);
    }
    
    //==========================================================================
    // MÉTODOS PÚBLICOS DE CONFIGURACIÓN
    //==========================================================================
    
    void setScaleFactor(float scale)
    {
        scaleFactor = scale;
        repaint();
    }
    
    void showTip(const juce::String& text)
    {
        if (currentText != text)
        {
            currentText = text;
            repaint();
        }
    }
    
    void clearTip()
    {
        showTip("");
    }
    
    void setTooltipsEnabled(bool shouldBeEnabled)
    {
        tooltipsEnabled = shouldBeEnabled;
        if (!tooltipsEnabled)
            clearTip();
    }
    
    bool areTooltipsEnabled() const { return tooltipsEnabled; }
    
private:
    //==========================================================================
    // OVERRIDE DE TIMER
    //==========================================================================
    
    void timerCallback() override
    {
        if (!tooltipsEnabled)
        {
            clearTip();
            return;
        }
        
        // Obtener componente bajo el mouse
        auto& desktop = juce::Desktop::getInstance();
        auto mouseSource = desktop.getMainMouseSource();
        
        if (!mouseSource.isMouse())
        {
            clearTip();
            return;
        }
        
        auto* comp = mouseSource.getComponentUnderMouse();
        
        // Verificar si el componente está en la misma ventana que este tooltip
        if (comp && isInSameWindow(comp))
        {
            // Intentar obtener tooltip de la interfaz TooltipClient
            if (auto* client = dynamic_cast<juce::TooltipClient*>(comp))
            {
                if (!comp->isCurrentlyBlockedByAnotherModalComponent())
                {
                    showTip(client->getTooltip());
                    return;
                }
            }
        }
        
        clearTip();
    }
    
    //==========================================================================
    // MÉTODOS PRIVADOS DE UTILIDAD
    //==========================================================================
    
    bool isInSameWindow(juce::Component* comp) const
    {
        if (!comp) return false;
        
        auto* myWindow = getTopLevelComponent();
        auto* compWindow = comp->getTopLevelComponent();
        
        return myWindow && myWindow == compWindow;
    }
    
    //==========================================================================
    // VARIABLES DE ESTADO
    //==========================================================================
    
    juce::String currentText;         // Texto actual del tooltip
    float scaleFactor = 1.0f;         // Factor de escala para el display
    bool tooltipsEnabled = true;      // Estado de tooltips habilitados
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomTooltip)
};