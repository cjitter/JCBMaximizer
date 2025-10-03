//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBMaximizer and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "TransferFunctionDisplay.h"
#include <juce_core/juce_core.h>
#include <cmath>
#include "../../Helpers/UTF8Helper.h"

//==============================================================================
// CONSTRUCTOR
//==============================================================================

juce::Range<float> TransferFunctionDisplay::getDbRange() const noexcept
{
    if (currentZoom == ZoomLevel::Normal)
        return { normalMinDb, normalMaxDb };

    const float clampedCeiling = juce::jlimit(normalMinDb, normalMaxDb, ceiling);
    float maxDb = juce::jmin(normalMaxDb, clampedCeiling + zoomTopMarginDb);
    float minDb = juce::jmax(normalMinDb, maxDb - zoomSpanDb);

    if (maxDb - minDb < 6.0f)
        minDb = maxDb - 6.0f;

    return { minDb, maxDb };
}

TransferFunctionDisplay::TransferFunctionDisplay()
{
    setOpaque(false);  // Fondo translúcido para mejor integración visual
    
    // Inicializar buffers de waveform con valores por defecto (silencio)
    inputWaveformBuffer.resize(waveformBufferSize, -100.0f);       // Buffer entrada (-100dB = silencio)
    processedWaveformBuffer.resize(waveformBufferSize, -100.0f);   // Buffer salida procesada
    gainReductionBuffer.resize(waveformBufferSize, 0.0f);          // Buffer GR (0dB = sin reducción)
    
    // Inicializar buffer específico para histograma DELTA (Thread-Safe)
    for (int i = 0; i < deltaHistorySize; ++i) {
        deltaGrHistory[i].store(0.0f, std::memory_order_relaxed);  // Buffer histograma DELTA (0dB = sin reducción)
    }
    
    // Estado inicial: sin datos de audio disponibles
    hasWaveformData.store(false);
    isSilent.store(true, std::memory_order_relaxed);
}

//==============================================================================
// OVERRIDES DE COMPONENT
//==============================================================================

void TransferFunctionDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Fondo translúcido sutil
    g.setColour(DarkTheme::backgroundMedium.withAlpha(0.3f));
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Contorno sutil
    g.setColour(DarkTheme::borderHighlight.withAlpha(0.2f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, 1.0f);
    
    // Área de gráfico (con margen para labels)
    auto graphBounds = bounds.reduced(15.0f, 10.0f);
    
    // Dibujar elementos del gráfico
    drawGrid(g, graphBounds);
    // Mostrar contenido según el modo
    if (!soloSidechainActive && !bypassMode) {
        if (deltaMode) {
            // En modo DELTA mostrar histograma temporal específico
            drawDeltaGainReductionHistory(g, graphBounds);  // NUEVO: Histograma temporal específico para DELTA
        } else if (envelopeVisible) {
            // Modo normal: solo mostrar waveforms (sin histograma de gain reduction)
            drawWaveformAreas(g, graphBounds);  // Formas de onda de entrada y salida
        }
    }
    
    // Solo mostrar elementos de compresión en modo normal
    if (!bypassMode && !deltaMode && !soloSidechainActive) {
        drawThresholdProjection(g, graphBounds);
        drawRangeProjection(g, graphBounds);
        drawKneeArea(g, graphBounds);
        drawTransferCurve(g, graphBounds);
    }
}

void TransferFunctionDisplay::resized()
{
    // Actualizar curva cuando cambie el tamaño
    updateCurve();
}

//==============================================================================
// MÉTODOS DE ACTUALIZACIÓN DE PARÁMETROS
//==============================================================================

void TransferFunctionDisplay::setThreshold(float thresholdDb)
{
    if (threshold != thresholdDb)
    {
        threshold = thresholdDb;
        updateCurve();
    }
}

void TransferFunctionDisplay::setCeiling(float ceilingDb)
{
    if (ceiling != ceilingDb)
    {
        ceiling = ceilingDb;
        updateCurve();
    }
}

void TransferFunctionDisplay::setKnee(float kneeDb)
{
    if (knee != kneeDb)
    {
        knee = kneeDb;
        updateCurve();
    }
}


void TransferFunctionDisplay::updateCurve()
{
    repaint();
}

//==============================================================================
// MÉTODOS DE CONFIGURACIÓN Y ESTADO
//==============================================================================

void TransferFunctionDisplay::setEnvelopeVisible(bool visible)
{
    if (envelopeVisible != visible)
    {
        envelopeVisible = visible;
        repaint();
    }
}

void TransferFunctionDisplay::clearWaveformData()
{
    // Limpiar todos los buffers de forma de onda
    std::fill(inputWaveformBuffer.begin(), inputWaveformBuffer.end(), -100.0f);
    std::fill(processedWaveformBuffer.begin(), processedWaveformBuffer.end(), -100.0f);
    std::fill(gainReductionBuffer.begin(), gainReductionBuffer.end(), 0.0f);
    
    // Limpiar buffer DELTA (Thread-Safe)
    for (int i = 0; i < deltaHistorySize; ++i) {
        deltaGrHistory[i].store(0.0f, std::memory_order_relaxed);
    }
    
    // Resetear estados de envolvente
    inputEnvelopeState.store(0.0f, std::memory_order_relaxed);
    processedEnvelopeState.store(0.0f, std::memory_order_relaxed);
    
    // Resetear índice de escritura y marcar que no hay datos
    waveformWriteIndex.store(0);
    hasWaveformData.store(false);
    
    // Forzar repintado
    repaint();
}

//==============================================================================
// OVERRIDES DE TOOLTIP CLIENT
//==============================================================================

juce::String TransferFunctionDisplay::getTooltip()
{
    // El tooltip se establece desde PluginEditor usando setHelpText()
    // Este método debe retornar el helpText establecido por el componente padre
    return getHelpText();
}

void TransferFunctionDisplay::setLogicStoppedState(bool stopped)
{
    bool wasStoppedBefore = isLogicStopped.load();
    isLogicStopped = stopped;
    
    // Si Logic acaba de parar o reanudar, limpiar los buffers de visualización
    if (!wasStoppedBefore && stopped)
    {
        // Logic acaba de parar - no hacer nada, mantener la última visualización
    }
    else if (wasStoppedBefore && !stopped)
    {
        // Logic acaba de reanudar - limpiar buffers para evitar mezclar datos antiguos
        clearWaveformData();
    }
}

//==============================================================================
// MÉTODOS PRIVADOS DE DIBUJO
//==============================================================================

void TransferFunctionDisplay::drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Determinar el rango de dB según el nivel de zoom
    auto range = getDbRange();
    const float minDb = range.getStart();
    const float maxDb = range.getEnd();
    
    g.setColour(DarkTheme::textSecondary.withAlpha(0.1f));
    
    // Ajustar las líneas de la grilla según el zoom
    std::vector<float> dbValues;
    std::vector<float> secondaryDbValues;

    if (currentZoom == ZoomLevel::Normal)
    {
        constexpr float majorTicks[] { 0.0f, -10.0f, -20.0f, -30.0f, -40.0f, -50.0f, -60.0f, -80.0f, -100.0f };
        constexpr float minorTicks[] { -5.0f, -15.0f, -25.0f, -35.0f, -45.0f, -55.0f, -70.0f, -90.0f };

        for (float value : majorTicks)
            if (value >= minDb && value <= maxDb)
                dbValues.push_back(value);

        for (float value : minorTicks)
            if (value >= minDb && value <= maxDb)
                secondaryDbValues.push_back(value);
    }
    else
    {
        const auto addTicks = [minDb, maxDb](float step, std::vector<float>& target)
        {
            if (step <= 0.0f)
                return;

            float startTick = std::ceil(minDb / step) * step;
            if (startTick > maxDb)
                startTick = maxDb;

            for (float value = startTick; value <= maxDb + 0.001f; value += step)
            {
                if (value >= minDb)
                    target.push_back(value);
            }
        };

        addTicks(3.0f, dbValues);
        addTicks(1.5f, secondaryDbValues);
    }

    // Grid vertical (Input dB)
    for (float db : dbValues)
    {
        const float clampedDb = juce::jlimit(minDb, maxDb, db);
        float x = juce::jmap(clampedDb, minDb, maxDb, bounds.getX(), bounds.getRight());
        g.drawVerticalLine(int(x), bounds.getY(), bounds.getBottom());
    }
    
    // Grid horizontal (Output dB - coordenadas invertidas)
    for (float db : dbValues)
    {
        const float clampedDb = juce::jlimit(minDb, maxDb, db);
        float y = juce::jmap(clampedDb, minDb, maxDb, bounds.getBottom(), bounds.getY());
        g.drawHorizontalLine(int(y), bounds.getX(), bounds.getRight());
    }
    
    // Líneas secundarias más tenues
    g.setColour(DarkTheme::textSecondary.withAlpha(0.05f));
    
    for (float db : secondaryDbValues)
    {
        const float clampedDb = juce::jlimit(minDb, maxDb, db);
        float x = juce::jmap(clampedDb, minDb, maxDb, bounds.getX(), bounds.getRight());
        g.drawVerticalLine(int(x), bounds.getY(), bounds.getBottom());
        
        float y = juce::jmap(clampedDb, minDb, maxDb, bounds.getBottom(), bounds.getY());
        g.drawHorizontalLine(int(y), bounds.getX(), bounds.getRight());
    }
}

