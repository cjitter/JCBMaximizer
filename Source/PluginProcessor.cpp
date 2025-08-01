//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBMaximizer and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
// Archivos del proyecto
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Helpers/UTF8Helper.h"

//==============================================================================
// CONSTRUCTOR Y DESTRUCTOR
//==============================================================================
JCBMaximizerAudioProcessor::JCBMaximizerAudioProcessor()
    : juce::AudioProcessor(createBusesProperties()),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout()),  // nullptr = no automatic undo (proven solution tested on jr-granular)
      m_CurrentBufferSize(0),
      editorSize(1260, 360),
      lastPreset(0),
      currentProgram(-1)
{
    // Configurar límites del guiUndoManager para optimizar rendimiento
    guiUndoManager.setMaxNumberOfStoredUnits(0, 20); // Solo 20 transacciones exactas (ahorro de memoria)
    
    // Inicializar Gen~ state
    m_PluginState = (CommonState *)JCBMaximizer::create(44100, 64);
    JCBMaximizer::reset(m_PluginState);

    // Inicializar buffers de Gen~
    m_InputBuffers = new t_sample *[JCBMaximizer::num_inputs()];
    m_OutputBuffers = new t_sample *[JCBMaximizer::num_outputs()];
    
    for (int i = 0; i < JCBMaximizer::num_inputs(); i++) {
        m_InputBuffers[i] = nullptr;
    }
    for (int i = 0; i < JCBMaximizer::num_outputs(); i++) {
        m_OutputBuffers[i] = nullptr;
    }

    // Vincular listeners de parámetros de gen~ a APVTS
    for (int i = 0; i < JCBMaximizer::num_params(); i++)
    {
        auto name = juce::String(JCBMaximizer::getparametername(m_PluginState, i));
        apvts.addParameterListener(name, this);
    }
    
    // IMPORTANTE: Sincronizar valores iniciales con Gen~
    // Esto asegura que Gen~ tenga los valores correctos desde el principio
    for (int i = 0; i < JCBMaximizer::num_params(); i++)
    {
        auto paramName = juce::String(JCBMaximizer::getparametername(m_PluginState, i));
        if (auto* param = apvts.getRawParameterValue(paramName)) {
            float value = param->load();
            
            // Aplicar misma validación que en parameterChanged() (MAXIMIZER)
            if (paramName == "d_ATK" && value < 0.01f) {
                value = 0.01f;  // Maximizer permite hasta 0.01ms
            }
            if (paramName == "e_REL" && value < 1.0f) {
                value = 1.0f;   // Maximizer: mínimo 1ms
            }
            
            JCBMaximizer::setparameter(m_PluginState, i, value, nullptr);
        }
    }

    // Añadir parámetro AAX directamente al juce::AudioProcessor (no al APVTS)
    #if JucePlugin_Build_AAX
    auto grMeter = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("aax_gr_meter", 1),  // Version hint fijo para este parámetro
        "Gain Reduction",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.0f,  // Default: 0 = sin reducción
        "",
        juce::AudioProcessorParameter::compressorLimiterGainReductionMeter);
    
    // Guardar referencia antes de moverlo
    aaxGainReductionParam = grMeter.get();
    
    // Añadir directamente al processor, NO al APVTS
    // El flag compressorLimiterGainReductionMeter ya indica que es un parámetro especial
    addParameter(grMeter.release());

    // Iniciar timer para actualizar parámetros AAX
    startTimerHz(30); // 15 Hz es suficiente para el medidor y más seguro
    #endif
}

JCBMaximizerAudioProcessor::~JCBMaximizerAudioProcessor()
{
    // CRÍTICO: Primero indicar que estamos destruyendo para evitar race conditions
    isBeingDestroyed = true;
    
    // Detener timer AAX inmediatamente (antes que cualquier otra cosa)
    #if JucePlugin_Build_AAX
    stopTimer();
    // Pequeña espera para asegurar que el timer callback no esté ejecutándose
    juce::Thread::sleep(10);
    #endif
    
    // Destruir listeners de parámetros del apvts
    for (int i = 0; i < JCBMaximizer::num_params(); i++)
    {
        auto name = juce::String(JCBMaximizer::getparametername(m_PluginState, i));
        apvts.removeParameterListener(name, this);
    }
    
    // Limpiar buffers con protección nullptr
    if (m_InputBuffers != nullptr) {
        for (int i = 0; i < JCBMaximizer::num_inputs(); i++) {
            if (m_InputBuffers[i] != nullptr) {
                delete[] m_InputBuffers[i];
                m_InputBuffers[i] = nullptr;
            }
        }
        delete m_InputBuffers;
        m_InputBuffers = nullptr;
    }
    
    if (m_OutputBuffers != nullptr) {
        for (int i = 0; i < JCBMaximizer::num_outputs(); i++) {
            if (m_OutputBuffers[i] != nullptr) {
                delete[] m_OutputBuffers[i];
                m_OutputBuffers[i] = nullptr;
            }
        }
        delete m_OutputBuffers;
        m_OutputBuffers = nullptr;
    }
    
    // Destruir Gen~ state al final
    if (m_PluginState != nullptr) {
        JCBMaximizer::destroy(m_PluginState);
        m_PluginState = nullptr;
    }
}

//==============================================================================
// MÉTODOS PRINCIPALES DEL AUDIOPROCESSOR
//==============================================================================
void JCBMaximizerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Configuración de canales en modo debug eliminada para release
    
    // Inicializar sample rate y vector size
    m_PluginState->sr = sampleRate;
    m_PluginState->vs = samplesPerBlock;
    
    // Pre-asignar buffers con tamaño máximo esperado para evitar allocations en audio thread
    // Usar un tamaño seguro que cubra la mayoría de casos (4096 samples es común máximo)
    const long maxExpectedBufferSize = juce::jmax(static_cast<long>(samplesPerBlock), 4096L);
    assureBufferSize(maxExpectedBufferSize);
    
    // Pre-asignar vectors de waveform data para evitar resize en audio thread
    {
        std::lock_guard<std::mutex> lock(waveformMutex);
        const size_t maxWaveformSize = static_cast<size_t>(maxExpectedBufferSize);
        currentInputSamples.resize(maxWaveformSize);
        currentProcessedSamples.resize(maxWaveformSize);
        currentGainReductionSamples.resize(maxWaveformSize);
    }
    
    // Inicializar latencia basada en el tiempo de lookahead
    auto latenciaMs = apvts.getRawParameterValue("n_LOOKAHEAD")->load();
    
    // Cálculo estándar: ms * (sampleRate / 1000)
    int latenciaSamples = static_cast<int>(latenciaMs * sampleRate / 1000.0);
    latenciaSamples = juce::jmax(0, latenciaSamples);

    // Aplicar Lookahead teniendo en cuenta el sample generado en gen~
    setLatencySamples(latenciaSamples+1);

    // Initialize atomic meter values
    // CRITICAL: Changed from SmoothedValue to atomic - no reset() needed
    leftInputRMS.store(-100.0f, std::memory_order_relaxed);
    rightInputRMS.store(-100.0f, std::memory_order_relaxed);
    
    leftOutputRMS.store(-100.0f, std::memory_order_relaxed);
    rightOutputRMS.store(-100.0f, std::memory_order_relaxed);
    
    gainReduction.store(0.0f, std::memory_order_relaxed);
    
    // NUEVO: Resetear el promedio móvil cuando se reinicia la reproducción
    // Esto asegura que el slider comience desde un estado limpio
    grMovingAverage.reset();
    
    // MAXIMIZER: No sidechain - removed leftSC/rightSC initialization
    // leftSC.store(-100.0f, std::memory_order_relaxed);
    // rightSC.store(-100.0f, std::memory_order_relaxed);
    
    // Configurar buffers auxiliares
    grBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    grBuffer.clear();
    
    trimInputBuffer.setSize(2, samplesPerBlock);
    trimInputBuffer.clear();
    
    // MAXIMIZER: No sidechain - removed sidechainBuffer initialization
    // sidechainBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    // sidechainBuffer.clear();
    
    // Inicializar buffers de forma de onda
    currentInputSamples.resize(samplesPerBlock, 0.0f);
    currentProcessedSamples.resize(samplesPerBlock, 0.0f);
    
    // IMPORTANTE: Re-sincronizar todos los parámetros con Gen~ en prepareToPlay
    // Esto asegura que los valores estén correctos cuando el DAW comienza a reproducir
    for (int i = 0; i < JCBMaximizer::num_params(); i++)
    {
        auto paramName = juce::String(JCBMaximizer::getparametername(m_PluginState, i));
        if (auto* param = apvts.getRawParameterValue(paramName)) {
            float value = param->load();
            JCBMaximizer::setparameter(m_PluginState, i, value, nullptr);
        }
    }
}

