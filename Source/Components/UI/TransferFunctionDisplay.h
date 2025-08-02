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

//==============================================================================
// CLASE PRINCIPAL DEL TRANSFER FUNCTION DISPLAY
//==============================================================================

/**
 * TransferFunctionDisplay para visualización de función de transferencia del expansor
 * 
 * Componente que muestra la entrada vs salida basada en parámetros
 * THD, RATIO y KNEE con capacidades interactivas. Características principales:
 * - Visualización en tiempo real de waveforms (envolventes) de entrada y salida
 * - Histograma de gain reduction/expansion con thread safety
 * - Interacción directa para modificar parámetros (drag)
 * - Modos especiales: BYPASS, DELTA, SOLO SC
 * - Sistema de zoom con 2 niveles
 * - Detección automática de silencio para optimización
 */
class TransferFunctionDisplay : public juce::Component, public juce::TooltipClient
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    TransferFunctionDisplay();
    ~TransferFunctionDisplay() override = default;

    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    //==========================================================================
    // OVERRIDES DE TOOLTIP CLIENT
    //==========================================================================
    
    juce::String getTooltip() override;
    
    //==========================================================================
    // OVERRIDES DE EVENTOS DE MOUSE
    //==========================================================================
    
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    //==========================================================================
    // MÉTODOS DE ACTUALIZACIÓN DE PARÁMETROS
    //==========================================================================
    
    void setThreshold(float gainDb);
    void setCeiling(float ceilingDb);
    void setKnee(float kneeDb);
    // MAXIMIZER: setRatio() y setRange() eliminados - no existen según CONTEXTO.txt
    void updateCurve();
    
    //==========================================================================
    // MÉTODOS DE DATOS DE WAVEFORM (THREAD-SAFE)
    //==========================================================================
    
    void updateWaveformData(const float* inputSamples, const float* processedSamples, int numSamples);
    void updateWaveformDataWithGR(const float* inputSamples, const float* processedSamples, const float* gainReductionSamples, int numSamples);
    
    // NUEVO: Método para establecer valor actual de gain reduction en tiempo real
    void setCurrentGainReduction(float grDb) noexcept { 
        currentGainReduction = grDb; 
        updateDeltaHistory(grDb);  // Actualizar histograma DELTA
    }
    
    // NUEVO: Método para establecer el parámetro RANGE para escalado dinámico
    void setRangeParameter(float rangeDb) noexcept { currentRangeParameter = rangeDb; }
    
    //==========================================================================
    // CONTROL DE VISUALIZACIÓN
    //==========================================================================
    
    void setEnvelopeVisible(bool visible);
    bool isEnvelopeVisible() const noexcept { return envelopeVisible; }
    
    //==========================================================================
    // CONTROL DE MODOS ESPECIALES
    //==========================================================================
    
    // Control de SOLO sidechain
    void setSoloSidechain(bool enabled) noexcept { soloSidechainActive = enabled; repaint(); }
    
    // Control de EXT KEY y nivel de sidechain
    void setExtKeyActive(bool active) noexcept { extKeyActive = active; repaint(); }
    void setSidechainLevel(float levelDb) noexcept { sidechainLevel = levelDb; }
    
    // Control de modo DELTA
    void setDeltaMode(bool enabled) noexcept { deltaMode = enabled; repaint(); }
    
    // Control de modo BYPASS
    void setBypassMode(bool enabled) noexcept { bypassMode = enabled; repaint(); }
    
    //==========================================================================
    // CONTROL DE ZOOM Y LIMPIEZA
    //==========================================================================
    
    // Sistema de zoom con 2 niveles
    enum class ZoomLevel { Normal = 0, Zoomed = 1 };
    void setZoomLevel(ZoomLevel level) noexcept { currentZoom = level; repaint(); }
    ZoomLevel getZoomLevel() const noexcept { return currentZoom; }
    
    // Limpieza de datos de histograma
    void clearWaveformData();
    
    
    //==========================================================================
    // CALLBACKS Y CONFIGURACIÓN
    //==========================================================================
    
    // Callbacks para notificar cambios de parámetros
    std::function<void(float)> onThresholdChange;  // Nota: mantener nombre por compatibilidad pero ahora es gain
    std::function<void(float)> onCeilingChange;
    std::function<void(float)> onKneeChange;
    // MAXIMIZER: onRatioChange y onRangeChange eliminados - no existen según CONTEXTO.txt
    
    // Configuración de estado
    void setLogicStoppedState(bool stopped);