void TransferFunctionDisplay::drawAxes(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(DarkTheme::textSecondary.withAlpha(0.4f));
    
    // Eje X (entrada) - parte inferior
    g.drawHorizontalLine(int(bounds.getBottom()), bounds.getX(), bounds.getRight());
    
    // Eje Y (salida) - parte izquierda
    g.drawVerticalLine(int(bounds.getX()), bounds.getY(), bounds.getBottom());
    
    // Labels de ejes (pequeños)
    g.setColour(DarkTheme::textPrimary.withAlpha(0.6f));
    g.setFont(7.0f);
    
    // Label entrada (abajo centro)
    g.drawText("Input (dB)", bounds.getCentreX() - 25, bounds.getBottom() + 2, 50, 8,
               juce::Justification::centred);
    
    // Label salida (izquierda centro, vertical)
    g.drawText("Output", bounds.getX() - 12, bounds.getCentreY() - 15, 12, 30,
               juce::Justification::centred);
}

void TransferFunctionDisplay::drawThresholdProjection(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // NUEVA LÓGICA: Ahora dibujamos la proyección del CEILING (que es el punto de limitación)
    // Determinar el rango de dB según el nivel de zoom
    auto range = getDbRange();
    const float minDb = range.getStart();
    const float maxDb = range.getEnd();
    
    // Solo dibujar si el ceiling está dentro del rango visible
    if (ceiling < minDb || ceiling > maxDb) return;
    
    g.setColour(DarkTheme::accent.withAlpha(0.6f));
    
    // Coordenadas del ceiling (punto de limitación)
    float ceilingX = juce::jmap(ceiling, minDb, maxDb, bounds.getX(), bounds.getRight());
    float ceilingY = juce::jmap(ceiling, minDb, maxDb, bounds.getBottom(), bounds.getY());
    
    // Línea punteada vertical desde el ceiling hasta abajo (proyección en eje X)
    juce::Path verticalDash;
    for (float y = ceilingY; y < bounds.getBottom(); y += 4.0f) {
        verticalDash.addRectangle(ceilingX - 0.5f, y, 1.0f, 2.0f);
    }
    g.fillPath(verticalDash);
    
    // Línea punteada horizontal desde el ceiling hasta la izquierda (proyección en eje Y)
    juce::Path horizontalDash;
    for (float x = bounds.getX(); x < ceilingX; x += 4.0f) {
        horizontalDash.addRectangle(x, ceilingY - 0.5f, 2.0f, 1.0f);
    }
    g.fillPath(horizontalDash);
    
    // Punto en la intersección del ceiling
    g.setColour(DarkTheme::accent);
    g.fillEllipse(ceilingX - 2, ceilingY - 2, 4, 4);
}

void TransferFunctionDisplay::drawRangeProjection(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Los limitadores no usan proyección de range, solo threshold y ceiling
}

void TransferFunctionDisplay::drawKneeArea(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (knee <= 0.0f) return; // No dibujar si knee es 0 (hard knee)
    
    // Determinar el rango de dB según el nivel de zoom
    auto range = getDbRange();
    const float minDb = range.getStart();
    const float maxDb = range.getEnd();
    
    // Calcular los puntos de inicio y fin del knee usando el ceiling
    const float rawKneeStart = ceiling - knee;
    const float rawKneeEnd = ceiling + knee;
    const float kneeStart = juce::jlimit(minDb, maxDb, rawKneeStart);
    const float kneeEnd = juce::jlimit(minDb, maxDb, rawKneeEnd);
    
    // Solo dibujar la parte visible del knee
    if (kneeStart >= maxDb || kneeEnd <= minDb) return; // Knee completamente fuera del rango visible
    
    // Dibujar el área de transición suave del knee - como una sombra muy sutil
    g.setColour(DarkTheme::textSecondary.withAlpha(0.03f));
    
    // Convertir a coordenadas de píxel usando valores recortados
    float kneeStartX = juce::jmap(kneeStart, minDb, maxDb, bounds.getX(), bounds.getRight());
    float kneeEndX = juce::jmap(kneeEnd, minDb, maxDb, bounds.getX(), bounds.getRight());
    
    juce::Rectangle<float> kneeRect(kneeStartX, bounds.getY(), kneeEndX - kneeStartX, bounds.getHeight());
    g.fillRect(kneeRect);
    
    // Dibujar líneas verticales punteadas en los extremos del knee - aún más sutiles
    g.setColour(DarkTheme::textSecondary.withAlpha(0.05f));
    
    // Línea inicio del knee (solo si está visible)
    if (rawKneeStart > minDb)
    {
        juce::Path startDash;
        const float dashX = juce::jmap(juce::jlimit(minDb, maxDb, rawKneeStart), minDb, maxDb, bounds.getX(), bounds.getRight());
        for (float y = bounds.getY(); y < bounds.getBottom(); y += 4.0f) {
            startDash.addRectangle(dashX - 0.5f, y, 1.0f, 2.0f);
        }
        g.fillPath(startDash);
    }
    
    // Línea fin del knee (solo si está visible)
    if (rawKneeEnd < maxDb)
    {
        juce::Path endDash;
        const float dashX = juce::jmap(juce::jlimit(minDb, maxDb, rawKneeEnd), minDb, maxDb, bounds.getX(), bounds.getRight());
        for (float y = bounds.getY(); y < bounds.getBottom(); y += 4.0f) {
            endDash.addRectangle(dashX - 0.5f, y, 1.0f, 2.0f);
        }
        g.fillPath(endDash);
    }
}

void TransferFunctionDisplay::drawTransferCurve(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Determinar el rango de dB según el nivel de zoom
    auto range = getDbRange();
    const float minDb = range.getStart();
    const float maxDb = range.getEnd();
    
    juce::Path curvePath;
    bool firstPoint = true;
    
    // Generar puntos de la diagonal 1:1 fija (referencia sin procesamiento)
    for (float inputDb = minDb; inputDb <= maxDb; inputDb += 0.5f)
    {
        float outputDb = inputDb;  // Diagonal 1:1 fija
        auto point = dbToPixel(inputDb, outputDb, bounds);
        
        if (firstPoint)
        {
            curvePath.startNewSubPath(point);
            firstPoint = false;
        }
        else
        {
            curvePath.lineTo(point);
        }
    }
    
    // Solo dibujar las curvas si NO estamos en modos especiales
    // (BYPASS, DELTA, SOLO SC ocultan la curva para mejor lectura de textos)
    if (!bypassMode && !deltaMode && !soloSidechainActive)
    {
        // 1. NUEVA: Dibujar línea roja del gain/drive (THD effect)
        juce::Path gainPath;
        bool firstGainPoint = true;
        
        for (float inputDb = minDb; inputDb <= maxDb; inputDb += 0.5f)
        {
            float gainOutputDb = calculateGainOutput(inputDb);
            
            // Solo dibujar puntos que estén dentro del rango visible
            if (gainOutputDb >= minDb && gainOutputDb <= maxDb)
            {
                auto point = dbToPixel(inputDb, gainOutputDb, bounds);
                
                if (firstGainPoint)
                {
                    gainPath.startNewSubPath(point);
                    firstGainPoint = false;
                }
                else
                {
                    gainPath.lineTo(point);
                }
            }
        }
        
        // Dibujar línea roja del gain/drive (muestra efecto del THD sin limitación)
        g.setColour(juce::Colours::red.withAlpha(0.6f));
        g.strokePath(gainPath, juce::PathStrokeType(1.5f));
        
        // 2. NUEVA: Dibujar línea blanca de limitación (THD + ceiling)
        juce::Path limitedPath;
        bool firstLimitedPoint = true;
        
        for (float inputDb = minDb; inputDb <= maxDb; inputDb += 0.5f)
        {
            float limitedOutputDb = calculateLimitedOutput(inputDb);
            
            // Solo dibujar puntos que estén dentro del rango visible
            if (limitedOutputDb >= minDb && limitedOutputDb <= maxDb)
            {
                auto point = dbToPixel(inputDb, limitedOutputDb, bounds);
                
                if (firstLimitedPoint)
                {
                    limitedPath.startNewSubPath(point);
                    firstLimitedPoint = false;
                }
                else
                {
                    limitedPath.lineTo(point);
                }
            }
        }
        
        // Dibujar línea blanca de limitación (resultado final del maximizer)
        g.setColour(juce::Colours::white);
        g.strokePath(limitedPath, juce::PathStrokeType(1.8f));
        
        // 3. DIAGONAL 1:1: Dibujar línea de referencia fija en verde
        g.setColour(juce::Colours::green.withAlpha(0.7f));
        g.strokePath(curvePath, juce::PathStrokeType(2.0f));
    }
}