void JCBMaximizerAudioProcessor::releaseResources()
{
    // Limpiar buffers auxiliares
    grBuffer.setSize(0, 0);
    trimInputBuffer.setSize(0, 0);
    // MAXIMIZER: No sidechain - removed sidechainBuffer cleanup
    // sidechainBuffer.setSize(0, 0);
}

//==============================================================================
// PROCESAMIENTO DE AUDIO
//==============================================================================
void JCBMaximizerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    const int numSamples = buffer.getNumSamples();
    assureBufferSize(numSamples);

    // Procesar audio usando métodos separados
    fillGenInputBuffers(buffer);
    processGenAudio(numSamples);
    fillOutputBuffers(buffer);
    
    // Capturar DESPUÉS del procesamiento para usar salidas de Gen~
    // Capturar entrada post-TRIM desde salidas 3 y 4 de Gen~
    captureInputWaveformData(buffer, numSamples);
    // Capturar salida procesada para visualización
    captureOutputWaveformData(numSamples);
    
    // Actualizar clip detection (debe ser DESPUÉS del procesamiento completo)
    updateClipDetection(buffer, buffer);  // input y output son el mismo buffer al final
    
    // Actualizar medidores
    updateInputMeters(buffer);
    updateOutputMeters(buffer);
    updateGainReductionMeter();
    // MAXIMIZER: No sidechain - removed updateSidechainMeters call
    // updateSidechainMeters(buffer);
}

//==============================================================================
// INTEGRACIÓN GEN~
//==============================================================================

/**
 * Asegurar que los buffers de Gen~ tengan el tamaño correcto
 * CRÍTICO: Esta función gestiona la memoria dinámica para la comunicación con el motor DSP Gen~
 * AUDIO-THREAD SAFE: Solo redimensiona si es absolutamente necesario para evitar RT violations
 */
void JCBMaximizerAudioProcessor::assureBufferSize(long bufferSize)
{
    if (bufferSize > m_CurrentBufferSize) {
        // NOTA: RT-safe porque prepareToPlay() pre-asigna hasta 4096 samples
        // Solo se ejecuta este bloque si el DAW solicita > 4096 samples (muy raro)
        
        // Redimensionar buffers de entrada
        for (int i = 0; i < JCBMaximizer::num_inputs(); i++) {
            delete[] m_InputBuffers[i];
            m_InputBuffers[i] = new t_sample[bufferSize];
        }
        
        // Redimensionar buffers de salida
        for (int i = 0; i < JCBMaximizer::num_outputs(); i++) {
            delete[] m_OutputBuffers[i];
            m_OutputBuffers[i] = new t_sample[bufferSize];
        }
        
        m_CurrentBufferSize = bufferSize;
    }
}

void JCBMaximizerAudioProcessor::fillGenInputBuffers(const juce::AudioBuffer<float>& buffer)
{
    const auto mainInputChannels = getMainBusNumInputChannels();
    const int numSamples = buffer.getNumSamples();
    
    // MAXIMIZER: Only 2 inputs (main L/R) - no sidechain inputs
    if (mainInputChannels > 1) {
        // Modo estéreo - fill main L/R inputs (inputs 0 and 1)
        for (int j = 0; j < numSamples; j++) {
            m_InputBuffers[0][j] = buffer.getReadPointer(0)[j];  // L
            m_InputBuffers[1][j] = buffer.getReadPointer(1)[j];  // R
        }
    } else {
        // Modo mono - duplicar señal mono a ambos canales L/R para procesamiento stereo-linked
        if (mainInputChannels == 1) {
            for (int j = 0; j < numSamples; j++) {
                m_InputBuffers[0][j] = buffer.getReadPointer(0)[j];  // L = mono
                m_InputBuffers[1][j] = buffer.getReadPointer(0)[j];  // R = mono (duplicado)
            }
        }
    }
}

void JCBMaximizerAudioProcessor::processGenAudio(int numSamples)
{
    JCBMaximizer::perform(m_PluginState,
                          m_InputBuffers,
                          JCBMaximizer::num_inputs(),
                          m_OutputBuffers,
                          JCBMaximizer::num_outputs(),
                          numSamples);
}

void JCBMaximizerAudioProcessor::fillOutputBuffers(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const auto mainOutputChannels = getMainBusNumOutputChannels();
    
    // Llenar buffers de salida principales - conversión double a float
    for (int i = 0; i < mainOutputChannels; i++) {
        float* destPtr = buffer.getWritePointer(i);
        const t_sample* srcPtr = m_OutputBuffers[i];
        for (int j = 0; j < numSamples; j++) {
            destPtr[j] = static_cast<float>(srcPtr[j]);
        }
    }
    
    // Preparar buffer para medidor de entrada (después de trim) - conversión double a float
    float* trimL = trimInputBuffer.getWritePointer(0);
    const t_sample* srcL = m_OutputBuffers[3];
    for (int j = 0; j < numSamples; j++) {
        trimL[j] = static_cast<float>(srcL[j]);
    }
    if (trimInputBuffer.getNumChannels() > 1) {
        float* trimR = trimInputBuffer.getWritePointer(1);
        const t_sample* srcR = m_OutputBuffers[4];
        for (int j = 0; j < numSamples; j++) {
            trimR[j] = static_cast<float>(srcR[j]);
        }
    }
}

//==============================================================================
// MEDIDORES DE AUDIO
//==============================================================================
void JCBMaximizerAudioProcessor::updateInputMeters(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const auto mainInputChannels = getMainBusNumInputChannels();

    // Calcular valores RMS
    const auto rmsValueL = juce::Decibels::gainToDecibels(trimInputBuffer.getRMSLevel(0, 0, numSamples));
    const auto rmsValueR = (trimInputBuffer.getNumChannels() > 1) ?
    juce::Decibels::gainToDecibels(trimInputBuffer.getRMSLevel(1, 0, numSamples)) : rmsValueL;
    
    // Calcular valores de pico reales
    const auto peakValueL = juce::Decibels::gainToDecibels(trimInputBuffer.getMagnitude(0, 0, numSamples));
    const auto peakValueR = (trimInputBuffer.getNumChannels() > 1) ?
    juce::Decibels::gainToDecibels(trimInputBuffer.getMagnitude(1, 0, numSamples)) : peakValueL;
    
    // Usar combinación ponderada 70% peak + 30% RMS
    const auto displayValueL = (peakValueL * 0.7f) + (rmsValueL * 0.3f);
    const auto displayValueR = (peakValueR * 0.7f) + (rmsValueR * 0.3f);
    
    // Si estamos cerca del clipping (> -3dB), mostrar el valor de pico puro
    const auto inputValueL = (peakValueL > -3.0f) ? peakValueL : displayValueL;
    const auto inputValueR = (peakValueR > -3.0f) ? peakValueR : displayValueR;
    
    // CRÍTICO: Usar atomic store para thread safety
    // No smoothing is done here - just direct atomic updates
    if (mainInputChannels < 2) {
        // Modo mono - ambos medidores muestran el mismo valor
        leftInputRMS.store(inputValueL, std::memory_order_relaxed);
        rightInputRMS.store(inputValueL, std::memory_order_relaxed);
    } else {
        // Modo estéreo - medidores independientes
        leftInputRMS.store(inputValueL, std::memory_order_relaxed);
        rightInputRMS.store(inputValueR, std::memory_order_relaxed);
    }
}

