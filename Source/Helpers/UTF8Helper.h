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

#include <juce_core/juce_core.h>

//==============================================================================
// HELPER PARA UTF-8 STRINGS SEGUROS
//==============================================================================

/**
 * UTF8Helper - Macro para manejo seguro de strings UTF-8 en JUCE
 * 
 * Helper minimal que define la macro JUCE_UTF8() para crear strings UTF-8
 * de forma segura, evitando assertions de JUCE con caracteres especiales
 * como tildes, ñ y otros caracteres con acentos.
 * 
 * USO EXTENSIVO EN:
 * - Tooltips en español (áéíóúñ)
 * - Credits window (nombres con acentos)
 * - Cualquier string que contenga caracteres no-ASCII
 * 
 * EJEMPLO DE USO:
 * juce::String texto = JUCE_UTF8("Configuración con acentos ñáéíóú");
 */

//==============================================================================
// MACRO DEFINITIONS - Macro para crear strings UTF-8 seguros
//==============================================================================

/**
 * Macro para crear strings UTF-8 de forma segura
 * Evita las assertions de JUCE con caracteres especiales
 */
#define JUCE_UTF8(text) juce::String(juce::CharPointer_UTF8(text))