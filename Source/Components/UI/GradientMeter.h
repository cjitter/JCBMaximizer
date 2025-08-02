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
#include <juce_core/juce_core.h>
#include <atomic>
#include "../Windows/DarkThemeColors.h"

//==============================================================================
// CLASE PRINCIPAL DEL GRADIENT METER
//==============================================================================

/**
 * GradientMeter para medición de niveles de audio con gradientes visuales
 * 
 * Diseñado para mostrar niveles RMS con indicadores de peak y detección de clip.
 * Características principales:
 * - Actualización controlada desde PluginEditor a 60Hz constante
 * - Ballística RMS con attack rápido y release lento
 * - Indicador de peak con hold temporal
 * - Detección de clipping con hold visual
 * - Soporte para modos DELTA y SOLO SC con gradientes específicos
 * - Thread-safe con std::atomic para variables de nivel
 */
class GradientMeter : public juce::Component
{
public:
    //==========================================================================
    // CONSTRUCTOR Y CONFIGURACIÓN
    //==========================================================================
    
    GradientMeter(std::function<float()>&& valueFunction) 
        : valueSupplier(std::move(valueFunction)),
          clipHoldTimer(0),
          isClipActive(false),
          smoothedLevel{-80.0f},
          peakLevel{-80.0f},
          peakHoldTimer(0),
          isDeltaMode(false),
          isSoloScMode(false)
    {
        // Sin Timer inheritance - control centralizado desde PluginEditor
    }
    
    //==========================================================================
    // MÉTODOS DE CONTROL DE ESTADO
    //==========================================================================
    
    void setClipDetected(bool detected) noexcept
    {
        if (detected && !isClipActive) {
            isClipActive = true;
            clipHoldTimer = 120; // 2 segundos a 60Hz = 120 frames
        }
    }
    
    void setDeltaMode(bool delta) noexcept
    {
        if (isDeltaMode != delta) {
            isDeltaMode = delta;
            resized(); // Actualizar gradiente
            repaint();
        }
    }

    void setSoloScMode(bool soloSc) noexcept
    {
        if (isSoloScMode != soloSc) {
            isSoloScMode = soloSc;
            resized(); // Actualizar gradiente
            repaint();
        }
    }
    
    // CRASH FIX: Permitir configuración diferida de valueSupplier
    void setValueFunction(std::function<float()>&& newValueFunction) noexcept
    {
        valueSupplier = std::move(newValueFunction);
    }
    
    //==========================================================================
    // MÉTODO DE ACTUALIZACIÓN (LLAMADO DESDE PLUGINEDITOR A 60HZ)
    //==========================================================================
    
    /**
     * Actualiza el nivel del medidor
     * Llamado desde PluginEditor::timerCallback() a 60Hz constante
     * Reemplaza el antiguo sistema de Timer inheritance
     */
    void updateLevel()
    {
        // Obtener nivel actual y aplicar suavizado
        const float currentLevel = valueSupplier();
        
        // Cargar nivel suavizado actual
        float currentSmoothedLevel = smoothedLevel.load(std::memory_order_relaxed);
        
        // Ballística RMS: attack rápido, release lento
        if (currentLevel > currentSmoothedLevel) {
            // Attack - respuesta más rápida cuando el nivel aumenta
            currentSmoothedLevel += (currentLevel - currentSmoothedLevel) * 0.7f;
        } else {
            // Release - decay más lento cuando el nivel disminuye
            currentSmoothedLevel += (currentLevel - currentSmoothedLevel) * 0.1f;
        }
        
        // Almacenar nivel suavizado actualizado
        smoothedLevel.store(currentSmoothedLevel, std::memory_order_relaxed);
        
        // Manejo de nivel peak - attack instantáneo, hold, luego decay lento
        float currentPeakLevel = peakLevel.load(std::memory_order_relaxed);
        
        if (currentLevel > currentPeakLevel) {
            peakLevel.store(currentLevel, std::memory_order_relaxed);
            peakHoldTimer = 30; // Hold por 0.5 segundos a 60Hz
        } else if (peakHoldTimer > 0) {
            peakHoldTimer--;
        } else {
            // Decay del nivel peak lentamente después del tiempo de hold
            currentPeakLevel += (currentLevel - currentPeakLevel) * 0.02f;
            peakLevel.store(currentPeakLevel, std::memory_order_relaxed);
        }
        
        // Detección de clipping
        if (currentLevel >= -0.01f) { // Detectar clipping a -0.01 dB
            setClipDetected(true);
        }
        
        // Manejar countdown del clip hold
        if (isClipActive && clipHoldTimer > 0) {
            clipHoldTimer--;
            if (clipHoldTimer <= 0) {
                isClipActive = false;
            }
        }
        
        repaint();
    }
    