void JCBMaximizerAudioProcessor::updateOutputMeters(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const auto mainOutputChannels = getMainBusNumOutputChannels();

    // Calcular valores RMS para el medidor (promedio de potencia)
    const auto rmsValueL = juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, numSamples));
    const auto rmsValueR = (mainOutputChannels > 1) ?
    juce::Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, numSamples)) : rmsValueL;
    
    // Calcular valores de pico reales (máximo absoluto en el buffer)
    const auto peakValueL = juce::Decibels::gainToDecibels(buffer.getMagnitude(0, 0, numSamples));
    const auto peakValueR = (mainOutputChannels > 1) ?
    juce::Decibels::gainToDecibels(buffer.getMagnitude(1, 0, numSamples)) : peakValueL;
    
    // Usar una combinación ponderada de RMS y Peak para mejor visualización
    // 70% peak + 30% RMS da una representación más precisa similar a medidores profesionales
    const auto displayValueL = (peakValueL * 0.7f) + (rmsValueL * 0.3f);
    const auto displayValueR = (peakValueR * 0.7f) + (rmsValueR * 0.3f);
    
    // Si estamos cerca del clipping (> -3dB), mostrar el valor de pico puro
    const auto finalValueL = (peakValueL > -3.0f) ? peakValueL : displayValueL;
    const auto finalValueR = (peakValueR > -3.0f) ? peakValueR : displayValueR;
    
    // CRÍTICO: Usar atomic store para thread safety
    if (mainOutputChannels > 1) {
        // Modo estéreo
        leftOutputRMS.store(finalValueL, std::memory_order_relaxed);
        rightOutputRMS.store(finalValueR, std::memory_order_relaxed);
    } else {
        // Modo mono
        leftOutputRMS.store(finalValueL, std::memory_order_relaxed);
        rightOutputRMS.store(finalValueL, std::memory_order_relaxed);
    }
}

void JCBMaximizerAudioProcessor::updateGainReductionMeter()
{
    const int numSamples = grBuffer.getNumSamples();
    t_sample* grData = m_OutputBuffers[2]; // out3 = gain reduction (valores lineales)
    
    // NUEVO: Replicar exactamente Max: outlet 3 → average~ 4800 1 → linear2db → slider
    float finalAveragedLinear = 1.0f; // Inicializar a 1.0 (sin reducción)
    
    // Verificar si hay muestras para procesar
    if (numSamples > 0 && grData != nullptr) {
        // Procesar cada muestra con promedio móvil de 4800 muestras (modo absoluto)
        for (int i = 0; i < numSamples; ++i) {
            float grLinear = static_cast<float>(grData[i]);
            // Aplicar promedio móvil con modo absoluto (average~ 4800 1)
            finalAveragedLinear = grMovingAverage.processSample(grLinear);
        }
    } else {
        // Sin muestras válidas - mantener estado actual del promedio móvil
        finalAveragedLinear = grMovingAverage.getCurrentAverage();
    }
    
    // CORREGIR: Gen~ out3 da gain multipliers donde:
    // 1.0 = sin reducción (0 dB en el slider)  
    // < 1.0 = reducción (valores negativos en el slider)
    
    // Convertir de gain multiplier a dB de reducción
    float valueGR_dB;
    
    // CORREGIDO FINAL: Lógica correcta para medidor de gain reduction
    // Sin reducción = 0 dB (sin barra blanca), Con reducción = valores negativos (barra blanca crece)
    if (finalAveragedLinear >= 0.999f) {
        // Sin reducción significativa - mostrar 0 dB (sin barra blanca visible)
        valueGR_dB = 0.0f;
    } else {
        // Hay reducción - convertir a dB negativos donde valores más negativos = más barra
        constexpr float minLinearValue = 0.000001f; // Evitar -inf
        finalAveragedLinear = std::max(finalAveragedLinear, minLinearValue);
        valueGR_dB = juce::Decibels::gainToDecibels(finalAveragedLinear);
        // Clampear al rango del slider
        valueGR_dB = juce::jlimit(-100.0f, 0.0f, valueGR_dB);
    }
    
    // CRÍTICO: Usar atomic store para thread safety
    gainReduction.store(valueGR_dB, std::memory_order_relaxed);
    
    // Almacenar el valor para AAX (Pro Tools lo leerá cuando lo necesite)
    #if JucePlugin_Build_AAX
    // Gen~ out3 ya devuelve valores lineales donde:
    // 1.0 = sin reducción
    // < 1.0 = hay reducción
    // Usar el valor promediado final en lugar del anterior minGainMultiplier
    currentGainReductionLinear.store(finalAveragedLinear);
    #endif
}

// MAXIMIZER: No sidechain - removed entire updateSidechainMeters function
/*
void JCBMaximizerAudioProcessor::updateSidechainMeters(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();

    // Resetear flags de clipping sidechain para este buffer
    bool scClip[2] = {false, false};
    
    // MAXIMIZER: No tiene sidechain externo, siempre usar silencio en medidores SC
    const bool extKeyActive = false;  // Maximizer no tiene external key
    
    // Si EXT KEY no está activo, mostrar silencio en los medidores
    if (!extKeyActive) {
        const auto valueSC = -100.0f;
        
        // MAXIMIZER: No sidechain - leftSC/rightSC variables eliminadas según CONTEXTO.txt
        // leftSC.store(valueSC, std::memory_order_relaxed);
        // rightSC.store(valueSC, std::memory_order_relaxed);
        
        return;  // No procesar más si EXT KEY está desactivado
    }
    
    // CAMBIO: Usar las salidas 5 y 6 de Gen~ (índices 5 y 6 en m_OutputBuffers)
    // Estas salidas ya incluyen el procesamiento de SC TRIM aplicado por Gen~
    if (JCBMaximizer::num_outputs() > 6) {
        // Calcular RMS y pico del sidechain desde las salidas de Gen~
        float maxSCL = 0.0f, maxSCR = 0.0f;
        float rmsSumL = 0.0f, rmsSumR = 0.0f;
        
        for (int i = 0; i < numSamples; ++i) {
            // Usar salidas 5 y 6 de Gen~ que ya tienen SC TRIM aplicado
            float sampleL = static_cast<float>(m_OutputBuffers[5][i]);
            float sampleR = static_cast<float>(m_OutputBuffers[6][i]);
            
            // Peak detection
            maxSCL = juce::jmax(maxSCL, std::abs(sampleL));
            maxSCR = juce::jmax(maxSCR, std::abs(sampleR));
            
            // RMS accumulation
            rmsSumL += sampleL * sampleL;
            rmsSumR += sampleR * sampleR;
        }
        
        // Calcular RMS
        float rmsL = std::sqrt(rmsSumL / static_cast<float>(numSamples));
        float rmsR = std::sqrt(rmsSumR / static_cast<float>(numSamples));
        
        const auto rmsValueSCLeft = juce::Decibels::gainToDecibels(rmsL);
        const auto rmsValueSCRight = juce::Decibels::gainToDecibels(rmsR);
        const auto peakValueSCLeft = juce::Decibels::gainToDecibels(maxSCL);
        const auto peakValueSCRight = juce::Decibels::gainToDecibels(maxSCR);
        
        // Detectar clipping basado en el valor de pico
        if (maxSCL >= 0.999f) {  // Usar el mismo umbral que los medidores principales
            scClip[0] = true;
        }
        if (maxSCR >= 0.999f) {
            scClip[1] = true;
        }
        
        // Usar combinación ponderada como en los medidores principales
        const auto displayValueSCLeft = (peakValueSCLeft * 0.7f) + (rmsValueSCLeft * 0.3f);
        const auto displayValueSCRight = (peakValueSCRight * 0.7f) + (rmsValueSCRight * 0.3f);
        
        const auto valueSCLeft = (peakValueSCLeft > -3.0f) ? peakValueSCLeft : displayValueSCLeft;
        const auto valueSCRight = (peakValueSCRight > -3.0f) ? peakValueSCRight : displayValueSCRight;
        
        // MAXIMIZER: No sidechain - leftSC/rightSC variables eliminadas según CONTEXTO.txt
        // if (!isProTools()) {
        //     leftSC.store(valueSCLeft, std::memory_order_relaxed);
        //     rightSC.store(valueSCRight, std::memory_order_relaxed);
        // } else {
        //     // ProTools: usar solo canal izquierdo para ambos medidores
        //     leftSC.store(valueSCLeft, std::memory_order_relaxed);
        //     leftSC.store(valueSCLeft, std::memory_order_relaxed);
        // }
    } else {
        // Sidechain no disponible - mostrar silencio
        const auto valueSC = -100.0f;
        
        // MAXIMIZER: No sidechain - leftSC/rightSC variables eliminadas según CONTEXTO.txt
        // leftSC.store(valueSC, std::memory_order_relaxed);
        // rightSC.store(valueSC, std::memory_order_relaxed);
    }
    
    // Actualizar flags atómicos de clip
    for (int channel = 0; channel < 2; ++channel) {
        if (scClip[channel]) {
            sidechainClipDetected[channel] = true;
        }
    }
}
*/