float TransferFunctionDisplay::calculateKneeOutput(float inputDb, float threshold, float knee, float ceiling)
{
    // NUEVA LÓGICA: THD actúa como gain/drive y CEILING controla la limitación
    
    // Aplicar THD como gain primero
    float processedInput = inputDb + threshold;
    
    if (knee <= 0.0f || processedInput < ceiling - knee || processedInput > ceiling + knee)
    {
        // Fuera de la zona del knee - no debería llamarse aquí
        return processedInput;
    }
    
    // Posición en el knee normalizada de 0 a 1
    float kneePos = (processedInput - (ceiling - knee)) / (2.0f * knee);
    
    // Interpolación suave usando función coseno
    float blend = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::pi * kneePos));
    
    // Calcular outputs para ambos extremos del LIMITADOR
    float noLimitingOutput = processedInput;  // Sin limitación (1:1)
    float fullLimitingOutput = ceiling;       // Limitado al ceiling
    
    // Interpolar entre sin limitación y limitación completa
    return blend * fullLimitingOutput + (1.0f - blend) * noLimitingOutput;
}

float TransferFunctionDisplay::calculateGainOutput(float inputDb)
{
    // NUEVA FUNCIÓN: Solo aplicar a_GAIN como gain/drive (sin limitación)
    // Esto genera la línea roja paralela que muestra el efecto del GAIN
    inputDb = juce::jlimit(-100.0f, 0.0f, inputDb);
    
    // LÓGICA ADAPTADA PARA a_GAIN: valores positivos significan más gain
    // GAIN = +10 dB → línea sube 10 dB para mostrar el aumento de ganancia
    // GAIN = 0 dB → línea sin cambio (igual a entrada)
    return inputDb + threshold;  // Ahora threshold es positivo, sumar directamente
}

float TransferFunctionDisplay::calculateLimitedOutput(float inputDb)
{
    // NUEVA FUNCIÓN: Aplicar THD + limitación del ceiling con soft-knee (resultado final)
    // Esto genera la línea que muestra el comportamiento completo del maximizer
    inputDb = juce::jlimit(-100.0f, 0.0f, inputDb);
    
    // 1. Aplicar a_GAIN como gain/drive
    float gainOutput = inputDb + threshold;
    
    // 2. Aplicar limitación del ceiling con soft-knee cuadrático
    if (knee <= 0.0f)
    {
        // Hard knee - transición abrupta
        return juce::jmin(gainOutput, ceiling);
    }
    else
    {
        // Soft knee cuadrático - transición suave
        float kneeHalf = knee * 0.5f;
        float kneeStart = ceiling - kneeHalf;
        float kneeEnd = ceiling + kneeHalf;

        if (gainOutput <= kneeStart)
        {
            // Sin limitación (línea recta)
            return gainOutput;
        }
        else if (gainOutput >= kneeEnd)
        {
            // Limitación completa al ceiling
            return ceiling;
        }
        else
        {
            // Dentro del knee: interpolación Hermite cúbica para transición suave
            float t = (gainOutput - kneeStart) / knee; // normalizar 0-1

            // Puntos de conexión exactos para LIMITADOR
            float y0 = kneeStart;  // punto inicial (sin limitación)
            float y1 = ceiling;    // punto final (limitado al ceiling)

            // Derivadas en los extremos (pendientes de las líneas)
            float m0 = 1.0f;   // pendiente línea sin limitación
            float m1 = 0.0f;   // pendiente línea limitada (horizontal)

            // Interpolación Hermite cúbica que respeta puntos y derivadas
            float t2 = t * t;
            float t3 = t2 * t;

            float h00 = 2*t3 - 3*t2 + 1;
            float h10 = t3 - 2*t2 + t;
            float h01 = -2*t3 + 3*t2;
            float h11 = t3 - t2;

            return h00*y0 + h10*knee*m0 + h01*y1 + h11*knee*m1;
        }
    }
}

//==============================================================================
// FUNCIONES MATEMÁTICAS DEL EXPANSOR
//==============================================================================

float TransferFunctionDisplay::calculateOutput(float inputDb)
{
    inputDb = juce::jlimit(-100.0f, 0.0f, inputDb);

    // NUEVA LÓGICA: THD actúa como gain/drive y CEILING controla la limitación
    // Aplicar THD como gain primero (boost/cut del input)
    float processedInput = inputDb + threshold;  // THD como gain/drive

    float output;

    if (knee <= 0.0f)
    {
        // Hard knee - LIMITADOR LOGIC
        // CEILING es ahora el punto de limitación
        if (processedInput <= ceiling)
            output = processedInput;  // Señales bajo ceiling pasan sin cambio (1:1)
        else
            output = ceiling;  // Limitar al ceiling
    }
    else
    {
        // Soft knee - LIMITADOR LOGIC
        float kneeHalf = knee * 0.5f;
        float kneeStart = ceiling - kneeHalf;  // CEILING es el punto de limitación
        float kneeEnd = ceiling + kneeHalf;

        if (processedInput <= kneeStart)
        {
            // Línea 1:1 (sin limitación)
            output = processedInput;
        }
        else if (processedInput >= kneeEnd)
        {
            // Línea limitada al ceiling
            output = ceiling;
        }
        else
        {
            // Dentro del knee: interpolación hermite cúbica
            float t = (processedInput - kneeStart) / knee; // normalizar 0-1

            // Puntos de conexión exactos para LIMITADOR
            float y0 = kneeStart;  // punto inicial (línea 1:1)
            float y1 = ceiling;    // punto final (limitado al ceiling)

            // Derivadas en los extremos (pendientes de las líneas)
            float m0 = 1.0f;   // pendiente línea 1:1
            float m1 = 0.0f;   // pendiente línea limitada (horizontal)

            // Interpolación Hermite que respeta puntos y derivadas
            float t2 = t * t;
            float t3 = t2 * t;

            float h00 = 2*t3 - 3*t2 + 1;
            float h10 = t3 - 2*t2 + t;
            float h01 = -2*t3 + 3*t2;
            float h11 = t3 - t2;

            output = h00 * y0 + h10 * knee * m0 + h01 * y1 + h11 * knee * m1;
        }
    }
    
    return output;
}


juce::Point<float> TransferFunctionDisplay::dbToPixel(float inputDb, float outputDb, juce::Rectangle<float> bounds)
{
    // Determinar el rango de dB según el nivel de zoom
    auto range = getDbRange();
    const float minDb = range.getStart();
    const float maxDb = range.getEnd();
    
    // X: Input va de izquierda (minDb) a derecha (maxDb)
    float x = juce::jmap(inputDb, minDb, maxDb, bounds.getX(), bounds.getRight());
    
    // Y: Output va de abajo (minDb) a arriba (maxDb) - coordenadas invertidas
    float y = juce::jmap(outputDb, minDb, maxDb, bounds.getBottom(), bounds.getY());
    
    return {x, y};
}

juce::Point<float> TransferFunctionDisplay::pixelToDb(juce::Point<float> pixel, juce::Rectangle<float> bounds)
{
    // Determinar el rango de dB según el nivel de zoom
    auto range = getDbRange();
    const float minDb = range.getStart();
    const float maxDb = range.getEnd();
    
    // Convertir coordenadas de píxel a dB
    float inputDb = juce::jmap(pixel.x, bounds.getX(), bounds.getRight(), minDb, maxDb);
    float outputDb = juce::jmap(pixel.y, bounds.getBottom(), bounds.getY(), minDb, maxDb);
    
    return {inputDb, outputDb};
}

//==============================================================================
// MÉTODOS DE DATOS DE WAVEFORM (THREAD-SAFE)
//==============================================================================

