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

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "../../Helpers/UndoableParameterHelper.h"
#include "../Windows/DarkThemeColors.h"

//==============================================================================
// CLASE PRINCIPAL DEL CUSTOM SLIDER
//==============================================================================

/**
 * Custom Slider con Look and Feel integrado
 * Contiene tanto la funcionalidad del slider como su styling visual
 * Diseñado para knobs rotativos con detección automática de tamaño
 * basada en el nombre del componente (trim, lookahead, hpf, lpf = pequeños)
 */
class CustomSlider : public juce::Slider
{
public:
    //==========================================================================
    // CONSTRUCTOR
    //==========================================================================
    
    CustomSlider(const juce::String& name = {}) : juce::Slider(name) {}

    //==========================================================================
    // LOOK AND FEEL INTEGRADO
    //==========================================================================
    
    /**
     * Clase Look and Feel integrada
     * LookAndFeel unificado para sliders rotativos pequeños y grandes
     * Determina automáticamente el tamaño basado en el nombre del componente
     * Elimina la necesidad del enum Size obsoleto
     */
    class LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        LookAndFeel() = default;
        
        void drawRotarySlider (juce::Graphics& g,
                               int x,
                               int y,
                               int width,
                               int height,
                               float sliderPos,
                               const float rotaryStartAngle,
                               const float rotaryEndAngle,
                               juce::Slider& slider) override
        {
            // Determinar tamaño automáticamente basado en nombre del componente
            bool isSmall = isSmallSlider(slider);
            
            // Parámetros basados en el tamaño detectado
            const float reduction = isSmall ? 6.0f : 8.0f;
            const float lineWidth = isSmall ? 4.0f : 6.0f;
            const float lineWidthFactor = isSmall ? 0.35f : 0.4f;
            const float thumbSize = isSmall ? lineWidth * 0.7f : 6.0f;
            
            // Setup común para ambos tamaños
            auto size = static_cast<float>(juce::jmin(width, height));
            auto centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
            auto centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
            auto bounds = juce::Rectangle<float>(size, size).withCentre(juce::Point<float>(centreX, centreY)).reduced(reduction);
            auto radius = bounds.getWidth() / 2.0f;
            auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto lineW = juce::jmin(lineWidth, radius * lineWidthFactor);
            auto arcRadius = radius - lineW * 0.5f;
            
            // Determinar color de acento basado en el tipo de componente
            juce::Colour accentColour = juce::Colours::white;  // Por defecto
            
            // Obtener nombre/ID del componente para asignación de color
            juce::String name = slider.getName().toLowerCase();
            juce::String id = slider.getComponentID().toLowerCase();
            
            // FILTROS (Blanco por defecto, pero verificar color custom)
            if (name == "hpf" || id == "hpf" || name == "lpf" || id == "lpf") {
                // Verificar si se ha establecido un color custom vía setColour()
                // Siempre usar el color establecido en el componente slider
                accentColour = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
            }
            // DETECTOR (Verde) - #66FF66
            else if (name == "detect" || id == "detect" ||
                     name == "attack" || id == "attack" || name == "release" || id == "release" ||
                     name == "react" || id == "react" || name == "smo" || id == "smo" ||
                     name == "hold" || id == "hold") {
                accentColour = juce::Colour(0xFF66FF66);
            }
            // AR (Naranja) - #FFB366
            else if (name == "speed" || id == "speed") {
                accentColour = juce::Colour(0xFFFFB366);
            }
            // COMPRESSOR CORE (Púrpura) - #B366FF (más vivo)
            else if (name == "thd" || id == "thd" || name == "ratio" || id == "ratio" || 
                     name == "knee" || id == "knee" || name == "range" || id == "range") {
                accentColour = juce::Colour(0xFFB366FF);
            }
            // AUTO-MAKEUP (Rojo) - #FF6666
            else if (name == "again" || id == "again" || name == "parall" || id == "parall" ||
                     name == "clipper" || id == "clipper") {
                accentColour = juce::Colour(0xFFFF6666);
            }
            // OUTPUT STAGE (Azul) - #6495ED
            else if (name == "drywet" || id == "drywet" || name == "lookahead" || id == "lookahead" ||
                     name == "trim" || id == "trim" || name == "makeup" || id == "makeup") {
                accentColour = juce::Colour(0xFF6495ED);
            }
            
            // Dibujar cuerpo del knob - estilos diferentes para grande vs pequeño
            if (!isSmall)
            {
                // Knob grande - relleno sólido con gradient sutil
                auto gradient = juce::ColourGradient(DarkTheme::backgroundLight.withAlpha(0.4f), 
                                              bounds.getCentreX(), bounds.getCentreY() - radius * 0.3f,
                                              DarkTheme::backgroundDark.withAlpha(0.7f), 
                                              bounds.getCentreX(), bounds.getCentreY() + radius * 0.3f, false);
                g.setGradientFill(gradient);
                g.fillEllipse(bounds);
                
                // Agregar gradient de color sutil encima
                auto colorGradient = juce::ColourGradient(accentColour.withAlpha(0.15f),
                                                  bounds.getCentreX(), bounds.getCentreY() - radius * 0.5f,
                                                  accentColour.withAlpha(0.05f),
                                                  bounds.getCentreX(), bounds.getCentreY() + radius * 0.5f, false);
                g.setGradientFill(colorGradient);
                g.fillEllipse(bounds.reduced(2.0f));
                
                // Label interno para knobs específicos
                juce::String knobLetter = getKnobLabel(slider);
                
                if (knobLetter.isNotEmpty()) {
                    g.setColour(DarkTheme::textSecondary.withAlpha(0.4f));
                    g.setFont(juce::Font(juce::FontOptions(radius * 0.6f)).withStyle(juce::Font::bold));
                    g.drawText(knobLetter, bounds.toNearestInt(), juce::Justification::centred);
                }
            }
            else
            {
                // Knob pequeño - cuerpo transparente solo con borde
                g.setColour(DarkTheme::border.withAlpha(0.6f));
                g.drawEllipse(bounds.reduced(1.0f), 1.0f);
                
                // Arco de fondo del track para knobs pequeños
                juce::Path backgroundArc;
                backgroundArc.addCentredArc(bounds.getCentreX(),
                                           bounds.getCentreY(),
                                           arcRadius,
                                           arcRadius,
                                           0.0f,
                                           rotaryStartAngle,
                                           rotaryEndAngle,
                                           true);
                
                g.setColour(juce::Colours::darkred.withAlpha(0.4f));
                g.strokePath(backgroundArc, juce::PathStrokeType(lineW * 0.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            
            // Dibujar borde para knobs grandes
            if (!isSmall) {
                g.setColour(accentColour.withAlpha(0.3f));
                g.drawEllipse(bounds, 1.5f);
            }
            
            // Dibujar arco de valor para knobs pequeños
            if (isSmall && slider.isEnabled())
            {
                bool isLPF = (name == "lpf" || id == "lpf");
                
                if (isLPF) {
                    // LPF: Color desde posición actual hasta el FINAL (lógica invertida)
                    juce::Path valueArc;
                    valueArc.addCentredArc(bounds.getCentreX(),
                                          bounds.getCentreY(),
                                          arcRadius,
                                          arcRadius,
                                          0.0f,
                                          toAngle,
                                          rotaryEndAngle,
                                          true);
                    
                    g.setColour(accentColour);
                    g.strokePath(valueArc, juce::PathStrokeType(lineW * 0.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                } else {
                    // Comportamiento normal para HPF y otros controles
                    juce::Path valueArc;
                    valueArc.addCentredArc(bounds.getCentreX(),
                                          bounds.getCentreY(),
                                          arcRadius,
                                          arcRadius,
                                          0.0f,
                                          rotaryStartAngle,
                                          toAngle,
                                          true);
                    
                    g.setColour(accentColour);
                    g.strokePath(valueArc, juce::PathStrokeType(lineW * 0.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }
            }
            
            // Dibujar indicador thumb
            juce::Point<float> thumbPoint(bounds.getCentreX() + arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi),
                                         bounds.getCentreY() + arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi));
            
            if (isSmall) {
                // Thumb pequeño con efecto glow
                g.setColour(accentColour.withAlpha(0.4f));
                g.fillEllipse(juce::Rectangle<float>(thumbSize * 1.8f, thumbSize * 1.8f).withCentre(thumbPoint));
                
                g.setColour(accentColour);
                g.fillEllipse(juce::Rectangle<float>(thumbSize, thumbSize).withCentre(thumbPoint));
                
                // Punto central para knobs pequeños
                g.setColour(DarkTheme::textPrimary.withAlpha(0.8f));
                g.fillEllipse(juce::Rectangle<float>(2.0f, 2.0f).withCentre(bounds.getCentre()));
            } else {
                // Thumb simple para knobs grandes
                g.setColour(accentColour);
                g.fillEllipse(juce::Rectangle<float>(thumbSize, thumbSize).withCentre(thumbPoint));
            }
        }
    
    private:
        //======================================================================
        // MÉTODOS HELPER PRIVADOS
        //======================================================================
        
        /**
         * Determina si un slider debe ser pequeño basado en su nombre
         * Todos los demás (lookahead, hpf, lpf, etc.) son grandes
         */
        bool isSmallSlider(const juce::Slider& slider) const
        {
            juce::String name = slider.getName().toLowerCase();
            juce::String id = slider.getComponentID().toLowerCase();
            
            return (name == "trim" || id == "trim");
        }
        
        /**
         * Obtiene el label interno para knobs grandes
         * Mapea nombres de componentes a sus etiquetas internas
         */
        juce::String getKnobLabel(const juce::Slider& slider) const
        {
            juce::String name = slider.getName().toLowerCase();
            juce::String id = slider.getComponentID().toLowerCase();
            
            if (name == "attack" || id == "attack") return "ATK";
            if (name == "release" || id == "release") return "REL";
            if (name == "hold" || id == "hold") return "HOLD";
            if (name == "react" || id == "react") return "REACT";
            if (name == "hpf" || id == "hpf") return "HPF";
            if (name == "lpf" || id == "lpf") return "LPF";
            if (name == "thd" || id == "thd") return "GAIN";
            if (name == "ratio" || id == "ratio") return "RATIO";
            if (name == "range" || id == "range") return "RANGE";
            if (name == "knee" || id == "knee") return "KNEE";
            if (name == "trim" || id == "trim") return "TRIM";
            if (name == "makeup" || id == "makeup") return "MAKE";
            if (name == "drywet" || id == "drywet") return "D/W";
            if (name == "lookahead" || id == "lookahead") return "LA";
            if (name == "clipper" || id == "clipper") return "CLIP";
            if (name == "a-rel" || id == "autorel") return "A-REL";
            if (name == "speed" || id == "speed") return "AR";
            if (name == "delta" || id == "delta") return "DELTA";
            if (name == "parall" || id == "parall") return "PARAL";
            if (name == "again" || id == "again") return "AGAIN";
            if (name == "smo" || id == "smo") return "SMO";
            if (name == "ceiling" || id == "ceiling") return "CEIL";
            if (name == "detect" || id == "detect") return "DET";  // NUEVO - label para detection slider
            
            return "";
        }
    };
};

//==============================================================================
// CUSTOM SLIDER ATTACHMENT CON UNDO SYSTEM
//==============================================================================

/**
 * Custom attachment usando ParameterAttachment + UndoableParameterHelper
 * Solución probada y testeada en jr-granular
 * APVTS con nullptr = no undo automático, undo manual solo para gestos de usuario
 */
class CustomSliderAttachment : private juce::Slider::Listener
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CustomSliderAttachment(juce::RangedAudioParameter& param,
                          CustomSlider& sliderToUse,
                          juce::UndoManager* um = nullptr)
        : parameter(param),
          slider(sliderToUse),
          undoManager(um),
          attachment(param, 
                    [this](float v) { updateSliderValue(v); },
                    nullptr)  // ParameterAttachment sin UndoManager (solución probada)
    {
        // Establecer rango del slider desde el parámetro
        auto range = param.getNormalisableRange();
        slider.setRange(range.start, range.end, range.interval);
        
        // Escuchar cambios del slider
        slider.addListener(this);
        
        // Establecer valor inicial
        attachment.sendInitialUpdate();
    }
    
    ~CustomSliderAttachment()
    {
        slider.removeListener(this);
    }
    
    //==========================================================================
    // CALLBACK PARA DETECCIÓN DE CAMBIOS
    //==========================================================================
    
    // Callback para notificaciones de cambio de parámetro (para detección de modificación de presets)
    std::function<void()> onParameterChange;
    
private:
    //==========================================================================
    // MÉTODOS DE SLIDER LISTENER
    //==========================================================================
    
    void sliderValueChanged(juce::Slider*) override
    {
        attachment.setValueAsPartOfGesture(static_cast<float>(slider.getValue()));
    }
    
    void sliderDragStarted(juce::Slider*) override
    {
        // Cachear valor al inicio del gesto (patrón helper estático)
        valueAtGestureStart = parameter.getValue();
        attachment.beginGesture();
    }
    
    void sliderDragEnded(juce::Slider*) override
    {
        attachment.endGesture();
        // Registrar undo usando helper estático (una línea - solución probada)
        UndoableParameterHelper::registerChange(undoManager, parameter, valueAtGestureStart);
        
        // Notificar sobre cambio de parámetro para detección de modificación de preset
        if (onParameterChange)
            onParameterChange();
    }
    
    void updateSliderValue(float newValue)
    {
        slider.setValue(newValue, juce::dontSendNotification);
    }
    
    //==========================================================================
    // VARIABLES DE ESTADO
    //==========================================================================
    
    juce::RangedAudioParameter& parameter;
    CustomSlider& slider;
    juce::UndoManager* undoManager;
    juce::ParameterAttachment attachment;
    float valueAtGestureStart{0.0f};  // Cache para el patrón helper estático
};