    //==========================================================================
    // OVERRIDE DE RENDERING
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.5f);
        
        // Fondo oscuro con sombra interior sutil
        g.setColour(DarkTheme::meterBackground.withAlpha(0.8f));
        g.fillRoundedRectangle(bounds, 2.0f);
        
        // Efecto de sombra interior
        g.setColour(DarkTheme::backgroundDark.withAlpha(0.4f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 1.5f, 1.0f);
        
        // Solo mostrar el gradiente si hay señal por encima del umbral (-95 dB)
        const float currentSmoothedLevel = smoothedLevel.load(std::memory_order_relaxed);
        if (currentSmoothedLevel > -95.0f) {
            // Relleno del medidor RMS con gradiente del tema oscuro
            g.setGradientFill(gradient);
            const auto scaledY = juce::jmap(currentSmoothedLevel, -100.f, 0.f, 0.f, static_cast<float>(getHeight() - 3));
            auto fillBounds = bounds.reduced(1.0f);
            g.fillRoundedRectangle(fillBounds.removeFromBottom(scaledY), 1.5f);
        }
        
        // Indicador de peak - línea delgada en la posición del peak
        const float currentPeakLevel = peakLevel.load(std::memory_order_relaxed);
        if (currentPeakLevel > -95.0f) {
            const auto peakY = getHeight() - 3 - juce::jmap(currentPeakLevel, -100.f, 0.f, 0.f, static_cast<float>(getHeight() - 3));
            g.setColour(DarkTheme::accent.brighter(0.3f).withAlpha(0.8f));
            g.fillRect(bounds.getX() + 1, peakY - 1, bounds.getWidth() - 2, 2.0f);
        }
        
        // Indicador de clip - línea roja en la parte superior si hay clip activo
        if (isClipActive) {
            g.setColour(DarkTheme::meterClipIndicator);
            auto clipBounds = bounds.removeFromTop(3.0f);
            g.fillRoundedRectangle(clipBounds, 1.0f);
        }
        
        // Borde exterior destacado
        g.setColour(DarkTheme::border.withAlpha(0.4f));
        g.drawRoundedRectangle(bounds, 2.0f, 0.5f);
    }
    
    void resized() override
    {
        const auto bounds = getLocalBounds().toFloat();
        if (isSoloScMode)
            gradient = DarkTheme::createSoloScMeterGradient(bounds);
        else if (isDeltaMode)
            gradient = DarkTheme::createDeltaMeterGradient(bounds);
        else
            gradient = DarkTheme::createMeterGradient(bounds);
    }
        
private:
    //==========================================================================
    // VARIABLES DE ESTADO
    //==========================================================================
    
    std::function<float()> valueSupplier;
    juce::ColourGradient gradient{};
    
    // Variables de detección de clip
    int clipHoldTimer;
    bool isClipActive;
    std::atomic<float> smoothedLevel;
    std::atomic<float> peakLevel;
    int peakHoldTimer;
    bool isDeltaMode;
    bool isSoloScMode;
};

//==============================================================================
// SIDECHAIN METER ESPECIALIZADO
//==============================================================================

/**
 * Medidor especializado para display de sidechain - más sutil e integrado
 * Misma funcionalidad que GradientMeter pero con transparencias y gradientes específicos
 */
class SidechainMeter : public juce::Component
{
public:
    //==========================================================================
    // CONSTRUCTOR Y CONFIGURACIÓN
    //==========================================================================
    