void TransferFunctionDisplay::updateWaveformData(const float* inputSamples, const float* processedSamples, int numSamples)
{
    if (numSamples <= 0) return;
    
    int writeIndex = waveformWriteIndex.load();
    
    // Verificar si hay señal o silencio
    float maxSignalLevel = 0.0f;
    for (int i = 0; i < numSamples; i += 64)  // Chequeo rápido cada 64 muestras
    {
        maxSignalLevel = juce::jmax(maxSignalLevel, std::abs(inputSamples[i]));
        if (maxSignalLevel > silenceLevel) break;
    }
    
    // Actualizar contador de silencio
    if (maxSignalLevel < silenceLevel)
    {
        int currentSilenceCounter = silenceCounter.load(std::memory_order_relaxed);
        silenceCounter.store(currentSilenceCounter + 1, std::memory_order_relaxed);
        
        if (currentSilenceCounter > silenceThreshold)
        {
            isSilent.store(true, std::memory_order_relaxed);
            // Comenzar fade out progresivo
            float currentFade = fadeOutFactor.load(std::memory_order_relaxed);
            fadeOutFactor.store(currentFade * 0.95f, std::memory_order_relaxed);  // Desvanecimiento gradual
            
            // NO limpiar el buffer inmediatamente, solo aplicar fade
            float currentInputEnv = inputEnvelopeState.load(std::memory_order_relaxed);
            float currentProcessedEnv = processedEnvelopeState.load(std::memory_order_relaxed);
            inputEnvelopeState.store(currentInputEnv * 0.95f, std::memory_order_relaxed);
            processedEnvelopeState.store(currentProcessedEnv * 0.95f, std::memory_order_relaxed);
            
            // Solo limpiar cuando el fade es muy bajo
            if (currentFade < 0.05f)
            {
                fadeOutFactor.store(0.0f, std::memory_order_relaxed);
                // Mantener los datos pero con fade completo
            }
        }
    }
    else
    {
        silenceCounter.store(0, std::memory_order_relaxed);
        isSilent.store(false, std::memory_order_relaxed);
        fadeOutFactor.store(1.0f, std::memory_order_relaxed);  // Restaurar opacidad completa
        hasWaveformData.store(true);
    }
    
    // Procesar TODO el bloque de una vez para obtener un solo valor de envolvente
    float maxInput = 0.0f;
    float maxProcessed = 0.0f;
    
    // Obtener el máximo del bloque completo
    // NOTA: Intercambiamos input y processed porque vienen invertidos del processor
    for (int i = 0; i < numSamples; ++i)
    {
        maxInput = juce::jmax(maxInput, std::abs(processedSamples[i]));  // processed es realmente la entrada post-TRIM
        maxProcessed = juce::jmax(maxProcessed, std::abs(inputSamples[i]));  // input es realmente la salida procesada
    }
    
    // Si estamos en silencio, aplicar fade progresivo
    if (isSilent.load(std::memory_order_relaxed))
    {
        float currentFade = fadeOutFactor.load(std::memory_order_relaxed);
        maxInput *= currentFade;
        maxProcessed *= currentFade;
    }
    
    // Detectar cambios rápidos en la señal
    float prevLevel = previousInputLevel.load(std::memory_order_relaxed);
    float inputChange = std::abs(maxInput - prevLevel);
    float currentDetector = changeDetector.load(std::memory_order_relaxed);
    changeDetector.store(currentDetector * 0.9f + inputChange * 0.1f, std::memory_order_relaxed);
    previousInputLevel.store(maxInput, std::memory_order_relaxed);
    
    // Cambiar a modo rápido si detectamos transientes
    useFastMode.store(currentDetector > 0.1f, std::memory_order_relaxed);
    
    // Parámetros para envelope más "encrespado" y abrupto
    float releaseTime = 0.3f;   // Release MUY rápido para máxima textura
    
    // Filtro de envolvente mínimo para mantener el carácter "encrespado"
    {
        // Para entrada - seguimiento directo sin ruido artificial
        float clampedInput = juce::jmin(maxInput, 0.99f); // Evitar que supere 0 dBFS
        float currentInputEnv = inputEnvelopeState.load(std::memory_order_relaxed);
        
        if (clampedInput > currentInputEnv * 1.1f)  // Umbral más bajo para más respuesta
        {
            // Attack instantáneo
            inputEnvelopeState.store(clampedInput, std::memory_order_relaxed);
        }
        else
        {
            // Release muy abrupto para mantener detalles
            float newValue = currentInputEnv * releaseTime + clampedInput * (1.0f - releaseTime);
            
            // Corte abrupto en silencio
            if (newValue < 0.001f) newValue = 0.0f;
            inputEnvelopeState.store(newValue, std::memory_order_relaxed);
        }
        
        // Para salida procesada - igual de directo sin ruido
        float currentProcessedEnv = processedEnvelopeState.load(std::memory_order_relaxed);
        
        if (maxProcessed > currentProcessedEnv * 1.1f)
        {
            // Attack instantáneo
            processedEnvelopeState.store(maxProcessed, std::memory_order_relaxed);
        }
        else
        {
            // Mezclar con la señal actual para más detalle
            float newValue = currentProcessedEnv * releaseTime + maxProcessed * (1.0f - releaseTime);
            
            // Corte abrupto
            if (newValue < 0.001f) newValue = 0.0f;
            processedEnvelopeState.store(newValue, std::memory_order_relaxed);
        }
        
        // Re-leer los valores finales para convertir a dB
        float finalInputEnv = inputEnvelopeState.load(std::memory_order_relaxed);
        float finalProcessedEnv = processedEnvelopeState.load(std::memory_order_relaxed);
        
        // Convertir a dB sin ruido artificial
        float inputDb = finalInputEnv > 0.0001f ?
            20.0f * std::log10(finalInputEnv) : -100.0f;
        float processedDb = finalProcessedEnv > 0.0001f ?
            20.0f * std::log10(finalProcessedEnv) : -100.0f;
        
        // Limitar al rango del gráfico (-100 dB a 0 dB)
        // Permitir llegar hasta 0 dB para consistencia con el rango visual
        inputDb = juce::jlimit(-100.0f, 0.0f, inputDb);
        processedDb = juce::jlimit(-100.0f, 0.0f, processedDb);
        
        inputWaveformBuffer[writeIndex] = inputDb;
        processedWaveformBuffer[writeIndex] = processedDb;
        
        // Calcular y almacenar la reducción de ganancia (positivo = reducción)
        float gainReduction = inputDb - processedDb;
        
        // No hay condiciones especiales de ratio como en expanders
        
        // Verificar si la señal está por debajo del threshold (considerando knee)
        // Si la señal está por debajo de (threshold - knee), no debería haber compresión
        if (inputDb < (threshold - knee))
        {
            gainReduction = 0.0f;
            // Forzar que las envolventes sean idénticas cuando no hay compresión
            processedDb = inputDb;
            processedWaveformBuffer[writeIndex] = inputDb;
        }
        
        // En este caso, no debería haber compresión visible
        if (extKeyActive && sidechainLevel < -60.0f)
        {
            gainReduction = 0.0f;
            processedDb = inputDb;
            processedWaveformBuffer[writeIndex] = inputDb;
        }
        
        
        gainReductionBuffer[writeIndex] = gainReduction;  // Allow negative values for expander
    }
    
    // Solo escribir UN valor por llamada (no múltiples)
    writeIndex = (writeIndex + 1) % waveformBufferSize;
    
    waveformWriteIndex.store(writeIndex);
    hasWaveformData.store(true);
}