//==============================================================================
// CONFIGURACIÓN DE BUSES Y PARÁMETROS
//==============================================================================
juce::AudioProcessor::BusesProperties JCBMaximizerAudioProcessor::createBusesProperties()
{
    // MAXIMIZER: Simple stereo I/O - no sidechain buses
    auto propBuses = juce::AudioProcessor::BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        .withInput("Input", juce::AudioChannelSet::stereo(), true);
    
    // MAXIMIZER: No sidechain - removed sidechain bus configuration
    /*
    juce::PluginHostType daw;
    
    if (daw.isProTools())
        propBuses.addBus(true, "Sidechain MONO", juce::AudioChannelSet::mono(), false);
    else
        propBuses.addBus(true, "Sidechain ST", juce::AudioChannelSet::stereo(), false);
    */
    
    return propBuses;
}

// Validación de configuraciones de canales - define qué layouts acepta el plugin
bool JCBMaximizerAudioProcessor::isBusesLayoutSupported(const juce::AudioProcessor::BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // Verificar bus principal de salida
    auto mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono()
        && mainOut != juce::AudioChannelSet::stereo())
        return false;
    
    // Verificar bus principal de entrada
    auto mainIn = layouts.getMainInputChannelSet();
    if (mainIn != juce::AudioChannelSet::mono()
        && mainIn != juce::AudioChannelSet::stereo())
        return false;
    
    // Pro Tools AAX: Rechazar específicamente stereo input → mono output
    // Solo permitir: 1→1, 2→2, 1→2. NO permitir: 2→1
#if JucePlugin_Build_AAX
    // En formato AAX, rechazar siempre stereo input → mono output
    if (mainIn == juce::AudioChannelSet::stereo() && mainOut == juce::AudioChannelSet::mono())
        return false;
#endif
    
    // MAXIMIZER: No sidechain - removed sidechain bus validation
    /*
    // Si hay bus de sidechain, verificar que sea válido
    if (layouts.inputBuses.size() > 1)
    {
        auto sidechainBus = layouts.inputBuses[1];
        
        // El sidechain puede estar desactivado (empty)
        if (!sidechainBus.isDisabled())
        {
            // Solo aceptar sidechain mono o estéreo si está activo
            if (sidechainBus != juce::AudioChannelSet::mono() 
                && sidechainBus != juce::AudioChannelSet::stereo())
                return false;
        }
            
        // No aceptar más de 2 buses de entrada
        if (layouts.inputBuses.size() > 2)
            return false;
    }
    */
    
    return true;
#endif
}

/**
 * Crear el layout de parámetros del plugin
 * CRÍTICO: Define todos los parámetros del compresor en orden alfabético
 * Incluye configuración de rangos, valores por defecto y metadata para cada parámetro
 * Version hint 21 fuerza re-escaneo en hosts para parámetros renombrados
 */