    SidechainMeter(std::function<float()>&& valueFunction) 
        : valueSupplier(std::move(valueFunction)),
          clipHoldTimer(0),
          isClipActive(false),
          smoothedLevel{-80.0f},
          peakLevel{-80.0f},
          peakHoldTimer(0),
          isSoloScMode(false),
          isDeltaMode(false)
    {
        // Control centralizado desde PluginEditor a 60Hz
    }
    
    //==========================================================================
    // MÉTODOS DE CONTROL DE ESTADO
    //==========================================================================
    
    void setClipDetected(bool detected) noexcept
    {
        if (detected && !isClipActive) {
            isClipActive = true;
            clipHoldTimer = 120; // 2 segundos a 60Hz = 120 frames
        }
    }
    
    void setSoloScMode(bool soloSc) noexcept
    {
        if (isSoloScMode != soloSc) {
            isSoloScMode = soloSc;
            resized(); // Actualizar gradiente
            repaint();
        }
    }
    
    void setDeltaMode(bool delta) noexcept
    {
        if (isDeltaMode != delta) {
            isDeltaMode = delta;
            resized(); // Actualizar gradiente
            repaint();
        }
    }
    
    //==========================================================================
    // MÉTODO DE ACTUALIZACIÓN (LLAMADO DESDE PLUGINEDITOR A 60HZ)
    //==========================================================================
    
    void updateLevel()
    {
        // Obtener nivel actual y aplicar suavizado
        const float currentLevel = valueSupplier();
        
        // Cargar nivel suavizado actual
        float currentSmoothedLevel = smoothedLevel.load(std::memory_order_relaxed);
        
        // Ballística RMS: attack rápido, release lento (igual que medidores principales)
        if (currentLevel > currentSmoothedLevel) {
            // Attack - respuesta más rápida cuando el nivel aumenta
            currentSmoothedLevel += (currentLevel - currentSmoothedLevel) * 0.7f;
        } else {
            // Release - decay más lento cuando el nivel disminuye
            currentSmoothedLevel += (currentLevel - currentSmoothedLevel) * 0.1f;
        }
        
        // Almacenar nivel suavizado actualizado
        smoothedLevel.store(currentSmoothedLevel, std::memory_order_relaxed);
        
        // Manejo de nivel peak - attack instantáneo, hold, luego decay lento
        float currentPeakLevel = peakLevel.load(std::memory_order_relaxed);
        
        if (currentLevel > currentPeakLevel) {
            peakLevel.store(currentLevel, std::memory_order_relaxed);
            peakHoldTimer = 30; // Hold por 0.5 segundos a 60Hz
        } else if (peakHoldTimer > 0) {
            peakHoldTimer--;
        } else {
            // Decay del nivel peak lentamente después del tiempo de hold
            currentPeakLevel += (currentLevel - currentPeakLevel) * 0.02f;
            peakLevel.store(currentPeakLevel, std::memory_order_relaxed);
        }
        
        // Detección de clipping
        if (currentLevel >= -0.01f) { // Detectar clipping a -0.01 dB
            setClipDetected(true);
        }
        
        // Manejar countdown del clip hold
        if (isClipActive && clipHoldTimer > 0) {
            clipHoldTimer--;
            if (clipHoldTimer <= 0) {
                isClipActive = false;
            }
        }
        
        repaint();
    }
    