void TransferFunctionDisplay::updateWaveformDataWithGR(const float* inputSamples, const float* processedSamples, const float* gainReductionSamples, int numSamples)
{
    if (numSamples <= 0) return;
    
    int writeIndex = waveformWriteIndex.load();
    
    // Procesar todas las muestras disponibles
    for (int i = 0; i < numSamples; i++)
    {
        // Encontrar el valor máximo en las muestras (detector de picos)
        float maxInput = std::abs(inputSamples[i]);
        float maxProcessed = std::abs(processedSamples[i]);
        
        // Actualizar el estado del envelope follower
        // Similar al método anterior pero usando la GR real de Gen~
        
        // Detectar cambios rápidos en la señal
        float inputChange = std::abs(maxInput - previousInputLevel);
        changeDetector = changeDetector * 0.9f + inputChange * 0.1f;
        previousInputLevel = maxInput;
        
        // Cambiar a modo rápido si detectamos transientes
        useFastMode = changeDetector > 0.1f;
        
        // Parámetros independientes para diferentes tipos de visualización
        //float envelopeReleaseTime = 0.3f;   // Release rápido para envolventes (comportamiento original)
        
        // Factor de suavizado por defecto para gain reduction (valor medio)
        // grSmoothingFactor = 0.7f;  // Equivalente a ~100ms de release - REMOVIDO (no usado)
        
        // TEMPORAL DEBUG - BYPASS ENVELOPE FILTERING
        /*
        // Filtro de envolvente original (COMENTADO TEMPORALMENTE)
        {
            // Para entrada - seguimiento directo sin ruido artificial
            float clampedInput = juce::jmin(maxInput, 0.99f); // Evitar que supere 0 dBFS
            float currentInputEnv = inputEnvelopeState.load(std::memory_order_relaxed);
            
            if (clampedInput > currentInputEnv * 1.1f)  // Umbral más bajo para más respuesta
            {
                // Attack instantáneo
                inputEnvelopeState.store(clampedInput, std::memory_order_relaxed);
            }
            else
            {
                // Release muy abrupto para mantener detalles
                float newValue = currentInputEnv * envelopeReleaseTime + clampedInput * (1.0f - envelopeReleaseTime);
                
                // Corte abrupto en silencio
                if (newValue < 0.001f) newValue = 0.0f;
                inputEnvelopeState.store(newValue, std::memory_order_relaxed);
            }
            
            // Para salida procesada - igual de directo sin ruido
            float currentProcessedEnv = processedEnvelopeState.load(std::memory_order_relaxed);
            
            if (maxProcessed > currentProcessedEnv * 1.1f)
            {
                // Attack instantáneo
                processedEnvelopeState.store(maxProcessed, std::memory_order_relaxed);
            }
            else
            {
                // Mezclar con la señal actual para más detalle
                float newValue = currentProcessedEnv * envelopeReleaseTime + maxProcessed * (1.0f - envelopeReleaseTime);
                
                // Corte abrupto
                if (newValue < 0.001f) newValue = 0.0f;
                processedEnvelopeState.store(newValue, std::memory_order_relaxed);

            }
            
            // Re-leer los valores finales para convertir a dB
            float finalInputEnv = inputEnvelopeState.load(std::memory_order_relaxed);
            float finalProcessedEnv = processedEnvelopeState.load(std::memory_order_relaxed);
            
            // Convertir a dB sin ruido artificial
            float inputDb = finalInputEnv > 0.0001f ?
                20.0f * std::log10(finalInputEnv) : -100.0f;
            float processedDb = finalProcessedEnv > 0.0001f ?
                20.0f * std::log10(finalProcessedEnv) : -100.0f;
            
            // Limitar al rango del gráfico (-100 dB a 0 dB)
            inputDb = juce::jlimit(-100.0f, 0.0f, inputDb);
            processedDb = juce::jlimit(-100.0f, 0.0f, processedDb);
            
            inputWaveformBuffer[writeIndex] = inputDb;
            processedWaveformBuffer[writeIndex] = processedDb;
        }
        */
        
        // DIRECT CONVERSION - NO FILTERING (DEBUG)
        {
            float inputDb = maxInput > 0.0001f ?
                20.0f * std::log10(maxInput) : -100.0f;
            float processedDb = maxProcessed > 0.0001f ?
                20.0f * std::log10(maxProcessed) : -100.0f;
            
            // SIEMPRE escribir los datos originales primero
            inputWaveformBuffer[writeIndex] = inputDb;
            processedWaveformBuffer[writeIndex] = processedDb;
            
            // NUEVO: Usar datos reales de gain reduction de Gen~
            if (gainReductionSamples != nullptr)
            {
                // Convertir gain reduction real de Gen~ a dB
                float grLinear = gainReductionSamples[i];
                float grDb = grLinear > 0.0001f ? 20.0f * std::log10(grLinear) : -60.0f;
                
                // Para expansor: valores negativos indican expansión (atenuación)
                // Los datos de Gen~ ya vienen en el formato correcto
                gainReductionBuffer[writeIndex] = grDb;
            }
            else
            {
                // Fallback: calcular gain reduction basándose en diferencia de envolventes
                // Para expansor: cuando processed < input hay expansión (valor positivo)
                float calculatedGR = inputDb - processedDb;
                gainReductionBuffer[writeIndex] = calculatedGR;
            }
        }
        
        // Solo escribir UN valor por llamada (no múltiples)
        writeIndex = (writeIndex + 1) % waveformBufferSize;
    }
    
    waveformWriteIndex.store(writeIndex);
    hasWaveformData.store(true);
}

void TransferFunctionDisplay::drawWaveformAreas(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (!hasWaveformData.load()) return;
    
    // Si Logic está parado, no dibujar las envolventes
    if (isLogicStopped.load()) return;
    
    const int readIndex = waveformWriteIndex.load();
    const float currentFadeOutFactor = fadeOutFactor.load(std::memory_order_relaxed);
    
    // NO expandir bounds - mantener dentro del área del gráfico
    // bounds = bounds.expanded(60.0f, 10.0f);  // COMENTADO - causaba el bloque azul
    
    // Determinar el rango de dB según el nivel de zoom (una sola vez, fuera del loop)
    auto range = getDbRange();
    const float minDb = range.getStart();
    const float maxDb = range.getEnd();
    
    // Crear paths para áreas rellenas
    juce::Path inputAreaPath;
    juce::Path processedAreaPath;   // Salida procesada
    
    // Línea base (parte inferior del gráfico) - con pequeño offset para evitar artefactos
    const float baseLine = bounds.getBottom() - 1.0f;
    
    // Colectar todos los puntos primero para suavizado
    std::vector<juce::Point<float>> inputPoints;
    std::vector<juce::Point<float>> processedPoints;    // Salida procesada
    
    // Mostrar los últimos 'displayPoints' valores del buffer circular con menos delay
    for (int i = 0; i < displayPoints; ++i)
    {
        // Leer con offset mínimo para entrada instantánea
        int samplesBack = displayPoints - i + 5;  // Offset mínimo para sincronía audio-visual
        int bufferIndex = (readIndex - samplesBack + waveformBufferSize) % waveformBufferSize;
        
        float inputDb = inputWaveformBuffer[bufferIndex];
        float processedDb = processedWaveformBuffer[bufferIndex];
        
        // Limitar valores al rango visible actual
        if (inputDb < minDb) inputDb = minDb;
        if (inputDb > maxDb) inputDb = maxDb;
        if (processedDb < minDb) processedDb = minDb;
        if (processedDb > maxDb) processedDb = maxDb;
        
        // Calcular posición X - usar todo el ancho disponible
        float normalizedTime = float(i) / float(displayPoints - 1);
        float x = bounds.getX() + normalizedTime * bounds.getWidth();
        
        // Calcular posiciones Y usando el rango de zoom actual
        // Añadir un pequeño offset para evitar que las envolventes toquen exactamente el borde superior
        float topOffset = 5.0f; // 5 píxeles de margen desde el borde superior (bajado desde 2.0f)
        float inputY = juce::jmap(inputDb, minDb, maxDb, bounds.getBottom(), bounds.getY() + topOffset);
        float processedY = juce::jmap(processedDb, minDb, maxDb, bounds.getBottom(), bounds.getY() + topOffset);
        
        // (los limitadores generalmente tienen diferencias más visibles que los expanders)
        
        // Sin fade en los extremos - usar todo el ancho disponible
        // Limitar con pequeño margen para evitar artefactos en los bordes
        inputY = juce::jlimit(bounds.getY() + 3.0f, bounds.getBottom() - 1.0f, inputY);
        processedY = juce::jlimit(bounds.getY() + 3.0f, bounds.getBottom() - 1.0f, processedY);
        
        inputPoints.push_back({x, inputY});
        processedPoints.push_back({x, processedY});
    }
    
    // Crear path suavizado para salida procesada
    if (!processedPoints.empty())
    {
        processedAreaPath.startNewSubPath(processedPoints[0].x, baseLine);
        processedAreaPath.lineTo(processedPoints[0]);
        
        // Interpolación lineal simple para evitar artefactos
        for (size_t i = 1; i < processedPoints.size(); ++i)
        {
            processedAreaPath.lineTo(processedPoints[i]);
        }
        
        processedAreaPath.lineTo(processedPoints.back().x, baseLine);
        processedAreaPath.closeSubPath();
    }
    
    // Crear path suavizado para entrada
    if (!inputPoints.empty())
    {
        inputAreaPath.startNewSubPath(inputPoints[0].x, baseLine);
        inputAreaPath.lineTo(inputPoints[0]);
        
        // Interpolación lineal simple para evitar artefactos
        for (size_t i = 1; i < inputPoints.size(); ++i)
        {
            inputAreaPath.lineTo(inputPoints[i]);
        }
        
        inputAreaPath.lineTo(inputPoints.back().x, baseLine);
        inputAreaPath.closeSubPath();
    }
    
    // Dibujar sin máscara para usar todo el ancho
    
    // 1. PRIMERO dibujar área de ENTRADA (DETRÁS) - Morado translúcido  
    // CORRECCIÓN: inputAreaPath es realmente para entrada, processedAreaPath para salida
    if (!inputAreaPath.isEmpty())
    {
        // Gradiente vertical para ENTRADA - MORADO GRISÁCEO MÁS SUTIL
        juce::ColourGradient inputGradient(
            juce::Colour(0x80, 0x70, 0x85).withAlpha(0.6f * currentFadeOutFactor), bounds.getCentreX(), bounds.getY(),
            juce::Colour(0x65, 0x58, 0x6A).withAlpha(0.4f * currentFadeOutFactor), bounds.getCentreX(), bounds.getBottom(),
            false
        );
        g.setGradientFill(inputGradient);
        g.fillPath(inputAreaPath);
        
        // Línea superior morada grisácea para ENTRADA - más sutil
        g.setColour(juce::Colour(0x80, 0x70, 0x85).withAlpha(0.8f * currentFadeOutFactor));
        juce::Path inputLine;
        if (!inputPoints.empty())
        {
            inputLine.startNewSubPath(inputPoints[0]);
            for (size_t i = 1; i < inputPoints.size(); ++i)
            {
                inputLine.lineTo(inputPoints[i]);
            }
            g.strokePath(inputLine, juce::PathStrokeType(1.5f));  // Línea más gruesa para entrada
        }
    }
    
    // 2. DESPUÉS dibujar área de SALIDA PROCESADA (ENCIMA) - Usando gradiente del medidor de salida
    // CORRECCIÓN: processedAreaPath es realmente para salida procesada  
    if (!processedAreaPath.isEmpty())
    {
        // Usar el mismo gradiente que los medidores de salida
        const juce::Colour outputBlue = juce::Colour(0xFF6495ED);     // Azul de OUTPUT
        const juce::Colour darkPurple = juce::Colour(0xFF202245);     // Morado oscuro personalizado
        const juce::Colour deepBlue = juce::Colour(0xFF0D3B52);       // Azul profundo intermedio
        
        // Crear gradiente con transparencia para la envolvente
        auto outputGradient = juce::ColourGradient(
            outputBlue.withAlpha(0.6f * currentFadeOutFactor), bounds.getCentreX(), bounds.getY(),
            darkPurple.withAlpha(0.4f * currentFadeOutFactor), bounds.getCentreX(), bounds.getBottom(),
            false
        );
        outputGradient.addColour(0.15, outputBlue.withAlpha(0.58f * currentFadeOutFactor));  // Azul claro se mantiene más arriba
        outputGradient.addColour(0.4, outputBlue.interpolatedWith(deepBlue, 0.3f).withAlpha(0.55f * currentFadeOutFactor));  // Transición más suave
        outputGradient.addColour(0.65, deepBlue.withAlpha(0.5f * currentFadeOutFactor));
        outputGradient.addColour(0.85, deepBlue.interpolatedWith(darkPurple, 0.5f).withAlpha(0.45f * currentFadeOutFactor));
        
        g.setGradientFill(outputGradient);
        g.fillPath(processedAreaPath);
        
        // Línea superior con el color azul OUTPUT para SALIDA PROCESADA
        g.setColour(outputBlue.withAlpha(0.95f * currentFadeOutFactor));
        juce::Path processedLine;
        if (!processedPoints.empty())
        {
            processedLine.startNewSubPath(processedPoints[0]);
            for (size_t i = 1; i < processedPoints.size(); ++i)
            {
                processedLine.lineTo(processedPoints[i]);
            }
            g.strokePath(processedLine, juce::PathStrokeType(1.0f));  // Línea más delgada para salida
        }
    }
    
}

