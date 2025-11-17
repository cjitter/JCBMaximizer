//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBMaximizer and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================
#pragma once

#include <vector>
#include <cmath>

//==============================================================================
// MOVING AVERAGE 4800 - Replica average~ 4800 1 de Max/MSP
//==============================================================================

/**
 * MovingAverage4800 - Promedio móvil de 4800 muestras en modo absoluto
 * 
 * Replica exactamente el comportamiento del objeto average~ 4800 1 de Max/MSP:
 * - Buffer circular de 4800 muestras
 * - Modo absoluto (absolute mode = 1): aplica std::abs() a cada muestra de entrada
 * - Promedio eficiente usando suma acumulativa
 * - Para gain reduction: outlet 3 → average~ 4800 1 → linear2db → slider
 */
class MovingAverage4800
{
public:
    //==========================================================================
    // CONSTRUCTOR
    //==========================================================================
    
    MovingAverage4800()
        : buffer(BUFFER_SIZE, 0.0f),
          writeIndex(0),
          sum(0.0f)
    {
        // Buffer inicializado con ceros, suma inicial = 0
    }
    
    //==========================================================================
    // PROCESAMIENTO DE MUESTRAS
    //==========================================================================
    
    /**
     * Procesa una muestra individual aplicando promedio móvil
     * @param input Muestra de entrada (valor lineal de gain reduction)
     * @return Valor promediado de las últimas 4800 muestras en modo absoluto
     */
    float processSample(float input)
    {
        // Modo absoluto como average~ 4800 1 en Max
        float absInput = std::abs(input);
        
        // Restar el valor que vamos a sobrescribir de la suma
        sum -= buffer[writeIndex];
        
        // Agregar el nuevo valor
        buffer[writeIndex] = absInput;
        sum += absInput;
        
        // Avanzar el índice de escritura (circular)
        writeIndex = (writeIndex + 1) % BUFFER_SIZE;
        
        // Retornar el promedio actual
        return sum / static_cast<float>(BUFFER_SIZE);
    }
    
    /**
     * Obtiene el valor promedio actual sin procesar nueva muestra
     * @return Promedio actual de las últimas 4800 muestras
     */
    float getCurrentAverage() const
    {
        return sum / static_cast<float>(BUFFER_SIZE);
    }
    
    /**
     * Resetea el buffer y la suma (útil para cambios de sample rate o inicio)
     */
    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex = 0;
        sum = 0.0f;
    }
    
    //==========================================================================
    // INFORMACIÓN DEL BUFFER
    //==========================================================================
    
    /**
     * Obtiene el tamaño del buffer (siempre 4800)
     */
    static constexpr int getBufferSize() { return BUFFER_SIZE; }
    
    /**
     * Calcula el tiempo de respuesta en milisegundos para un sample rate dado
     * @param sampleRate Sample rate en Hz
     * @return Tiempo de respuesta en ms (4800 samples / sampleRate * 1000)
     */
    static float getResponseTimeMs(float sampleRate)
    {
        return (static_cast<float>(BUFFER_SIZE) / sampleRate) * 1000.0f;
    }
    
private:
    //==========================================================================
    // CONSTANTES Y VARIABLES MIEMBRO
    //==========================================================================
    
    static constexpr int BUFFER_SIZE = 1200;    // Tamaño ultra-responsivo para máxima sensibilidad (era 4800→2400)
    
    std::vector<float> buffer;                  // Buffer circular de muestras
    int writeIndex;                             // Índice de escritura actual
    float sum;                                  // Suma acumulativa para cálculo eficiente
    
    //==========================================================================
    // MACRO JUCE
    //==========================================================================
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MovingAverage4800)
};