private:
    //==========================================================================
    // PARÁMETROS DEL LIMITADOR/MAXIMIZER
    //==========================================================================
    
    float threshold = 0.0f;            // Gain por defecto en dB (a_GAIN, valores positivos)
    float ceiling = 0.0f;              // Techo máximo por defecto en dB (b_CELLING)
    float knee = 0.0f;                // Suavizado de la curva en dB
    // MAXIMIZER: ratio y range eliminados - no existen según CONTEXTO.txt
    
    //==========================================================================
    // DATOS DE WAVEFORM PARA VISUALIZACIÓN
    //==========================================================================
    
    static constexpr int waveformBufferSize = 16384;  // Buffer ampliado para mostrar más historia
    static constexpr int displayPoints = 120;         // Más puntos para visualización más ancha
    std::vector<float> inputWaveformBuffer;           // Buffer de entrada para histograma
    std::vector<float> processedWaveformBuffer;       // Buffer de salida procesada
    std::vector<float> gainReductionBuffer;           // Historial de reducción de ganancia
    std::atomic<int> waveformWriteIndex{0};         // Índice de escritura thread-safe
    std::atomic<bool> hasWaveformData{false};       // Flag de datos disponibles
    
    //==========================================================================
    // VARIABLES DE FILTRO DE ENVOLVENTE (THREAD-SAFE)
    //==========================================================================
    
    std::atomic<float> inputEnvelopeState{0.0f};      // Estado del filtro de entrada
    std::atomic<float> processedEnvelopeState{0.0f};  // Estado del filtro de salida procesada
    std::atomic<float> gainReductionSmoothed{0.0f};   // Estado suavizado para gain reduction
    
    //==========================================================================
    // DETECCIÓN DE CAMBIOS Y OPTIMIZACIONES
    //==========================================================================
    
    std::atomic<float> previousInputLevel{0.0f};      // Nivel anterior para comparación
    std::atomic<float> changeDetector{0.0f};          // Detector de cambios rápidos
    std::atomic<bool> useFastMode{false};             // Modo rápido para transitorios
    
    //==========================================================================
    // DETECCIÓN DE SILENCIO Y LIMPIEZA
    //==========================================================================
    
    std::atomic<int> silenceCounter{0};               // Contador de frames en silencio
    static constexpr int silenceThreshold = 120;     // Frames antes de limpiar (2 segundos a 60Hz)
    static constexpr float silenceLevel = 0.001f;    // Nivel de silencio (-60dB)
    std::atomic<bool> isSilent{false};               // Flag de estado silencioso
    std::atomic<float> fadeOutFactor{1.0f};          // Factor de desvanecimiento progresivo
    
    //==========================================================================
    // ESTADO DE INTERACCIÓN Y DRAG
    //==========================================================================
    
    enum class DragMode {
        None,
        Threshold,
        Knee
        // MAXIMIZER: Ratio y Range eliminados - no existen según CONTEXTO.txt
    };
    
    DragMode currentDragMode = DragMode::None;        // Modo de arrastre actual
    juce::Point<float> lastMousePos;                  // Última posición del mouse
    float dragStartValue = 0.0f;                      // Valor inicial del drag
    
    //==========================================================================
    // VARIABLES DE ESTADO Y VISUALIZACIÓN
    //==========================================================================
    
    bool envelopeVisible = true;                      // Visibilidad de envolvente
    bool soloSidechainActive = false;                 // Modo SOLO sidechain activo
    bool extKeyActive = false;                        // External key activo
    float sidechainLevel = -100.0f;                   // Nivel de sidechain en dB
    bool deltaMode = false;                           // Modo DELTA activo
    bool bypassMode = false;                          // Modo BYPASS activo

    //==========================================================================
    // MÉTODOS PRIVADOS DE DIBUJO
    //==========================================================================
    
    void drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawAxes(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawTransferCurve(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawThresholdProjection(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawRangeProjection(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawKneeArea(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawWaveformAreas(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGainReductionHistory(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawDeltaGainReduction(juce::Graphics& g, juce::Rectangle<float> bounds);  // NUEVO: Visualización específica para DELTA
    void drawDeltaGainReductionHistory(juce::Graphics& g, juce::Rectangle<float> bounds);  // NUEVO: Histograma temporal específico para DELTA
    
    // Métodos de manejo del buffer DELTA
    void updateDeltaHistory(float grDb) noexcept;  // Actualiza buffer temporal DELTA

    //==========================================================================
    // FUNCIONES MATEMÁTICAS DEL EXPANSOR
    //==========================================================================
    
    float calculateOutput(float inputDb);
    float calculateKneeOutput(float inputDb, float threshold, float knee, float ceiling);
    float calculateGainOutput(float inputDb);  // NUEVO: Solo aplicar THD como gain/drive
    float calculateLimitedOutput(float inputDb); // NUEVO: THD + limitación del ceiling
    
    //==========================================================================
    // UTILIDADES DE CONVERSIÓN Y DETECCIÓN
    //==========================================================================
    
    juce::Point<float> dbToPixel(float inputDb, float outputDb, juce::Rectangle<float> bounds);
    juce::Point<float> pixelToDb(juce::Point<float> pixel, juce::Rectangle<float> bounds);
    DragMode detectDragMode(juce::Point<float> mousePos, juce::Rectangle<float> bounds);
    bool isNearThresholdLine(juce::Point<float> mousePos, juce::Rectangle<float> bounds);
    bool isNearRangeLine(juce::Point<float> mousePos, juce::Rectangle<float> bounds);
    bool isNearKneeArea(juce::Point<float> mousePos, juce::Rectangle<float> bounds);
    bool isNearTransferCurve(juce::Point<float> mousePos, juce::Rectangle<float> bounds);
    
    //==========================================================================
    // ESTADO INTERNO ADICIONAL
    //==========================================================================
    
    ZoomLevel currentZoom = ZoomLevel::Normal;  // Por defecto modo normal
    std::atomic<bool> isLogicStopped{false};  // Estado de Logic Audio stopped
    
    // NUEVO: Valor actual de gain reduction para visualización en tiempo real
    std::atomic<float> currentGainReduction{0.0f};
    
    // NUEVO: Parámetro RANGE actual para escalado dinámico
    std::atomic<float> currentRangeParameter{-40.0f}; // Valor por defecto
    
    // NUEVO: Buffer temporal para histograma DELTA específico (Thread-Safe)
    static constexpr int deltaHistorySize = 120;  // Mismo que displayPoints
    std::atomic<float> deltaGrHistory[deltaHistorySize]; // Buffer circular thread-safe
    std::atomic<int> deltaHistoryIndex{0};       // Índice circular thread-safe
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferFunctionDisplay)
};