    //==========================================================================
    // OVERRIDE DE RENDERING
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);
        
        // Fondo oscuro muy sutil con alta transparencia
        g.setColour(DarkTheme::meterBackground.withAlpha(0.3f));
        g.fillRoundedRectangle(bounds, 2.0f);
        
        // Solo mostrar nivel si está por encima del umbral (-95 dB como los principales)
        const float currentSmoothedLevel = smoothedLevel.load(std::memory_order_relaxed);
        if (currentSmoothedLevel > -95.0f) {
            // Usar gradiente de medidor de entrada con transparencia
            g.setGradientFill(gradient);
            const auto scaledY = juce::jmap(currentSmoothedLevel, -100.f, 0.f, 0.f, static_cast<float>(getHeight() - 2));
            auto fillBounds = bounds.reduced(1.0f);
            
            // Aplicar transparencia adicional al relleno
            g.setOpacity(0.7f);
            g.fillRoundedRectangle(fillBounds.removeFromBottom(scaledY), 1.5f);
            g.setOpacity(1.0f);
        }
        
        // Indicador de peak - línea delgada en la posición del peak (más sutil)
        const float currentPeakLevel = peakLevel.load(std::memory_order_relaxed);
        if (currentPeakLevel > -95.0f) {
            const auto peakY = getHeight() - 2 - juce::jmap(currentPeakLevel, -100.f, 0.f, 0.f, static_cast<float>(getHeight() - 2));
            g.setColour(DarkTheme::meterClip.withAlpha(0.6f));  // Usar el color azul del tope del gradiente
            g.fillRect(bounds.getX() + 1, peakY - 0.5f, bounds.getWidth() - 2, 1.0f);
        }
        
        // Indicador de clip - línea roja sutil en la parte superior si hay clip activo
        if (isClipActive) {
            g.setColour(DarkTheme::meterClipIndicator.withAlpha(0.8f));
            auto clipBounds = bounds.removeFromTop(2.0f);
            g.fillRoundedRectangle(clipBounds, 1.0f);
        }
        
        // Borde muy sutil
        g.setColour(DarkTheme::border.withAlpha(0.2f));
        g.drawRoundedRectangle(bounds, 2.0f, 0.5f);
    }
    
    void resized() override
    {
        const auto bounds = getLocalBounds().toFloat();
        // Usar gradiente apropiado basado en el modo
        if (isSoloScMode)
            gradient = DarkTheme::createSoloScMeterGradient(bounds);
        else if (isDeltaMode)
            gradient = DarkTheme::createDeltaMeterGradient(bounds);
        else
            gradient = DarkTheme::createSidechainMeterGradient(bounds);
    }
    
private:
    //==========================================================================
    // VARIABLES DE ESTADO
    //==========================================================================
    
    std::function<float()> valueSupplier;
    juce::ColourGradient gradient{};
    
    // Variables de detección de clip
    int clipHoldTimer;
    bool isClipActive;
    std::atomic<float> smoothedLevel;
    std::atomic<float> peakLevel;
    int peakHoldTimer;
    bool isSoloScMode;
    bool isDeltaMode;
};

//==============================================================================
// GRADIENT METER INVERTIDO (GAIN REDUCTION)
//==============================================================================

/**
 * Medidor de reducción de ganancia con comportamiento invertido
 * Diseñado específicamente para mostrar gain reduction del compresor
 */
class GradientMeterInv : public juce::Component
{
public:
    //==========================================================================
    // CONSTRUCTOR Y CONFIGURACIÓN
    //==========================================================================
    
    GradientMeterInv(std::function<float()>&& valueFunction) 
        : valueSupplier(std::move(valueFunction)),
          smoothedValue{0.0f},
          targetValue{0.0f},
          isZoomed(false),
          peakValue{0.0f},
          isDeltaMode(false)
    {
        // Control centralizado desde PluginEditor a 60Hz
    }
    
    //==========================================================================
    // MÉTODOS DE CONTROL DE ESTADO
    //==========================================================================
    
    // Método para sincronizar con el zoom del histograma
    void setZoomLevel(bool zoomed) noexcept
    {
        if (isZoomed != zoomed) {
            isZoomed = zoomed;
            repaint();
        }
    }
    
    // Método para cambiar a modo DELTA
    void setDeltaMode(bool delta) noexcept
    {
        if (isDeltaMode != delta) {
            isDeltaMode = delta;
            resized(); // Actualizar gradiente
            repaint();
        }
    }
    
    // CRASH FIX: Permitir configuración diferida de valueSupplier
    void setValueFunction(std::function<float()>&& newValueFunction) noexcept
    {
        valueSupplier = std::move(newValueFunction);
    }
    
    //==========================================================================
    // MÉTODO DE ACTUALIZACIÓN (LLAMADO DESDE PLUGINEDITOR A 60HZ)
    //==========================================================================
    
