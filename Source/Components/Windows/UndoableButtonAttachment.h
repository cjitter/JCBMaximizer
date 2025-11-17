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
#include "../../Helpers/UndoableParameterHelper.h"

//==============================================================================
// CLASE PRINCIPAL PARA BUTTON ATTACHMENTS CON UNDO/REDO
//==============================================================================

/**
 * UndoableButtonAttachment - Sistema de attachment para botones con undo/redo
 * 
 * Attachment especializado que provee funcionalidad de undo/redo para controles
 * de botón conectados a parámetros del plugin. Características principales:
 * - Integration con ParameterAttachment de JUCE para automation sync
 * - Manual undo/redo registration solo para user interactions
 * - Thread safety garantizado para message thread operations
 * - Support específico para filter order parameters (12/24 dB)
 * - Callback system para notificación de cambios de parámetros
 *
 * IMPORTANTE: Solo usar desde message thread (mouse/key callbacks)
 * APVTS con nullptr = no automatic undo, manual undo solo para clicks de usuario
 */
class UndoableButtonAttachment : private juce::Button::Listener
{
public:
    //==========================================================================
    // CALLBACK SYSTEM - Sistema de callbacks
    //==========================================================================
    
    std::function<void()> onParameterChange;  // Callback para cambios de parámetro (preset modification indicator)
    
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    UndoableButtonAttachment(juce::RangedAudioParameter& param,
                           juce::Button& buttonToUse,
                           juce::UndoManager* um = nullptr)
        : parameter(param),
          button(buttonToUse),
          undoManager(um),
          attachment(param, 
                    [this](float v) { updateButtonState(v); },
                    nullptr)  // ParameterAttachment sin UndoManager (solución probada)
    {
        // THREAD SAFETY: Solo llamar desde message thread
        jassert (juce::MessageManager::existsAndIsCurrentThread());
        
        // Configurar listener para cambios de botón
        button.addListener(this);
        
        // Establecer estado inicial del botón
        attachment.sendInitialUpdate();
    }
    
    ~UndoableButtonAttachment()
    {
        button.removeListener(this);
    }
    
private:
    //==========================================================================
    // OVERRIDES DE BUTTON LISTENER
    //==========================================================================
    
    void buttonClicked(juce::Button*) override
    {
        // THREAD SAFETY: Button clicks siempre en message thread
        jassert (juce::MessageManager::existsAndIsCurrentThread());
        
        // Cachear valor anterior antes del cambio (patrón helper estático)
        float oldValue = parameter.getValue();
        
        // Obtener estado actual del botón (ya toggled por setClickingTogglesState)
        bool newState = button.getToggleState();
        float newValue = newState ? 1.0f : 0.0f;
        
        // Actualizar parámetro para coincidir con estado del botón
        attachment.setValueAsCompleteGesture(newValue);
        
        // Registrar undo usando helper estático (solución probada)
        // Usar threshold más alto para botones (valores discretos)
        UndoableParameterHelper::registerChange(undoManager, parameter, oldValue, newValue, 0.5f);
        
        // Llamar callback de cambio de parámetro (preset modification indicator)
        if (onParameterChange)
            onParameterChange();
    }
    
    //==========================================================================
    // MÉTODOS INTERNOS - Parameter update methods
    //==========================================================================
    
    void updateButtonState(float newValue)
    {
        // Actualizar estado del botón desde valor del parámetro (automation, preset loading, etc.)
        bool shouldBeToggled = newValue >= 0.5f;
        button.setToggleState(shouldBeToggled, juce::dontSendNotification);
        
        // juce::String paramId = parameter.getParameterID();
        // if (paramId == "j_HPFORDER" || paramId == "k_LPFORDER")
        // {
        //     button.setButtonText(shouldBeToggled ? "24" : "12");
        // }
    }
    
    //==========================================================================
    // VARIABLES MIEMBRO - Member variables
    //==========================================================================
    
    juce::RangedAudioParameter& parameter;  // Referencia al parámetro del plugin
    juce::Button& button;                   // Referencia al botón UI
    juce::UndoManager* undoManager;         // UndoManager para undo/redo (puede ser nullptr)
    juce::ParameterAttachment attachment;   // JUCE ParameterAttachment para automation sync
    
    //==========================================================================
    // MACRO JUCE - Memory management
    //==========================================================================
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UndoableButtonAttachment)
};