//==============================================================================
// Eventos de mouse

void TransferFunctionDisplay::mouseEnter(const juce::MouseEvent&)
{
    // Tooltip ya está configurado en el constructor
}

void TransferFunctionDisplay::mouseExit(const juce::MouseEvent&)
{
    // Resetear cursor
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void TransferFunctionDisplay::mouseDown(const juce::MouseEvent& e)
{
    auto bounds = getLocalBounds().toFloat().reduced(15.0f, 10.0f);
    auto mousePos = e.position;
    
    // Detectar qué elemento se está arrastrando
    currentDragMode = detectDragMode(mousePos, bounds);
    lastMousePos = mousePos;
    
    // Guardar valor inicial para el drag
    switch (currentDragMode) {
        case DragMode::Threshold:
            // NUEVA LÓGICA: Guardar valor del ceiling ya que es lo que se arrastra
            dragStartValue = ceiling;
            break;
        case DragMode::Knee:
            dragStartValue = knee;
            break;
        default:
            break;
    }
}

void TransferFunctionDisplay::mouseDrag(const juce::MouseEvent& e)
{
    if (currentDragMode == DragMode::None) return;
    
    auto bounds = getLocalBounds().toFloat().reduced(15.0f, 10.0f);
    auto mousePos = e.position;
    auto deltaPixels = mousePos - lastMousePos;
    
    switch (currentDragMode) {
        case DragMode::Threshold: {
            // NUEVA LÓGICA: Arrastrar horizontalmente para cambiar CEILING (que controla la limitación visible)
            float deltaDb = juce::jmap(deltaPixels.x, 0.0f, bounds.getWidth(), 0.0f, 60.0f);
            float newCeiling = juce::jlimit(-60.0f, 0.0f, ceiling + deltaDb);
            setCeiling(newCeiling);
            if (onCeilingChange) onCeilingChange(newCeiling);
            break;
        }
        case DragMode::Knee: {
            // Arrastrar horizontalmente para cambiar knee
            float deltaDb = juce::jmap(deltaPixels.x, 0.0f, bounds.getWidth(), 0.0f, 30.0f);
            float newKnee = juce::jlimit(0.0f, 20.0f, knee + deltaDb);
            setKnee(newKnee);
            if (onKneeChange) onKneeChange(newKnee);
            break;
        }
        default:
            break;
    }
    
    lastMousePos = mousePos;
}

void TransferFunctionDisplay::mouseUp(const juce::MouseEvent&)
{
    currentDragMode = DragMode::None;
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

//==============================================================================
// Métodos de detección

TransferFunctionDisplay::DragMode TransferFunctionDisplay::detectDragMode(juce::Point<float> mousePos, juce::Rectangle<float> bounds)
{
    // Prioridad: Threshold > Range > Knee > Ratio
    if (isNearThresholdLine(mousePos, bounds)) {
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        return DragMode::Threshold;
    }
    if (isNearKneeArea(mousePos, bounds)) {
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        return DragMode::Knee;
    }
    
    setMouseCursor(juce::MouseCursor::NormalCursor);
    return DragMode::None;
}

bool TransferFunctionDisplay::isNearThresholdLine(juce::Point<float> mousePos, juce::Rectangle<float> bounds)
{
    // NUEVA LÓGICA: Detectar proximidad a la línea del CEILING (que es el punto de limitación)
    // Determinar el rango de dB según el nivel de zoom
    auto range = getDbRange();
    const float minDb = range.getStart();
    const float maxDb = range.getEnd();
    
    // Solo verificar si el ceiling está dentro del rango visible
    if (ceiling < minDb || ceiling > maxDb) return false;
    
    float ceilingX = juce::jmap(ceiling, minDb, maxDb, bounds.getX(), bounds.getRight());
    return std::abs(mousePos.x - ceilingX) < 8.0f; // Tolerancia de 8 píxeles
}

bool TransferFunctionDisplay::isNearRangeLine(juce::Point<float> /*mousePos*/, juce::Rectangle<float> /*bounds*/)
{
    return false;
}

bool TransferFunctionDisplay::isNearKneeArea(juce::Point<float> mousePos, juce::Rectangle<float> bounds)
{
    if (knee <= 0.0f) return false; // No hay área de knee si es hard knee
    
    // Verificar si el mouse está en la zona del knee
    auto dbPos = pixelToDb(mousePos, bounds);
    float kneeStart = threshold - knee;
    float kneeEnd = threshold + knee;
    
    // Verificar si está dentro del rango horizontal del knee
    return dbPos.x >= kneeStart && dbPos.x <= kneeEnd;
}

bool TransferFunctionDisplay::isNearTransferCurve(juce::Point<float> mousePos, juce::Rectangle<float> bounds)
{
    // Calcular si el mouse está cerca de la curva de transferencia
    auto dbPos = pixelToDb(mousePos, bounds);
    float expectedOutput = calculateOutput(dbPos.x);
    float distanceToCurve = std::abs(dbPos.y - expectedOutput);
    return distanceToCurve < 4.0f; // Tolerancia de 4 dB
}

void TransferFunctionDisplay::drawGainReductionHistory(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (!hasWaveformData.load()) return;
    
    // Si Logic está parado, no dibujar el histograma
    if (isLogicStopped.load()) return;
    
    const int readIndex = waveformWriteIndex.load();
    const float currentFadeOutFactor = fadeOutFactor.load(std::memory_order_relaxed);
    
    // Determinar el rango de gain reduction según el nivel de zoom
    // NUEVO: Rangos ajustados para expansor (valores típicos más pequeños que compresor)
    float maxGainReduction;
    bool useNonLinearScale = false;
    switch (currentZoom)
    {
        case ZoomLevel::Normal:
            maxGainReduction = 12.0f;  // Expansor: rango típico más pequeño (0-12dB)
            break;
        case ZoomLevel::Zoomed:
            maxGainReduction = 6.0f;   // Expansor: zoom para rangos pequeños (0-6dB)
            break;
    }
    
    // Crear path para la línea de reducción de ganancia
    juce::Path grPath;
    std::vector<juce::Point<float>> grPoints;
    
    // Recolectar puntos de reducción de ganancia
    for (int i = 0; i < displayPoints; ++i)
    {
        int samplesBack = displayPoints - i + 5;
        int bufferIndex = (readIndex - samplesBack + waveformBufferSize) % waveformBufferSize;
        
        float grDb = gainReductionBuffer[bufferIndex];
        
        // Posición X - usar todo el ancho
        float normalizedTime = float(i) / float(displayPoints - 1);
        float x = bounds.getX() + normalizedTime * bounds.getWidth();
        
        // Posición Y - mapear GR al rango vertical según el zoom
        // Para expansor: manejar valores positivos (amplificación) y negativos (atenuación)
        float normalizedGR;
        float y;
        
        if (grDb >= 0.0f) {
            // Valores positivos (amplificación) - mapear a la mitad superior
            normalizedGR = juce::jlimit(0.0f, 1.0f, grDb / maxGainReduction);
            y = bounds.getY() + (0.5f - normalizedGR * 0.5f) * bounds.getHeight();
        } else {
            // Valores negativos (atenuación) - mapear a la mitad inferior
            normalizedGR = juce::jlimit(0.0f, 1.0f, std::abs(grDb) / maxGainReduction);
            
            if (useNonLinearScale)
            {
                // Aplicar escala no lineal para expandir visualmente el rango 0 a -6 dB
                float expandedGR = std::pow(normalizedGR, 0.75f);  
                y = bounds.getY() + (0.5f + expandedGR * 0.5f) * bounds.getHeight();
            }
            else
            {
                // Escala lineal normal
                y = bounds.getY() + (0.5f + normalizedGR * 0.5f) * bounds.getHeight();
            }
        }
        
        grPoints.push_back({x, y});
    }
    
    // Crear path suavizado
    if (!grPoints.empty())
    {
        if (deltaMode)
        {
            // En modo DELTA: usar color verde (mismo que el fondo de delta)
            // Color verde tipo teal/cyan
            auto deltaGreen = juce::Colour(0x00, 0xC8, 0x96);  // Verde turquesa
            
            // Crear gradiente verde para el área
            juce::ColourGradient grGradient(
                deltaGreen.withAlpha(0.6f * currentFadeOutFactor), bounds.getX(), bounds.getY(),
                deltaGreen.darker(0.3f).withAlpha(0.3f * currentFadeOutFactor), bounds.getRight(), bounds.getY(),
                false
            );
            
            // Dibujar área bajo la curva - empezar sin offset
            grPath.startNewSubPath(grPoints[0].x, bounds.getY());
            grPath.lineTo(grPoints[0]);
            
            // Suavizado con spline
            for (size_t i = 1; i < grPoints.size(); ++i)
            {
                if (i < grPoints.size() - 1)
                {
                    auto p0 = (i > 1) ? grPoints[i - 2] : grPoints[i - 1];
                    auto p1 = grPoints[i - 1];
                    auto p2 = grPoints[i];
                    auto p3 = (i < grPoints.size() - 1) ? grPoints[i + 1] : grPoints[i];
                    
                    for (float t = 0.0f; t < 1.0f; t += 0.2f)
                    {
                        float t2 = t * t;
                        float t3 = t2 * t;
                        
                        float x = 0.5f * ((2.0f * p1.x) +
                                         (-p0.x + p2.x) * t +
                                         (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
                                         (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3);
                        
                        float y = 0.5f * ((2.0f * p1.y) +
                                         (-p0.y + p2.y) * t +
                                         (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
                                         (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3);
                        
                        grPath.lineTo(x, y);
                    }
                }
                else
                {
                    grPath.lineTo(grPoints[i]);
                }
            }
            
            grPath.lineTo(grPoints.back().x, bounds.getY());
            grPath.closeSubPath();
            
            // En modo DELTA: dibujar área rellena verde
            g.setGradientFill(grGradient);
            g.fillPath(grPath);
            
            // Dibujar la línea verde más brillante
            juce::Path grLine;
            grLine.startNewSubPath(grPoints[0]);
            for (size_t i = 1; i < grPoints.size(); ++i)
            {
                grLine.lineTo(grPoints[i]);
            }
            g.setColour(deltaGreen.withAlpha(0.95f * currentFadeOutFactor));
            g.strokePath(grLine, juce::PathStrokeType(2.5f)); // Línea más gruesa
        }
        else
        {
            // Modo normal: usar colores morados originales
            juce::ColourGradient grGradient(
                DarkTheme::accentSecondary.withAlpha(0.5f * currentFadeOutFactor), bounds.getX(), bounds.getY(),
                DarkTheme::accentSecondary.darker(0.3f).withAlpha(0.3f * currentFadeOutFactor), bounds.getRight(), bounds.getY(),
                false
            );
            
            // Dibujar área bajo la curva - empezar sin offset
            grPath.startNewSubPath(grPoints[0].x, bounds.getY());
            grPath.lineTo(grPoints[0]);
            
            // Suavizado con spline
            for (size_t i = 1; i < grPoints.size(); ++i)
            {
                if (i < grPoints.size() - 1)
                {
                    auto p0 = (i > 1) ? grPoints[i - 2] : grPoints[i - 1];
                    auto p1 = grPoints[i - 1];
                    auto p2 = grPoints[i];
                    auto p3 = (i < grPoints.size() - 1) ? grPoints[i + 1] : grPoints[i];
                    
                    for (float t = 0.0f; t < 1.0f; t += 0.2f)
                    {
                        float t2 = t * t;
                        float t3 = t2 * t;
                        
                        float x = 0.5f * ((2.0f * p1.x) +
                                         (-p0.x + p2.x) * t +
                                         (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
                                         (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3);
                        
                        float y = 0.5f * ((2.0f * p1.y) +
                                         (-p0.y + p2.y) * t +
                                         (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
                                         (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3);
                        
                        grPath.lineTo(x, y);
                    }
                }
                else
                {
                    grPath.lineTo(grPoints[i]);
                }
            }
            
            grPath.lineTo(grPoints.back().x, bounds.getY());
            grPath.closeSubPath();
            
            // Modo normal: No dibujar área rellena para gain reduction
            // g.setGradientFill(grGradient);
            // g.fillPath(grPath);
            
            // Solo dibujar la línea morada con currentFadeOutFactor
            juce::Path grLine;
            grLine.startNewSubPath(grPoints[0]);
            for (size_t i = 1; i < grPoints.size(); ++i)
            {
                grLine.lineTo(grPoints[i]);
            }
            g.setColour(DarkTheme::accentSecondary.withAlpha(0.9f * currentFadeOutFactor));
            g.strokePath(grLine, juce::PathStrokeType(2.0f)); // Línea un poco más gruesa para mejor visibilidad
        }
    }
    
    // Dibujar línea central de referencia (0 dB - sin ganancia ni atenuación)
    float centerY = bounds.getY() + bounds.getHeight() * 0.5f;
    if (deltaMode) {
        // NUEVO: En modo DELTA hacer la línea de referencia más visible
        g.setColour(DarkTheme::textPrimary.withAlpha(0.6f * currentFadeOutFactor));
        g.drawHorizontalLine(static_cast<int>(centerY), bounds.getX(), bounds.getRight());
        // Añadir línea punteada para mejor distinción
        juce::Path dashPath;
        for (float x = bounds.getX(); x < bounds.getRight(); x += 6.0f) {
            dashPath.addRectangle(x, centerY - 0.5f, 3.0f, 1.0f);
        }
        g.setColour(DarkTheme::accent.withAlpha(0.4f * currentFadeOutFactor));
        g.fillPath(dashPath);
    } else {
        // Modo normal: línea sutil original
        g.setColour(DarkTheme::textSecondary.withAlpha(0.3f * currentFadeOutFactor));
        g.drawHorizontalLine(static_cast<int>(centerY), bounds.getX(), bounds.getRight());
    }
}

void TransferFunctionDisplay::drawDeltaGainReduction(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // NUEVO: Visualización específica para modo DELTA - Simple y efectiva
    
    // Obtener valor actual de gain reduction
    float grValue = currentGainReduction.load(std::memory_order_relaxed);
    const float currentFadeOutFactor = fadeOutFactor.load(std::memory_order_relaxed);
    
    // Color verde para DELTA
    auto deltaGreen = juce::Colour(0x00, 0xC8, 0x96);  // Verde turquesa
    
    // Línea de referencia en la parte superior (0 dB - sin procesamiento)
    float topOffset = 0.0f; // Posición máxima - sin offset
    float referenceY = bounds.getY() + topOffset;
    
    // Dibujar línea de referencia mejorada en la parte superior
    g.setColour(DarkTheme::textPrimary.withAlpha(0.6f * currentFadeOutFactor));
    g.drawHorizontalLine(static_cast<int>(referenceY), bounds.getX(), bounds.getRight());
    
    // Añadir línea punteada para mejor distinción
    juce::Path dashPath;
    for (float x = bounds.getX(); x < bounds.getRight(); x += 6.0f) {
        dashPath.addRectangle(x, referenceY - 0.5f, 3.0f, 1.0f);
    }
    g.setColour(DarkTheme::accent.withAlpha(0.4f * currentFadeOutFactor));
    g.fillPath(dashPath);
    
    // SINCRONIZADO con grMeter optimizado - usar misma lógica ultra-sensible
    // Rango dinámico, mapeo logarítmico y zoom sincronizados para consistencia visual total
    
    // Rango de trabajo ultra-sensible sincronizado con grMeter optimizado
    const float minReduction = 0.0f;    // Sin reducción (sin área)
    const float maxReduction = (currentZoom == ZoomLevel::Zoomed) ? -20.0f : -40.0f;  // Zoom x2: -20dB, Normal: -40dB
    
    // Mapeo logarítmico para realzar reducciones pequeñas: valores más negativos = más área verde
    float fillRatio = juce::jmap(grValue, minReduction, maxReduction, 0.0f, 1.0f);
    fillRatio = juce::jlimit(0.0f, 1.0f, fillRatio);
    
    // Aplicar mapeo logarítmico para amplificar visualmente las reducciones pequeñas (sincronizado con grMeter)
    fillRatio = std::pow(fillRatio, 0.6f);  // Comprime menos las reducciones pequeñas (más visibles)
    
    // Altura disponible para la visualización (desde la línea de referencia hacia abajo)
    float availableHeight = bounds.getHeight() - topOffset - 2.0f; // Margen inferior reducido para mayor altura
    
    // Calcular altura de área (0 dB = sin área, -40/-20 dB = área completa según zoom) - SINCRONIZADO CON grMeter
    float barHeight = availableHeight * fillRatio;
    
    // Siempre partir desde la línea de referencia hacia abajo
    float barY = referenceY;
    
    // Solo dibujar si hay altura significativa (similar al grMeter que chequea > 1.0f)
    if (barHeight > 1.0f) {
        // Crear área rellena que crece desde arriba hacia abajo
        juce::Rectangle<float> grRect(bounds.getX(), barY, bounds.getWidth(), barHeight);
        
        // Gradiente vertical más sutil y elegante
        juce::ColourGradient grGradient(
            deltaGreen.withAlpha(0.4f * currentFadeOutFactor), bounds.getCentreX(), grRect.getY(),
            deltaGreen.darker(0.3f).withAlpha(0.25f * currentFadeOutFactor), bounds.getCentreX(), grRect.getBottom(),
            false
        );
        
        g.setGradientFill(grGradient);
        g.fillRect(grRect);
        
        // Línea inferior sutil y difuminada para definir el nivel actual de GR
        g.setColour(deltaGreen.withAlpha(0.3f * currentFadeOutFactor));
        g.drawHorizontalLine(static_cast<int>(barY + barHeight), bounds.getX(), bounds.getRight());
    }
}

//==============================================================================
// MÉTODOS DE HISTOGRAMA DELTA ESPECÍFICO
//==============================================================================

void TransferFunctionDisplay::updateDeltaHistory(float grDb) noexcept
{
    // Actualizar buffer circular con nuevo valor (Thread-Safe)
    int currentIndex = deltaHistoryIndex.load(std::memory_order_relaxed);
    deltaGrHistory[currentIndex].store(grDb, std::memory_order_relaxed);
    
    // Avanzar índice circular
    int nextIndex = (currentIndex + 1) % deltaHistorySize;
    deltaHistoryIndex.store(nextIndex, std::memory_order_relaxed);
}

void TransferFunctionDisplay::drawDeltaGainReductionHistory(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const float currentFadeOutFactor = fadeOutFactor.load(std::memory_order_relaxed);
    const int readIndex = deltaHistoryIndex.load(std::memory_order_relaxed);
    
    // Color verde para DELTA
    auto deltaGreen = juce::Colour(0x00, 0xC8, 0x96);  // Verde turquesa
    
    // Crear path para histograma
    juce::Path grPath;
    std::vector<juce::Point<float>> grPoints;
    
    // Recolectar puntos del buffer DELTA
    for (int i = 0; i < deltaHistorySize; ++i)
    {
        int samplesBack = deltaHistorySize - i;
        int bufferIndex = (readIndex - samplesBack + deltaHistorySize) % deltaHistorySize;
        
        float grDb = deltaGrHistory[bufferIndex].load(std::memory_order_relaxed);
        
        // Posición X - distribuir a lo ancho
        float normalizedTime = float(i) / float(deltaHistorySize - 1);
        float x = bounds.getX() + normalizedTime * bounds.getWidth();
        
        // Posición Y - mapear gain reduction usando rango ultra-sensible sincronizado con grMeter
        const float minReduction = 0.0f;    // Sin reducción (parte superior)
        const float maxReduction = (currentZoom == ZoomLevel::Zoomed) ? -20.0f : -40.0f;  // Sincronizado con grMeter
        
        float fillRatio = juce::jmap(grDb, minReduction, maxReduction, 0.0f, 1.0f);
        fillRatio = juce::jlimit(0.0f, 1.0f, fillRatio);
        
        // Aplicar mapeo logarítmico para consistencia con grMeter y área estática
        fillRatio = std::pow(fillRatio, 0.6f);
        
        // Y desde arriba hacia abajo (como área estática)
        float y = bounds.getY() + fillRatio * bounds.getHeight();
        
        grPoints.push_back({x, y});
    }
    
    // Crear área bajo la curva con spline suavizado
    if (!grPoints.empty())
    {
        // Gradiente vertical elegante
        juce::ColourGradient grGradient(
            deltaGreen.withAlpha(0.4f * currentFadeOutFactor), bounds.getCentreX(), bounds.getY(),
            deltaGreen.darker(0.3f).withAlpha(0.25f * currentFadeOutFactor), bounds.getCentreX(), bounds.getBottom(),
            false
        );
        
        // Crear área desde el top hacia la curva
        grPath.startNewSubPath(grPoints[0].x, bounds.getY());
        grPath.lineTo(grPoints[0]);
        
        // Spline suavizado simple (interpolación lineal suficiente para tiempo real)
        for (size_t i = 1; i < grPoints.size(); ++i)
        {
            grPath.lineTo(grPoints[i]);
        }
        
        // Cerrar área
        grPath.lineTo(grPoints.back().x, bounds.getY());
        grPath.closeSubPath();
        
        // Dibujar área rellena
        g.setGradientFill(grGradient);
        g.fillPath(grPath);
        
        // Línea superior para definir la curva
        juce::Path curvePath;
        curvePath.startNewSubPath(grPoints[0]);
        for (size_t i = 1; i < grPoints.size(); ++i)
        {
            curvePath.lineTo(grPoints[i]);
        }
        
        g.setColour(deltaGreen.withAlpha(0.8f * currentFadeOutFactor));
        g.strokePath(curvePath, juce::PathStrokeType(1.0f));
    }
}