juce::AudioProcessorValueTreeState::ParameterLayout JCBMaximizerAudioProcessor::createParameterLayout()
{
   const int versionHint = 21;
   std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;

   // a_THD @min -20 @max 0 @default 0 (Maximizer: rango más estrecho)
   auto thd = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("a_THD", versionHint),
                                                          juce::CharPointer_UTF8("Threshold"),
                                                          juce::NormalisableRange<float>(-20.f, 0.f, 0.1f, 1.0f),
                                                          0.f);

   // b_CELLING @min -60 @max 0 @default 0 (NUEVO - específico del Maximizer)
   auto ceiling = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("b_CELLING", versionHint),
                                                              juce::CharPointer_UTF8("Ceiling"),
                                                              juce::NormalisableRange<float>(-60.f, 0.f, 0.1f, 1.0f),
                                                              0.f);

   // d_ATK @min 0.01 @max 500 @default 1 (mantener mapeo exponencial)
   auto atkRange = juce::NormalisableRange<float>(
       0.01f, 500.f,
       [](float start, float end, float normalised) {
           return start + (end - start) * std::pow(normalised, 1.8f);
       },
       [](float start, float end, float value) {
           float proportion = juce::jlimit(0.0f, 1.0f, (value - start) / (end - start));
           return std::pow(proportion, 1.0f / 1.8f);
       },
       [](float start, float end, float value) {
           if (value < 10.0f)
               return std::round(value * 100.0f) / 100.0f;
           else
               return std::round(value * 10.0f) / 10.0f;
       }
   );
   auto atk = std::make_unique<juce::AudioParameterFloat>(
       juce::ParameterID("d_ATK", versionHint),
       juce::CharPointer_UTF8("Attack"),
       atkRange,
       1.0f,
       juce::String(),
       juce::AudioParameterFloat::genericParameter,
       [](float value, int){
           if (value < 1.0f)
               return juce::String(value, 1) + " ms";
           else if (value < 100.0f)
               return juce::String(value, 1) + " ms";
           else
               return juce::String(value, 0) + " ms";
       },
       nullptr);

   // e_REL @min 1 @max 1500 @default 50 (cambiar rango y default para Maximizer)
   auto relRange = juce::NormalisableRange<float>(
       1.f, 1500.f,
       [](float start, float end, float normalised) {
           return start + (end - start) * std::pow(normalised, 1.4f);
       },
       [](float start, float end, float value) {
           float proportion = juce::jlimit(0.0f, 1.0f, (value - start) / (end - start));
           return std::pow(proportion, 1.0f / 1.4f);
       },
       [](float start, float end, float value) {
           if (value < 10.0f)
               return std::round(value * 100.0f) / 100.0f;
           else
               return std::round(value * 10.0f) / 10.0f;
       }
   );
   auto rel = std::make_unique<juce::AudioParameterFloat>(
       juce::ParameterID("e_REL", versionHint),
       juce::CharPointer_UTF8("Release"),
       relRange,
       50.0f,  // Cambiar default de 120 a 50
       juce::String(),
       juce::AudioParameterFloat::genericParameter,
       [](float value, int){
           if (value < 10.0f)
               return juce::String(value, 1) + " ms";
           else if (value < 100.0f)
               return juce::String(value, 1) + " ms";
           else
               return juce::String(value, 0) + " ms";
       },
       nullptr);

   // g_DITHER @min 0 @max 1 @default 0 (NUEVO - específico del Maximizer)
   auto dither = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("g_DITHER", versionHint),
                                                             juce::CharPointer_UTF8("Dither"),
                                                             juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                             0.f,
                                                             juce::String(),
                                                             juce::AudioParameterFloat::genericParameter,
                                                             [](float value, int){
                                                                 if (value <= 0.f)
                                                                     return juce::String("OFF");
                                                                 else
                                                                     return juce::String(static_cast<int>(value * 100)) + "%";
                                                             },
                                                             nullptr);

   // h_BYPASS @min 0 @max 1 @default 0 - NO AUTOMATABLE (renombrado de p_BYPASS)
   auto bypass = std::make_unique<juce::AudioParameterInt>(juce::ParameterID("h_BYPASS", versionHint),
                                                           juce::CharPointer_UTF8("BYPASS"),
                                                           0, 1, 0,
                                                           juce::AudioParameterIntAttributes().withAutomatable(false).withCategory(juce::AudioProcessorParameter::genericParameter));

   // j_TRIM @min -12 @max 12 @default 0 (renombrado de a_TRIM)
   auto trim = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("j_TRIM", versionHint),
                                                           juce::CharPointer_UTF8("Input Trim"),
                                                           juce::NormalisableRange<float>(-12.f, 12.f, 0.1f, 1.f),
                                                           0.f);

   // k_DELTA @min 0 @max 1 @default 0 - NO AUTOMATABLE (renombrado de v_DELTA)
   auto delta = std::make_unique<juce::AudioParameterInt>(
       juce::ParameterID("k_DELTA", versionHint),
       juce::CharPointer_UTF8("DELTA"),
       0, 1, 0,
       juce::AudioParameterIntAttributes().withAutomatable(false).withCategory(juce::AudioProcessorParameter::genericParameter));

   // l_DETECT @min 0 @max 1 @default 0 (NUEVO - Detection mode Peak/RMS)
   auto detect = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("l_DETECT", versionHint),
                                                             juce::CharPointer_UTF8("Detection"),
                                                             juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                             0.f,
                                                             juce::String(),
                                                             juce::AudioParameterFloat::genericParameter,
                                                             [](float value, int){
                                                                 if (value < 0.01f)
                                                                     return juce::String("PEAK");
                                                                 else if (value > 0.99f)
                                                                     return juce::String("RMS");
                                                                 else
                                                                     return juce::String(value, 2);
                                                             },
                                                             nullptr);

   // m_AUTOREL @min 0 @max 1 @default 0 (NUEVO - Auto-release)
   auto autorel = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("m_AUTOREL", versionHint),
                                                              juce::CharPointer_UTF8("Auto Release"),
                                                              juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                              0.f,
                                                              juce::String(),
                                                              juce::AudioParameterFloat::genericParameter,
                                                              [](float value, int){
                                                                  if (value <= 0.f)
                                                                      return juce::String("OFF");
                                                                  else
                                                                      return juce::String(static_cast<int>(value * 100)) + "%";
                                                              },
                                                              nullptr);

   // n_LOOKAHEAD @min 0 @max 5 @default 2 (cambiar rango y default para Maximizer)
   auto lookahead = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("n_LOOKAHEAD", versionHint),
                                                                juce::CharPointer_UTF8("Lookahead"),
                                                                juce::NormalisableRange<float>(0.f, 5.f, 0.1f, 1.f),
                                                                2.f);  // Default cambiado de 0 a 2

   // Añadir parámetros en orden alfabético (exactamente como Gen~)
   params.push_back(std::move(thd));            // a_THD
   params.push_back(std::move(ceiling));        // b_CELLING
   params.push_back(std::move(atk));            // d_ATK
   params.push_back(std::move(rel));            // e_REL
   params.push_back(std::move(dither));         // g_DITHER
   params.push_back(std::move(bypass));         // h_BYPASS
   params.push_back(std::move(trim));           // j_TRIM
   params.push_back(std::move(delta));          // k_DELTA
   params.push_back(std::move(detect));         // l_DETECT
   params.push_back(std::move(autorel));        // m_AUTOREL
   params.push_back(std::move(lookahead));      // n_LOOKAHEAD

   // NOTA: El parámetro aax_gr_meter se añade directamente en el constructor
   // para evitar que forme parte del APVTS y el sistema undo/redo

   return { params.begin(), params.end() };
}


//==============================================================================
// GESTIÓN DE PARÁMETROS
//==============================================================================
void JCBMaximizerAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    
    // Validar valores mínimos para ATK y REL (MAXIMIZER: rangos ajustados)
    if (parameterID == "d_ATK" && newValue < 0.01f) {
        newValue = 0.01f;  // Maximizer permite hasta 0.01ms
    }
    if (parameterID == "e_REL" && newValue < 1.0f) {
        newValue = 1.0f;   // Maximizer: mínimo 1ms (no 0.1ms)
    }
    
    // Buscar el índice correcto en Gen~ basado en el nombre del parámetro
    int genIndex = -1;
    for (int i = 0; i < JCBMaximizer::num_params(); i++) {
        if (parameterID == JCBMaximizer::getparametername(m_PluginState, i)) {
            genIndex = i;
            break;
        }
    }
    
    if (genIndex < 0) {
        return;  // Parámetro no encontrado en Gen~
    }
    
    JCBMaximizer::setparameter(m_PluginState, genIndex, newValue, nullptr);
    
    // Actualizar latencia cuando cambia el lookahead
    if (parameterID == "n_LOOKAHEAD")
    {
        double sampleRate = getSampleRate();
        
        // Solo actualizar si tenemos un sample rate válido
        if (sampleRate > 0)
        {
            // Cálculo estándar de latencia: ms * (sampleRate / 1000)
            int latenciaSamples = static_cast<int>(newValue * sampleRate / 1000.0);
            
            // Asegurar que nunca sea negativo
            latenciaSamples = juce::jmax(0, latenciaSamples);
            
            setLatencySamples(latenciaSamples+1);
            
            // Lookahead latency compensation updated
        }
    }
}

//==============================================================================
// Métodos de programa (presets)
int JCBMaximizerAudioProcessor::getNumPrograms()
{
    return 0; // 3 antes, añadir 1 por comportamiento extraño algunos hosts
}

int JCBMaximizerAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

void JCBMaximizerAudioProcessor::setCurrentProgram(int index)
{
    currentProgram = index;
}

const juce::String JCBMaximizerAudioProcessor::getProgramName(int index)
{
    return juce::String();
}

void JCBMaximizerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    // No implementado - los nombres de preset son fijos
}

//==============================================================================
// Métodos de medidores
float JCBMaximizerAudioProcessor::getRmsInputValue(const int channel) const noexcept
{
    // CRASH FIX: Safety check - return safe value if not initialized
    if (!isInitialized()) {
        return -100.0f;  // Safe default value
    }
    
    jassert(channel == 0 || channel == 1);
    if (channel == 0)
        return leftInputRMS.load(std::memory_order_relaxed);
    if (channel == 1)
        return rightInputRMS.load(std::memory_order_relaxed);
    return -100.0f;  // Return -100dB for invalid channels
}

float JCBMaximizerAudioProcessor::getRmsOutputValue(const int channel) const noexcept
{
    // CRASH FIX: Safety check - return safe value if not initialized
    if (!isInitialized()) {
        return -100.0f;  // Safe default value
    }
    
    jassert(channel == 0 || channel == 1);
    if (channel == 0)
        return leftOutputRMS.load(std::memory_order_relaxed);
    if (channel == 1)
        return rightOutputRMS.load(std::memory_order_relaxed);
    return -100.0f;  // Return -100dB for invalid channels
}

float JCBMaximizerAudioProcessor::getGainReductionValue(const int channel) const noexcept
{
    // CRASH FIX: Safety check - return safe value if not initialized
    if (!isInitialized()) {
        return 0.0f;  // Safe default value (no gain reduction)
    }
    
    jassert(channel == 0);
    if (channel == 0)
        return gainReduction.load(std::memory_order_relaxed);
    return 0.0f;
}