    void updateLevel()
    {
        // REVERTIDO: Obtener el valor actual sin suavizado - comportamiento original
        const float currentValue = valueSupplier();
        targetValue.store(currentValue, std::memory_order_relaxed);
        smoothedValue.store(currentValue, std::memory_order_relaxed);  // Valor directo sin procesar
        
        // Peak sin hold - sigue el valor instantáneamente
        peakValue.store(currentValue, std::memory_order_relaxed);
        
        // Reseteo cuando no hay reducción significativa
        // 0 dB = sin reducción, valores negativos = reducción activa  
        if (std::abs(currentValue) < 0.05f) {
            // Asegurar que está completamente en reposo (sin barra)
            smoothedValue.store(0.0f, std::memory_order_relaxed);
            peakValue.store(0.0f, std::memory_order_relaxed);
        }
        
        repaint();
    }
    
    //==========================================================================
    // OVERRIDE DE RENDERING
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.5f);
        
        // Fondo oscuro del medidor
        g.setColour(DarkTheme::meterBackground.withAlpha(0.8f));
        g.fillRoundedRectangle(bounds, 2.0f);
        
        // NUEVO: Fondo con gradiente azul (como base del slider)
        auto blueBgGradient = DarkTheme::createBlueGainReductionGradient(bounds);
        g.setGradientFill(blueBgGradient);
        g.fillRoundedRectangle(bounds.reduced(1.0f), 1.5f);
        
        // Medidor de reducción de ganancia - FINAL: barra blanca muestra reducción
        // 0 dB = sin reducción/reposo = sin barra (oscuro)  
        // -100 dB = máxima reducción = barra llena (gradiente visible)
        const float currentSmoothedValue = smoothedValue.load(std::memory_order_relaxed);
        
        // Mapear valores dB del procesador (0 a -100 dB) a altura de barra  
        // currentSmoothedValue ahora viene en dB desde el MovingAverage4800
        float barHeight;
        
        // Rango de trabajo ultra-sensible para máxima visibilidad de reducciones pequeñas
        const float minReduction = 0.0f;    // Sin reducción (sin barra)
        const float maxReduction = isZoomed ? -20.0f : -40.0f;  // Zoom x2: -20dB, Normal: -40dB (ultra-sensible)
        
        // Mapeo logarítmico para realzar reducciones pequeñas: valores más negativos = más barra blanca
        float fillRatio = juce::jmap(currentSmoothedValue, minReduction, maxReduction, 0.0f, 1.0f);
        fillRatio = juce::jlimit(0.0f, 1.0f, fillRatio);
        
        // Aplicar mapeo logarítmico para amplificar visualmente las reducciones pequeñas
        fillRatio = std::pow(fillRatio, 0.6f);  // Comprime menos las reducciones pequeñas (más visibles)
        
        // Calcular altura de barra (0 dB = sin barra, -40/-20 dB = barra llena según zoom)
        barHeight = (getHeight() - 3) * fillRatio;
        
        // Dibujar el gradiente desde arriba con la altura calculada - BARRA OPTIMIZADA
        if (barHeight > 1.0f) {
            g.setGradientFill(gradient);
            
            // Crear barra ultra-visible (75% del ancho original) centrada
            auto fillBounds = bounds.reduced(1.0f);
            float barWidth = fillBounds.getWidth() * 0.75f;  // Aumentado de 40% → 60% → 75%
            float xOffset = (fillBounds.getWidth() - barWidth) * 0.5f;
            auto narrowFillBounds = fillBounds.withTrimmedLeft(xOffset).withTrimmedRight(xOffset);
            
            auto barRect = narrowFillBounds.removeFromTop(barHeight);
            g.fillRoundedRectangle(barRect, 1.5f);
        }
        
        
        // Borde exterior
        g.setColour(DarkTheme::border.withAlpha(0.4f));
        g.drawRoundedRectangle(bounds, 2.0f, 0.5f);
    }
    
    void resized() override
    {
        const auto bounds = getLocalBounds().toFloat();
        if (isDeltaMode)
            gradient = DarkTheme::createDeltaGainReductionGradient(bounds);
        else
            gradient = DarkTheme::createGainReductionGradient(bounds);
    }
    
private:
    //==========================================================================
    // VARIABLES DE ESTADO
    //==========================================================================
    
    std::function<float()> valueSupplier;
    juce::ColourGradient gradient{};
    std::atomic<float> smoothedValue;
    std::atomic<float> targetValue;
    bool isZoomed;
    std::atomic<float> peakValue;
    bool isDeltaMode;
};

