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
// CLASE PRINCIPAL PARA MULTI-STATE BUTTON ATTACHMENTS CON UNDO/REDO
//==============================================================================

/**
 * UndoableMultiStateButtonAttachment - Sistema de attachment para botones de múltiples estados con undo/redo
 * 
 * Attachment especializado que provee funcionalidad de undo/redo para controles
 * de botón con múltiples estados conectados a parámetros del plugin. Características principales:
 * - Integration con ParameterAttachment de JUCE para automation sync
 * - Manual undo/redo registration solo para user interactions
 * - Thread safety garantizado para message thread operations
 * - Support específico para filter order parameters (12/24/48 dB correspondientes a 0/1/2)
 * - Callback system para notificación de cambios de parámetros
 * - Cicla entre estados al hacer click
 *
 * IMPORTANTE: Solo usar desde message thread (mouse/key callbacks)
 * APVTS con nullptr = no automatic undo, manual undo solo para clicks de usuario
 */
class UndoableMultiStateButtonAttachment : private juce::Button::Listener
{
public:
    //==========================================================================
    // CALLBACK SYSTEM - Sistema de callbacks
    //==========================================================================
    
    std::function<void()> onParameterChange;  // Callback para cambios de parámetro (preset modification indicator)
    
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    UndoableMultiStateButtonAttachment(juce::RangedAudioParameter& param,
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
        
        // Desactivar toggle automático - lo manejamos manualmente
        button.setClickingTogglesState(false);
        
        // Establecer estado inicial del botón
        attachment.sendInitialUpdate();
    }
    
    ~UndoableMultiStateButtonAttachment()
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
        
        // Obtener valor actual del parámetro
        float oldValue = parameter.getValue();
        
        // Obtener valor actual del parámetro y ciclar al siguiente estado
        int currentIntValue = static_cast<int>(oldValue + 0.5f); // Redondear para evitar errores de punto flotante
        int nextIntValue = (currentIntValue + 1) % 3; // Ciclar entre 0, 1, 2
        float newValue = static_cast<float>(nextIntValue);
        
        // Actualizar parámetro
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
        // Actualizar texto del botón desde valor del parámetro (automation, preset loading, etc.)
        int intValue = static_cast<int>(newValue + 0.5f); // Redondear para evitar errores de punto flotante
        
        // MAXIMIZER: No filter order parameters - comentado según CONTEXTO.txt
        // juce::String paramId = parameter.getParameterID();
        // if (paramId == "j_HPFORDER" || paramId == "k_LPFORDER")
        // {
        //     switch (intValue)
        //     {
        //         case 0:
        //             button.setButtonText("12");
        //             break;
        //         case 1:
        //             button.setButtonText("24");
        //             break;
        //         case 2:
        //             button.setButtonText("48");
        //             break;
        //         default:
        //             button.setButtonText("12"); // Fallback
        //             break;
        //     }
        // }
        else
        {
            // Para otros parámetros, mostrar el número directamente
            button.setButtonText(juce::String(intValue));
        }
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UndoableMultiStateButtonAttachment)
};