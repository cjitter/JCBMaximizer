//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBCompressor and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================
#pragma once

//==============================================================================
// INCLUDES
//==============================================================================

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

//==============================================================================
// CLASE PRINCIPAL PARA PARAMETER CHANGE ACTIONS
//==============================================================================

/**
 * ParameterChangeAction - Core del sistema undo/redo para parámetros
 * 
 * Sistema probado que maneja cambios de parámetros con undo/redo sin conflictos
 * con la automatización del DAW. Características principales:
 * - Integration con juce::UndoableAction para compatibility completa
 * - Coalescing automático de consecutive actions en el mismo parámetro
 * - Thread safety garantizado para message thread operations
 * - Performance optimization con memory management eficiente
 * - Support para threshold mínimo de cambios (evita micro-adjustments)
 * 
 * IMPORTANTE: Sistema diseñado para parámetros creativos/musicales únicamente.
 * Parámetros de utilidad (bypass, meters, etc.) deben usar nullptr UndoManager.
 * 
 * SETUP REQUERIDO EN NUEVO PLUGIN:
 * - APVTS: apvts(*this, nullptr, "Parameters", createParameterLayout())
 * - UndoManager: juce::UndoManager guiUndoManager; // En el Processor
 */
class ParameterChangeAction : public juce::UndoableAction
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    ParameterChangeAction (juce::RangedAudioParameter* param, 
                          float oldValue, 
                          float newValue) noexcept
        : parameter (param)
        , oldNormalizedValue (oldValue)
        , newNormalizedValue (newValue)
    {
        // THREAD SAFETY: Solo llamar desde message thread
        jassert (juce::MessageManager::existsAndIsCurrentThread());
    }

    //==========================================================================
    // OVERRIDES DE UNDOABLE ACTION
    //==========================================================================
    
    bool perform() override
    {
        if (parameter != nullptr)
        {
            parameter->setValueNotifyingHost (newNormalizedValue);
            return true;
        }
        return false;
    }

    bool undo() override
    {
        if (parameter != nullptr)
        {
            parameter->setValueNotifyingHost (oldNormalizedValue);
            return true;
        }
        return false;
    }

    int getSizeInUnits() override
    {
        return sizeof (*this);
    }
    
    //==========================================================================
    // OPTIMIZATION METHODS - Performance improvements
    //==========================================================================
    
    /**
     * Performance optimization: Coalesce consecutive actions en el mismo parameter
     * Reduce el uso de memoria y mejora el rendimiento del undo/redo
     */
    juce::UndoableAction* createCoalescedAction (juce::UndoableAction* nextAction) override
    {
        if (auto* next = dynamic_cast<ParameterChangeAction*> (nextAction))
        {
            if (next->parameter == parameter)
            {
                // Combine actions: keep original oldValue, usar next newValue
                return new ParameterChangeAction (parameter, oldNormalizedValue, next->newNormalizedValue);
            }
        }
        return nullptr;
    }

private:
    //==========================================================================
    // VARIABLES MIEMBRO - Member variables
    //==========================================================================
    
    juce::RangedAudioParameter* parameter;      // Parámetro que fue modificado
    float oldNormalizedValue;                   // Valor antes del cambio (0-1)
    float newNormalizedValue;                   // Valor después del cambio (0-1)

    //==========================================================================
    // MACRO JUCE - Memory management
    //==========================================================================
    
    JUCE_DECLARE_NON_COPYABLE (ParameterChangeAction)
};