//==============================================================================
// GRADIENT METER OUTPUT
//==============================================================================

/**
 * Medidor especializado para salida con gradientes invertidos
 */
class GradientMeterOutput : public juce::Component
{
public:
    //==========================================================================
    // CONSTRUCTOR Y CONFIGURACIÓN
    //==========================================================================
    
    GradientMeterOutput(std::function<float()>&& valueFunction) 
        : valueSupplier(std::move(valueFunction)),
          clipHoldTimer(0),
          isClipActive(false),
          smoothedLevel{-80.0f},
          peakLevel{-80.0f},
          peakHoldTimer(0),
          isDeltaMode(false),
          isSoloScMode(false),
          isBypassMode(false)
    {
        // Control centralizado desde PluginEditor
    }
    
    //==========================================================================
    // MÉTODOS DE CONTROL DE ESTADO
    //==========================================================================
    
    void setClipDetected(bool detected) noexcept
    {
        if (detected && !isClipActive) {
            isClipActive = true;
            clipHoldTimer = 120; // 2 segundos a 60Hz = 120 frames
        }
    }
    
    void setDeltaMode(bool delta) noexcept
    {
        if (isDeltaMode != delta) {
            isDeltaMode = delta;
            resized(); // Actualizar gradiente
            repaint();
        }
    }
    
    void setSoloScMode(bool soloSc) noexcept
    {
        if (isSoloScMode != soloSc) {
            isSoloScMode = soloSc;
            resized(); // Actualizar gradiente
            repaint();
        }
    }
    
    void setBypassMode(bool bypass) noexcept
    {
        if (isBypassMode != bypass) {
            isBypassMode = bypass;
            resized(); // Actualizar gradiente
            repaint();
        }
    }
    
    // CRASH FIX: Permitir configuración diferida de valueSupplier
    void setValueFunction(std::function<float()>&& newValueFunction) noexcept
    {
        valueSupplier = std::move(newValueFunction);
    }
    
    //==========================================================================
    // MÉTODO DE ACTUALIZACIÓN (LLAMADO DESDE PLUGINEDITOR A 60HZ)
    //==========================================================================
    
    void updateLevel()
    {
        // Obtener nivel actual y aplicar suavizado
        const float currentLevel = valueSupplier();
        
        // Cargar nivel suavizado actual
        float currentSmoothedLevel = smoothedLevel.load(std::memory_order_relaxed);
        
        // Ballística RMS: attack rápido, release lento
        if (currentLevel > currentSmoothedLevel) {
            // Attack - respuesta más rápida cuando el nivel aumenta
            currentSmoothedLevel += (currentLevel - currentSmoothedLevel) * 0.7f;
        } else {
            // Release - decay más lento cuando el nivel disminuye
            currentSmoothedLevel += (currentLevel - currentSmoothedLevel) * 0.1f;
        }
        
        // Almacenar nivel suavizado actualizado
        smoothedLevel.store(currentSmoothedLevel, std::memory_order_relaxed);
        
        // Manejo de nivel peak - attack instantáneo, hold, luego decay lento
        float currentPeakLevel = peakLevel.load(std::memory_order_relaxed);
        
        if (currentLevel > currentPeakLevel) {
            peakLevel.store(currentLevel, std::memory_order_relaxed);
            peakHoldTimer = 30; // Hold por 0.5 segundos a 60Hz
        } else if (peakHoldTimer > 0) {
            peakHoldTimer--;
        } else {
            // Decay del nivel peak lentamente después del tiempo de hold
            currentPeakLevel += (currentLevel - currentPeakLevel) * 0.02f;
            peakLevel.store(currentPeakLevel, std::memory_order_relaxed);
        }
        
        // Detección de clipping
        if (currentLevel >= -0.01f) { // Detectar clipping a -0.01 dB
            setClipDetected(true);
        }
        
        // Manejar countdown del clip hold
        if (isClipActive && clipHoldTimer > 0) {
            clipHoldTimer--;
            if (clipHoldTimer <= 0) {
                isClipActive = false;
            }
        }
        
        repaint();
    }
    