// MAXIMIZER: No sidechain - removed getSCValue function
/*
float JCBMaximizerAudioProcessor::getSCValue(const int channel) const noexcept
{
    jassert(channel == 0 || channel == 1);
    if (channel == 0)
        return leftSC.load(std::memory_order_relaxed);
    if (channel == 1)
        return rightSC.load(std::memory_order_relaxed);
    return -100.0f;  // Return -100dB for invalid channels
}
*/

//==============================================================================
// Utilidades
bool JCBMaximizerAudioProcessor::isProTools() const noexcept
{
    juce::PluginHostType daw;
    return daw.isProTools();
}

juce::String JCBMaximizerAudioProcessor::getPluginFormat() const noexcept
{
    // Detectar el formato del plugin en tiempo de ejecución
    const auto format = juce::PluginHostType().getPluginLoadedAs();
    
    switch (format)
    {
        case juce::AudioProcessor::wrapperType_VST3:
            return "VST3";
        case juce::AudioProcessor::wrapperType_AudioUnit:
            return "AU";
        case juce::AudioProcessor::wrapperType_AudioUnitv3:
            return "AUv3";
        case juce::AudioProcessor::wrapperType_AAX:
            return "AAX";
        case juce::AudioProcessor::wrapperType_VST:
            return "VST2";
        case juce::AudioProcessor::wrapperType_Standalone:
            return "Standalone";
        default:
            return "";
    }
}


//==============================================================================
// CAPTURA DE DATOS PARA VISUALIZACIÓN DE ENVOLVENTES
//==============================================================================
void JCBMaximizerAudioProcessor::captureInputWaveformData(const juce::AudioBuffer<float>& inputBuffer, int numSamples)
{
    // AUDIO-THREAD SAFE: Usar try_lock para evitar bloquear el audio thread
    std::unique_lock<std::mutex> lock(waveformMutex, std::try_to_lock);
    if (!lock.owns_lock()) {
        // Si no puede obtener el lock, salir inmediatamente para evitar RT violations
        return;
    }
    
    // AUDIO-THREAD SAFE: Usar tamaño fijo pre-asignado, no resize() dinámico
    if (currentInputSamples.size() < static_cast<size_t>(numSamples)) {
        // Solo redimensionar si es absolutamente necesario (debería estar pre-asignado)
        currentInputSamples.resize(juce::jmax(numSamples, 4096));
    }
    
    // CAMBIO: Usar las salidas 3 y 4 de Gen~ (post-TRIM) en lugar de entrada RAW
    // Esto da una visualización más precisa de lo que realmente está procesando el compresor
    for (int i = 0; i < numSamples; ++i)
    {
        // Usar directamente las salidas de Gen~ que son post-TRIM
        if (JCBMaximizer::num_outputs() > 4) {
            // Promedio de canales L/R post-TRIM (salidas 3 y 4 de Gen~)
            currentInputSamples[i] = static_cast<float>((m_OutputBuffers[3][i] + m_OutputBuffers[4][i]) * 0.5);
        } else {
            // Fallback: usar solo canal izquierdo post-TRIM
            currentInputSamples[i] = static_cast<float>(m_OutputBuffers[3][i]);
        }
    }
}

void JCBMaximizerAudioProcessor::captureOutputWaveformData(int numSamples)
{
    // AUDIO-THREAD SAFE: Usar try_lock para evitar bloquear el audio thread
    std::unique_lock<std::mutex> lock(waveformMutex, std::try_to_lock);
    if (!lock.owns_lock()) {
        // Si no puede obtener el lock, salir inmediatamente para evitar RT violations
        return;
    }
    
    // AUDIO-THREAD SAFE: Usar tamaño fijo pre-asignado, no resize() dinámico
    if (currentProcessedSamples.size() < static_cast<size_t>(numSamples)) {
        currentProcessedSamples.resize(juce::jmax(numSamples, 4096));
    }
    if (currentGainReductionSamples.size() < static_cast<size_t>(numSamples)) {
        currentGainReductionSamples.resize(juce::jmax(numSamples, 4096));
    }
    
    // Copiar salida procesada (promedio de canales principales)
    // Y también capturar la gain reduction de Gen~ (salida 2)
    for (int i = 0; i < numSamples; ++i)
    {
        currentProcessedSamples[i] = (m_OutputBuffers[0][i] + m_OutputBuffers[1][i]) * 0.5f;
        
        // Capturar gain reduction directamente de Gen~ (salida 2)
        // Gen~ devuelve gain lineal, convertir a dB
        float grLinear = m_OutputBuffers[2][i];
        currentGainReductionSamples[i] = grLinear > 0.0f ? 
            20.0f * std::log10(grLinear) : -60.0f;
    }
}

void JCBMaximizerAudioProcessor::getWaveformData(std::vector<float>& inputSamples, std::vector<float>& processedSamples) const
{
    std::lock_guard<std::mutex> lock(waveformMutex);
    inputSamples = currentInputSamples;
    processedSamples = currentProcessedSamples;
}

void JCBMaximizerAudioProcessor::getWaveformDataWithGR(std::vector<float>& inputSamples, std::vector<float>& processedSamples, std::vector<float>& gainReductionSamples) const
{
    std::lock_guard<std::mutex> lock(waveformMutex);
    inputSamples = currentInputSamples;
    processedSamples = currentProcessedSamples;
    gainReductionSamples = currentGainReductionSamples;
}

float JCBMaximizerAudioProcessor::getMaxGainReduction() const noexcept
{
    // Método optimizado que obtiene el valor máximo de gain reduction
    // directamente del valor atómico
    return gainReduction.load(std::memory_order_relaxed);
}


bool JCBMaximizerAudioProcessor::isPlaybackActive() const noexcept
{
    // Siempre activo para decay permanente como plugins profesionales
    return true;
}


//==============================================================================
// GESTIÓN DEL EDITOR
//==============================================================================
juce::AudioProcessorEditor* JCBMaximizerAudioProcessor::createEditor()
{
    return new JCBMaximizerAudioProcessorEditor(*this, guiUndoManager);
}


//==============================================================================
// SERIALIZACIÓN DEL ESTADO
//==============================================================================
void JCBMaximizerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Crear una copia del state para no modificar el original
    auto stateCopy = apvts.state.createCopy();
    
    // Remover parámetros momentáneos antes de guardar
    // Estos botones no deben persistir entre sesiones
    auto paramsNode = stateCopy.getChildWithName("PARAMETERS");
    if (paramsNode.isValid()) {
        auto param = paramsNode.getChildWithProperty("id", "h_BYPASS");
        if (param.isValid())
            param.setProperty("value", 0.0f, nullptr);
        
        // MAXIMIZER: Solo resetear BYPASS y DELTA (no tiene SOLOSC)    
        param = paramsNode.getChildWithProperty("id", "k_DELTA");
        if (param.isValid())
            param.setProperty("value", 0.0f, nullptr);
    }
    
    auto preset = stateCopy.getOrCreateChildWithName("Presets", nullptr);
    preset.setProperty("currentPresetID", lastPreset, nullptr);
    preset.setProperty("tooltipEnabled", tooltipEnabled, nullptr);
    preset.setProperty("presetDisplayText", presetDisplayText, nullptr);
    preset.setProperty("presetTextItalic", presetTextItalic, nullptr);
    preset.setProperty("envelopeVisualEnabled", envelopeVisualEnabled, nullptr);
    preset.setProperty("tooltipLanguageEnglish", tooltipLanguageEnglish, nullptr);
    
    // Guardar tamaño del editor
    preset.setProperty("editorWidth", editorSize.x, nullptr);
    preset.setProperty("editorHeight", editorSize.y, nullptr);
    
    // Save A/B states
    auto abNode = stateCopy.getOrCreateChildWithName("ABStates", nullptr);
    abNode.setProperty("isStateA", isStateA, nullptr);
    
    // Save state A
    auto stateANode = abNode.getOrCreateChildWithName("StateA", nullptr);
    stateANode.removeAllChildren(nullptr);
    for (const auto& [paramId, value] : stateA.values) {
        stateANode.setProperty(paramId, value, nullptr);
    }
    
    // Save state B
    auto stateBNode = abNode.getOrCreateChildWithName("StateB", nullptr);
    stateBNode.removeAllChildren(nullptr);
    for (const auto& [paramId, value] : stateB.values) {
        stateBNode.setProperty(paramId, value, nullptr);
    }
    
    juce::MemoryOutputStream memoria(destData, true);
    stateCopy.writeToStream(memoria);
}

void JCBMaximizerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.state = tree;
        
        // Forzar parámetros momentáneos a OFF después de cargar (MAXIMIZER)
        apvts.getParameter("h_BYPASS")->setValueNotifyingHost(0.0f);
        apvts.getParameter("k_DELTA")->setValueNotifyingHost(0.0f);
        // MAXIMIZER: No tiene m_SOLOSC
        
        // Clear undo history AFTER all values have been set
        // This prevents any parameter changes from being recorded in undo history
        guiUndoManager.clearUndoHistory();
        
        auto preset = apvts.state.getChildWithName("Presets");
        if (preset.isValid()) {
            lastPreset = preset.getProperty("currentPresetID");
            tooltipEnabled = preset.getProperty("tooltipEnabled");
            presetDisplayText = preset.getProperty("presetDisplayText", "DEFAULT");
            presetTextItalic = preset.getProperty("presetTextItalic", false);
            envelopeVisualEnabled = preset.getProperty("envelopeVisualEnabled", true);
            tooltipLanguageEnglish = preset.getProperty("tooltipLanguageEnglish", false);
            
            // Restaurar tamaño del editor
            int savedWidth = preset.getProperty("editorWidth", 1250);
            int savedHeight = preset.getProperty("editorHeight", 350);
            editorSize = {savedWidth, savedHeight};
        }
        
        // Restore A/B states
        auto abNode = apvts.state.getChildWithName("ABStates");
        if (abNode.isValid()) {
            isStateA = abNode.getProperty("isStateA", true);
            
            // Restore state A
            auto stateANode = abNode.getChildWithName("StateA");
            if (stateANode.isValid()) {
                stateA.values.clear();
                for (int i = 0; i < stateANode.getNumProperties(); ++i) {
                    auto propName = stateANode.getPropertyName(i);
                    stateA.values[propName.toString()] = stateANode[propName];
                }
            }
            
            // Restore state B
            auto stateBNode = abNode.getChildWithName("StateB");
            if (stateBNode.isValid()) {
                stateB.values.clear();
                for (int i = 0; i < stateBNode.getNumProperties(); ++i) {
                    auto propName = stateBNode.getPropertyName(i);
                    stateB.values[propName.toString()] = stateBNode[propName];
                }
            }
        }
        
        // IMPORTANTE: Sincronizar todos los parámetros con Gen~ después de cargar el estado
        for (int i = 0; i < JCBMaximizer::num_params(); i++) {
            auto paramName = juce::String(JCBMaximizer::getparametername(m_PluginState, i));
            if (auto* param = apvts.getRawParameterValue(paramName)) {
                float value = param->load();
                
                // Corregir valores muy pequeños en ATK y REL
                if (paramName == "d_ATK") {
                    if (value < 0.1f) {
                        value = 0.1f;
                        // Actualizar el parámetro en el APVTS
                        if (auto* audioParam = apvts.getParameter(paramName)) {
                            audioParam->setValueNotifyingHost(audioParam->convertTo0to1(value));
                        }
                    }
                }
                if (paramName == "e_REL") {
                    if (value < 0.1f) {
                        value = 0.1f;
                        // Actualizar el parámetro en el APVTS
                        if (auto* audioParam = apvts.getParameter(paramName)) {
                            audioParam->setValueNotifyingHost(audioParam->convertTo0to1(value));
                        }
                    }
                }
                // NOTA: El compresor no tiene parámetro HOLD (es del expansor/gate)
                
                parameterChanged(paramName, value);
            }
        }
        
        
        // Forzar actualización del editor de forma thread-safe
        // Usar MessageManager para evitar llamadas directas a getActiveEditor()
        juce::MessageManager::callAsync([this]() {
            if (auto* editor = dynamic_cast<JCBMaximizerAudioProcessorEditor*>(getActiveEditor())) {
                // El editor necesita actualizar la función de transferencia
                editor->updateTransferFunctionFromProcessor();
            }
        });
    }
}

//==============================================================================
// COMPARACIÓN A/B
//==============================================================================
void JCBMaximizerAudioProcessor::saveCurrentStateToActive() 
{
    if (isStateA) {
        stateA.captureFrom(apvts);
    } else {
        stateB.captureFrom(apvts);
    }
}

void JCBMaximizerAudioProcessor::toggleAB() 
{
    // Save current state before switching
    saveCurrentStateToActive();
    
    // Switch state
    isStateA = !isStateA;
    
    // Load the other state
    if (isStateA) {
        stateA.applyTo(apvts);
    } else {
        stateB.applyTo(apvts);
    }
}

void JCBMaximizerAudioProcessor::copyAtoB() 
{
    stateA.captureFrom(apvts);
    stateB = stateA;
}

void JCBMaximizerAudioProcessor::copyBtoA() 
{
    stateB.captureFrom(apvts);
    stateA = stateB;
}

// isBeingAutomated() eliminado - ya no necesario con sistema undo simplificado

//==============================================================================
// MÉTODOS LEGACY
//==============================================================================
int JCBMaximizerAudioProcessor::getNumParameters()
{
    // Retornar el número real de parámetros del juce::AudioProcessor
    // que incluye los de Gen~ más cualquier parámetro adicional (como AAX gain reduction)
    return static_cast<int>(getParameters().size());
}

float JCBMaximizerAudioProcessor::getParameter(int index)
{
    // Verificar si el índice está dentro del rango de Gen~
    if (index < JCBMaximizer::num_params())
    {
        t_param value;
        t_param min = JCBMaximizer::getparametermin(m_PluginState, index);
        t_param range = fabs(JCBMaximizer::getparametermax(m_PluginState, index) - min);
        
        JCBMaximizer::getparameter(m_PluginState, index, &value);
        
        value = (value - min) / range;
        
        return value;
    }
    else
    {
        // Manejar parámetros adicionales (como AAX gain reduction)
        // Para el parámetro de gain reduction, devolver 0.0 (sin reducción)
        return 0.0f;
    }
}

void JCBMaximizerAudioProcessor::setParameter(int index, float newValue)
{
    // Verificar si el índice está dentro del rango de Gen~
    if (index < JCBMaximizer::num_params())
    {
        t_param min = JCBMaximizer::getparametermin(m_PluginState, index);
        t_param range = fabs(JCBMaximizer::getparametermax(m_PluginState, index) - min);
        t_param value = newValue * range + min;
        
        JCBMaximizer::setparameter(m_PluginState, index, value, nullptr);
    }
    else
    {
        // Manejar parámetros adicionales (como AAX gain reduction)
        // El parámetro de gain reduction es de solo lectura, no hacer nada
    }
}

const juce::String JCBMaximizerAudioProcessor::getParameterName(int index)
{
    // Verificar si el índice está dentro del rango de Gen~
    if (index < JCBMaximizer::num_params())
    {
        return juce::String(JCBMaximizer::getparametername(m_PluginState, index));
    }
    else
    {
        // Manejar parámetros adicionales (como AAX gain reduction)
        #if JucePlugin_Build_AAX
        if (index == JCBMaximizer::num_params()) // Índice 25 para AAX
        {
            return "Gain Reduction";
        }
        #endif
        return "";
    }
}

