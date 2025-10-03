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
#include <cmath>
#include "ParameterChangeAction.h"

//==============================================================================
// CLASE HELPER ESTÁTICA PARA PARAMETER CHANGES
//==============================================================================

/**
 * UndoableParameterHelper - Helper estático para registro de parameter changes
 * 
 * Clase utility que simplifica enormemente el registro de cambios de parámetros
 * para undo/redo. Características principales:
 * - Static methods para uso desde cualquier control
 * - Threshold automático para evitar micro-adjustments insignificantes
 * - Integration perfecta con ParameterChangeAction
 * - Thread safety garantizado para message thread operations
 * - Pattern de uso minimal y reusable
 * 
 * PATRÓN DE USO ESTÁNDAR:
 * 1. Cachear parameter value en gesture start (mouseDown, etc.)
 * 2. Llamar UndoableParameterHelper::registerChange() en gesture end (mouseUp, etc.)
 * 
 * EJEMPLO DE IMPLEMENTACIÓN:
 * // En mouseDown: float oldValue = param.getValue();
 * // En mouseUp: UndoableParameterHelper::registerChange(undoManager, param, oldValue);
 */
class UndoableParameterHelper
{
public:
    //==============================================================================
    // MÉTODOS DE REGISTRO - Register parameter changes para undo/redo
    //==============================================================================
    
    /**
     * Registra un parameter change para undo/redo.
     * Solo crea undo transaction si los values son diferentes y undoManager es válido.
     * 
     * @param um UndoManager para registrar (puede ser nullptr para disable undo)
     * @param param El parámetro que fue modificado
     * @param oldValue El value antes del cambio (normalized 0-1)
     * @param newValue El value después del cambio (normalized 0-1)
     * @param threshold Minimum change threshold para registrar (default: 0.0001f)
     */
    static void registerChange(juce::UndoManager* um,
                             juce::RangedAudioParameter& param,
                             float oldValue,
                             float newValue,
                             float threshold = 0.0001f)
    {
        // THREAD SAFETY: Solo llamar desde message thread
        jassert (juce::MessageManager::existsAndIsCurrentThread());
        
        if (um == nullptr || std::abs(oldValue - newValue) < threshold)
            return;  // No registrar cambios insignificantes
            
        um->beginNewTransaction();
        um->perform(new ParameterChangeAction(&param, oldValue, newValue));
    }
    
    /**
     * Overload conveniente que usa el valor del parámetro actual como nuevo valor.
     * 
     * @param um UndoManager para registrar (puede ser nullptr para disable undo)
     * @param param El parámetro que fue modificado
     * @param oldValue El value antes del cambio (normalized 0-1)
     * @param threshold Minimum change threshold para registrar (default: 0.0001f)
     */
    static void registerChange(juce::UndoManager* um,
                             juce::RangedAudioParameter& param,
                             float oldValue,
                             float threshold = 0.0001f)
    {
        registerChange(um, param, oldValue, param.getValue(), threshold);
    }

private:
    //==============================================================================
    // CONSTRUCTORES PRIVADOS - Helper class estática, no instances permitidas
    //==============================================================================
    
    UndoableParameterHelper() = delete;
    ~UndoableParameterHelper() = delete;
    UndoableParameterHelper(const UndoableParameterHelper&) = delete;
    UndoableParameterHelper& operator=(const UndoableParameterHelper&) = delete;
};