    //==========================================================================
    // OVERRIDE DE RENDERING
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.5f);
        
        // Fondo oscuro con sombra interior sutil
        g.setColour(DarkTheme::meterBackground.withAlpha(0.8f));
        g.fillRoundedRectangle(bounds, 2.0f);
        
        // Efecto de sombra interior
        g.setColour(DarkTheme::backgroundDark.withAlpha(0.4f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 1.5f, 1.0f);
        
        // Solo mostrar el gradiente si hay señal por encima del umbral (-95 dB)
        const float currentSmoothedLevel = smoothedLevel.load(std::memory_order_relaxed);
        if (currentSmoothedLevel > -95.0f) {
            // Relleno del medidor RMS con gradiente invertido (violeta arriba, azul abajo)
            g.setGradientFill(gradient);
            const auto scaledY = juce::jmap(currentSmoothedLevel, -100.f, 0.f, 0.f, static_cast<float>(getHeight() - 3));
            auto fillBounds = bounds.reduced(1.0f);
            g.fillRoundedRectangle(fillBounds.removeFromBottom(scaledY), 1.5f);
        }
        
        // Indicador de peak - línea delgada en la posición del peak
        const float currentPeakLevel = peakLevel.load(std::memory_order_relaxed);
        if (currentPeakLevel > -95.0f) {
            const auto peakY = getHeight() - 3 - juce::jmap(currentPeakLevel, -100.f, 0.f, 0.f, static_cast<float>(getHeight() - 3));
            g.setColour(DarkTheme::accentSecondary.brighter(0.3f).withAlpha(0.8f));
            g.fillRect(bounds.getX() + 1, peakY - 1, bounds.getWidth() - 2, 2.0f);
        }
        
        // Indicador de clip - línea roja en la parte superior si hay clip activo
        if (isClipActive) {
            g.setColour(DarkTheme::meterClipIndicator);
            auto clipBounds = bounds.removeFromTop(3.0f);
            g.fillRoundedRectangle(clipBounds, 1.0f);
        }
        
        // Borde exterior destacado
        g.setColour(DarkTheme::border.withAlpha(0.4f));
        g.drawRoundedRectangle(bounds, 2.0f, 0.5f);
    }
    
    void resized() override
    {
        const auto bounds = getLocalBounds().toFloat();
        if (isBypassMode)
            gradient = DarkTheme::createMeterGradient(bounds);  // Usar gradiente de entrada cuando bypass activo
        else if (isSoloScMode)
            gradient = DarkTheme::createSoloScOutputMeterGradient(bounds);
        else if (isDeltaMode)
            gradient = DarkTheme::createDeltaOutputMeterGradient(bounds);
        else
            gradient = DarkTheme::createOutputMeterGradient(bounds);
    }
        
private:
    //==========================================================================
    // VARIABLES DE ESTADO
    //==========================================================================
    
    std::function<float()> valueSupplier;
    juce::ColourGradient gradient{};
    
    // Variables de detección de clip
    int clipHoldTimer;
    bool isClipActive;
    std::atomic<float> smoothedLevel;
    std::atomic<float> peakLevel;
    int peakHoldTimer;
    bool isDeltaMode;
    bool isSoloScMode;
    bool isBypassMode;
};

//==============================================================================
// DUMMY METER (PLACEHOLDER)
//==============================================================================

/**
 * Medidor dummy para elementos que no requieren funcionalidad activa
 */
class GradientMeterInvDummy : public juce::Component
{
public:
    GradientMeterInvDummy(std::function<float()>&& valueFunction) 
        : valueSupplier(std::move(valueFunction))
    {
        // Sin actualización - medidor inactivo
    }
    
    void updateLevel()
    {
        // Método dummy para consistencia de interfaz
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.5f);
        
        // Fondo de medidor dummy (inactivo)
        g.setColour(DarkTheme::meterBackground.darker(0.3f));
        g.fillRoundedRectangle(bounds, 2.0f);
        
        // Borde sutil
        g.setColour(DarkTheme::border.withAlpha(0.2f));
        g.drawRoundedRectangle(bounds, 2.0f, 0.5f);
    }
    
    void resized() override
    {
        // Sin gradiente necesario para medidor dummy
    }
    
private:
    std::function<float()> valueSupplier;
    juce::ColourGradient gradient{};
};