const juce::String JCBMaximizerAudioProcessor::getParameterText(int index)
{
    // Método legacy para compatibilidad con hosts - algunos DAWs pueden llamarlo
    if (index >= 0 && index < JCBMaximizer::num_params())
    {
        juce::String text = juce::String(getParameter(index));
        text += juce::String(" ");
        text += juce::String(JCBMaximizer::getparameterunits(m_PluginState, index));
        return text;
    }
    
    // Retornar string vacío para índices inválidos
    return "";
}

const juce::String JCBMaximizerAudioProcessor::getInputChannelName(int channelIndex) const
{
    return juce::String(channelIndex + 1);
}

const juce::String JCBMaximizerAudioProcessor::getOutputChannelName(int channelIndex) const
{
    return juce::String(channelIndex + 1);
}

bool JCBMaximizerAudioProcessor::isInputChannelStereoPair(int index) const
{
    if (isProTools())
    {
        if (getMainBusNumInputChannels() > 1)
            return JCBMaximizer::num_inputs() == 4;
        else
            return false;
    }
    else
        return JCBMaximizer::num_inputs() == 4;
}

bool JCBMaximizerAudioProcessor::isOutputChannelStereoPair(int index) const
{
    return JCBMaximizer::num_outputs() == 2;
}


//==============================================================================
// Clip Detection Methods
//==============================================================================

void JCBMaximizerAudioProcessor::updateClipDetection(const juce::AudioBuffer<float>& inputBuffer, const juce::AudioBuffer<float>& outputBuffer)
{
    const int numSamples = inputBuffer.getNumSamples();
    const auto mainInputChannels = getMainBusNumInputChannels();
    const auto mainOutputChannels = getMainBusNumOutputChannels();
    
    // NOTA: El compresor no tiene soft clip, siempre detectar clips en salida
    bool isSoftClipActive = false; // Compresor no tiene soft clip
    
    // Reset clip flags for this buffer
    bool inputClip[2] = {false, false};
    bool outputClip[2] = {false, false};
    
    // Detectar clips en entrada (usando trimInputBuffer para consistencia con medidores)
    for (int channel = 0; channel < juce::jmin(2, mainInputChannels); ++channel) {
        for (int sample = 0; sample < numSamples; ++sample) {
            if (channel < trimInputBuffer.getNumChannels()) {
                float value = std::abs(trimInputBuffer.getSample(channel, sample));
                if (value >= 0.999f) {  // Umbral de clip ligeramente por debajo de 1.0
                    inputClip[channel] = true;
                    break;  // Solo necesitamos detectar un clip por buffer
                }
            }
        }
    }
    
    // Detectar clips en salida solo si soft clip NO está activo
    if (!isSoftClipActive) {
        for (int channel = 0; channel < juce::jmin(2, mainOutputChannels); ++channel) {
            for (int sample = 0; sample < numSamples; ++sample) {
                float value = std::abs(outputBuffer.getSample(channel, sample));
                if (value >= 0.999f) {  // Umbral de clip ligeramente por debajo de 1.0
                    outputClip[channel] = true;
                    break;  // Solo necesitamos detectar un clip por buffer
                }
            }
        }
    }
    
    // Actualizar flags atómicos
    for (int channel = 0; channel < 2; ++channel) {
        if (inputClip[channel]) {
            inputClipDetected[channel] = true;
        }
        if (outputClip[channel]) {
            outputClipDetected[channel] = true;
        }
    }
}

bool JCBMaximizerAudioProcessor::getInputClipDetected(const int channel) const noexcept
{
    jassert(channel == 0 || channel == 1);
    if (channel >= 0 && channel < 2) {
        return inputClipDetected[channel].load();
    }
    return false;
}

bool JCBMaximizerAudioProcessor::getOutputClipDetected(const int channel) const noexcept
{
    jassert(channel == 0 || channel == 1);
    if (channel >= 0 && channel < 2) {
        return outputClipDetected[channel].load();
    }
    return false;
}

// MAXIMIZER: No sidechain - removed getSidechainClipDetected function
/*
bool JCBMaximizerAudioProcessor::getSidechainClipDetected(const int channel) const noexcept
{
    jassert(channel == 0 || channel == 1);
    if (channel >= 0 && channel < 2) {
        return sidechainClipDetected[channel].load();
    }
    return false;
}
*/

void JCBMaximizerAudioProcessor::resetClipIndicators()
{
    for (int channel = 0; channel < 2; ++channel) {
        inputClipDetected[channel] = false;
        outputClipDetected[channel] = false;
        // MAXIMIZER: No sidechain - removed sidechainClipDetected reset
        // sidechainClipDetected[channel] = false;
    }
}

float JCBMaximizerAudioProcessor::getGainReductionForHost() const noexcept
{
    // Devuelve el valor actual del gain reduction en dB
    // Este valor es actualizado en el audio thread y leído de forma segura aquí
    return currentGainReductionDb.load();
}

//==============================================================================
// Timer implementation
void JCBMaximizerAudioProcessor::timerCallback()
{
    #if JucePlugin_Build_AAX
    // CRÍTICO: Verificar si estamos siendo destruidos para evitar acceso a memoria liberada
    if (isBeingDestroyed.load()) return;
    
    // Actualizar el parámetro zz_GR en Gen~ con el valor de gain reduction
    // Este parámetro es solo para visualización y no participa en undo/redo
    float grLinear = currentGainReductionLinear.load();
    
    // Actualizar el parámetro zz_GR (índice 25) en Gen~
    // JCBMaximizer::setparameter(m_PluginState, 25, grLinear, nullptr); // No necesario - Gen~ ya conoce su propio GR
    
    // También actualizar el parámetro AAX para que Pro Tools lo muestre
    if (aaxGainReductionParam != nullptr) 
    {
        // Pro Tools espera 0.0 = sin reducción, 1.0 = máxima reducción
        float normalizedForProTools = juce::jlimit(0.0f, 1.0f, 1.0f - grLinear);
        
        // Usar setValueNotifyingHost para notificar a Pro Tools
        // Al no estar en APVTS, no afecta al undo/redo
        aaxGainReductionParam->setValueNotifyingHost(normalizedForProTools);
    }
    #endif
}

//==============================================================================
// Format-specific implementations

#if JucePlugin_Build_AAX
float JCBMaximizerAudioProcessor::getAAXMeterValue(int meterID)
{
    // Pro Tools llama a este método para obtener el valor del medidor
    // El ID 0 es típicamente el gain reduction meter
    if (meterID == 0)
    {
        // Obtener el valor lineal de gain reduction
        float grLinear = currentGainReductionLinear.load();
        
        // Gen~ devuelve valores lineales donde:
        // 1.0 = sin reducción
        // < 1.0 = hay reducción
        // Pro Tools espera 0.0 = sin reducción, 1.0 = máxima reducción
        
        // Invertir el valor para Pro Tools
        float normalized = 1.0f - grLinear;
        
        return normalized;
    }
    
    return 0.0f;
}

#endif

#if JucePlugin_Build_VST3
void JCBMaximizerAudioProcessor::updateVST3GainReduction()
{
    // Para VST3, necesitaríamos acceso al IEditController
    // Esto se haría típicamente en el wrapper VST3
    // Por ahora solo preparamos el método
}
#endif

//==============================================================================
// FACTORY FUNCTION DEL PLUGIN
//==============================================================================
/**
 * Función factory requerida por JUCE
 * CRÍTICO: Punto de entrada que utilizan los hosts (DAWs) para crear instancias del plugin
 * Esta función es llamada automáticamente por el framework JUCE cuando:
 * - El DAW carga el plugin por primera vez
 * - Se crea una nueva instancia del plugin en el proyecto
 * - Se duplica una pista que contiene el plugin
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JCBMaximizerAudioProcessor();
}