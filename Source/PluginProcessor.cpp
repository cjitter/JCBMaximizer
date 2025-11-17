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
    
    // 1) Guardar SR/VS en el estado
    m_PluginState->sr = sampleRate;
    m_PluginState->vs = samplesPerBlock;
    
    // 2) Pre-asignar buffers con tamaño máximo esperado para evitar allocations en audio thread
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
    
    // 3) ***CRÍTICO***: sincroniza SR/VS con Gen y re-dimensiona sus delays/constantes
    //    Esto asegura que Gen use el sampleRate real del host
    JCBMaximizer::reset(m_PluginState);
    
    // 4) Cachear índices de gen para evitar bucles por nombre
    genIdxLookahead = genIdxBypass = -1;
    for (int i = 0; i < JCBMaximizer::num_params(); ++i)
    {
        const char* raw = JCBMaximizer::getparametername(m_PluginState, i);
        juce::String name(raw ? raw : "");
        if (name == "n_LOOKAHEAD") genIdxLookahead = i;
        if (name == "h_BYPASS")    genIdxBypass   = i;
    }
    
    // 5) Re-sincronizar TODOS los parámetros con Gen~ DESPUÉS del reset
    for (int i = 0; i < JCBMaximizer::num_params(); i++)
    {
        auto paramName = juce::String(JCBMaximizer::getparametername(m_PluginState, i));
        if (auto* param = apvts.getRawParameterValue(paramName)) {
            float value = param->load();
            JCBMaximizer::setparameter(m_PluginState, i, value, nullptr);
        }
    }
    
    // Calcular latencia inicial
    // Nota: computeLatencySamples() ya incluye intrinsicGenOffset (0 para este plugin)
    const int latenciaSamples = computeLatencySamples(sampleRate);
    setLatencySamples(latenciaSamples);
    currentLatency = latenciaSamples;
    
    // Bypass ring: PRE-ASIGNACIÓN a la latencia máxima + colchón
    {
        const int chans = getTotalNumOutputChannels();
        // Obtener máximo lookahead del parámetro (5ms para JCBMaximizer)
        float maxLAMs = 5.0f; // JCBMaximizer tiene máximo 5ms de lookahead
        if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("n_LOOKAHEAD")))
            maxLAMs = p->range.end;

        const int maxDelaySamples = juce::roundToInt(maxLAMs * (float) sampleRate / 1000.0f);
        const int minCapacity = juce::jmax(maxDelaySamples, samplesPerBlock * 2);

        if (minCapacity > 0) {
            bypassDelayCapacity = minCapacity;
            bypassDelayBuffer.setSize(chans, bypassDelayCapacity);
            bypassDelayBuffer.clear();
            bypassDelayWritePos = 0;
            dryPrimedSamples    = 0;
        }
    }
    
    // Scratch buffers preasignados
    ensureScratchCapacity(juce::jmax(samplesPerBlock, 4096));

    // Configurar longitud de fade desde bypassFadeMs
    {
        const int lenFromMs = juce::roundToInt(bypassFadeMs * (float) getSampleRate() / 1000.0f);
        bypassFadeLen = juce::jlimit(128, 512, lenFromMs > 0 ? lenFromMs : 128);
    }
    bypassFadePos = 0;

    // Estado inicial coherente con el host
    const bool hb = isHostBypassed();
    hostBypassMirror.store(hb, std::memory_order_relaxed);
    bypassState = hb ? BypassState::Bypassed : BypassState::Active;
    lastWantsBypass = hb;
    
    // Inicializar variables de debounce
    if (auto* la = apvts.getRawParameterValue("n_LOOKAHEAD")) {
        stagedLookaheadMs.store(la->load(), std::memory_order_relaxed);
        lastLAChangeMs.store(juce::Time::getMillisecondCounter(), std::memory_order_relaxed);
        laCommitPending.store(false, std::memory_order_relaxed);
    }

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
    
    // leftSC.store(-100.0f, std::memory_order_relaxed);
    // rightSC.store(-100.0f, std::memory_order_relaxed);
    
    // Configurar buffers auxiliares
    grBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    grBuffer.clear();
    
    trimInputBuffer.setSize(2, samplesPerBlock);
    trimInputBuffer.clear();
    
    
    // Inicializar buffers de forma de onda
    currentInputSamples.resize(samplesPerBlock, 0.0f);
    currentProcessedSamples.resize(samplesPerBlock, 0.0f);
}

void JCBMaximizerAudioProcessor::releaseResources()
{
    // Limpiar buffers auxiliares
    grBuffer.setSize(0, 0);
    trimInputBuffer.setSize(0, 0);
}

//==============================================================================
// PROCESAMIENTO DE AUDIO
//==============================================================================
void JCBMaximizerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    processBlockCommon(buffer, /*hostWantsBypass*/ false);
}

void JCBMaximizerAudioProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    processBlockCommon(buffer, /*hostWantsBypass*/ true);
}

void JCBMaximizerAudioProcessor::processBlockCommon(juce::AudioBuffer<float>& buffer, bool hostWantsBypass)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // === Ajustes de buffers (idéntico a tu processBlock actual) ===
    if (grBuffer.getNumChannels() != getTotalNumOutputChannels()
     || grBuffer.getNumSamples()  != numSamples)
        grBuffer.setSize(getTotalNumOutputChannels(), numSamples, false, false, true);
    if (trimInputBuffer.getNumChannels() != 2
     || trimInputBuffer.getNumSamples() != numSamples)
        trimInputBuffer.setSize(2, numSamples, false, false, true);
    { std::unique_lock<std::mutex> lock(waveformMutex, std::try_to_lock);
      if (lock.owns_lock()) {
          if (currentInputSamples.size() < (size_t)numSamples)    currentInputSamples.resize(numSamples);
          if (currentProcessedSamples.size() < (size_t)numSamples) currentProcessedSamples.resize(numSamples);
          if (currentGainReductionSamples.size() < (size_t)numSamples) currentGainReductionSamples.resize(numSamples);
      } }
    assureBufferSize(numSamples);
    
    // === Debounce de lookahead ===
    if (laCommitPending.load(std::memory_order_acquire))
    {
        const uint32_t now  = juce::Time::getMillisecondCounter();
        const uint32_t last = lastLAChangeMs.load(std::memory_order_relaxed);
        if ((int)(now - last) > laDebounceMs)
        {
            // Aplicar a Gen~ ya debounced
            if (genIdxLookahead >= 0) {
                const float ms = stagedLookaheadMs.load(std::memory_order_relaxed);
                JCBMaximizer::setparameter(m_PluginState, genIdxLookahead, ms, nullptr);
            }
            // Actualizar latencia reportada de forma atómica
            const int newL = juce::jmin(computeLatencySamples(getSampleRate()),
                                        juce::jmax(0, bypassDelayCapacity));
            if (newL != pendingLatency.load(std::memory_order_relaxed) && newL != currentLatency) {
                pendingLatency.store(newL, std::memory_order_relaxed);
                triggerAsyncUpdate();
            }
            laCommitPending.store(false, std::memory_order_release);
        }
    }
    
    // Capturar la ENTRADA del host (RT-safe, sin allocs por bloque)
    ensureScratchCapacity (numSamples);
    float* inL = scratchIn.getWritePointer(0);
    float* inR = scratchIn.getWritePointer(1);
    {
        const float* srcL = buffer.getReadPointer(0);
        const float* srcR = (numChannels > 1) ? buffer.getReadPointer(1) : srcL;
        std::memcpy(inL, srcL, sizeof(float) * (size_t) numSamples);
        if (numChannels > 1) std::memcpy(inR, srcR, sizeof(float) * (size_t) numSamples);
        else                 std::memcpy(inR, inL,  sizeof(float) * (size_t) numSamples);
    }
    
    // === Render WET (Gen~ siempre encendido) ===
    fillGenInputBuffers(buffer);
    processGenAudio(numSamples);
    fillOutputBuffers(buffer); // buffer = WET
    auto* wetL = buffer.getWritePointer(0);
    auto* wetR = (numChannels > 1) ? buffer.getWritePointer(1) : wetL;
    
    // === Dry compensado SIEMPRE activo ===
    const int delaySamplesRaw = juce::jmax(0, getLatencySamples());
    const int delaySamples = (bypassDelayCapacity > 0) ? juce::jmin(delaySamplesRaw, bypassDelayCapacity) : 0;
    float* dryL = scratchDry.getWritePointer(0);
    float* dryR = scratchDry.getWritePointer(1);
    {
        const juce::SpinLock::ScopedLockType sl(bypassDelayLock);
        
        // Nunca realocar aquí. Si cambia nº de canales en caliente (raro), rehacer limpio.
        if (bypassDelayCapacity > 0 && bypassDelayBuffer.getNumChannels() != numChannels) {
            bypassDelayBuffer.setSize(numChannels, bypassDelayCapacity);
            bypassDelayBuffer.clear();
            bypassDelayWritePos = 0;
            dryPrimedSamples    = 0;
        }
        // Operación normal
        if (delaySamples <= 0 || bypassDelayCapacity == 0) {
            // Sin latencia efectiva: DRY = entrada instantánea
            for (int n = 0; n < numSamples; ++n) { 
                dryL[n] = inL[n]; 
                if (numChannels > 1) dryR[n] = inR[n]; 
            }
        } else {
            for (int n = 0; n < numSamples; ++n) {
                int readPosA = bypassDelayWritePos - delaySamples;
                while (readPosA < 0) readPosA += bypassDelayCapacity;
                readPosA %= juce::jmax(1, bypassDelayCapacity);
                for (int ch = 0; ch < numChannels; ++ch) {
                    float* dline = bypassDelayBuffer.getWritePointer(ch);
                    const float delayedA = dline[readPosA];
                    const float inSample = (ch == 0 ? inL[n] : (numChannels > 1 ? inR[n] : inL[n]));
                    const bool primedA = (dryPrimedSamples >= delaySamples);
                    float drySample = primedA ? delayedA : inSample;
                    if (ch == 0) dryL[n] = drySample; else dryR[n] = drySample;
                    dline[bypassDelayWritePos] = inSample;
                }
                bypassDelayWritePos = (bypassDelayWritePos + 1) % juce::jmax(1, bypassDelayCapacity);
                dryPrimedSamples    = juce::jmin(dryPrimedSamples + 1, bypassDelayCapacity);
            }
        }
    }
    
    // === FSM y mezcla equal-power ===
    // Latch del estado de bypass al INICIO de bloque
    const bool wantsBypass = hostWantsBypass;
    hostBypassMirror.store(wantsBypass, std::memory_order_relaxed);
    const bool bypassEdge = (wantsBypass != lastWantsBypass);
    lastWantsBypass = wantsBypass;
    
    const int primingTarget = juce::jmax(0, juce::jmin(delaySamples, bypassDelayCapacity) - 1);
    const bool ringReady = (delaySamples <= 0) || (dryPrimedSamples >= primingTarget);
    
    // Arrancar/invertir fade sólo en flanco (bypassEdge) y al comienzo de bloque
    switch (bypassState)
    {
        case BypassState::Active:
            if (bypassEdge && wantsBypass) {
                bypassState = BypassState::FadingToBypass;
                bypassFadePos = 0;
            }
            break;
        case BypassState::Bypassed:
            if (bypassEdge && !wantsBypass) {
                bypassState = BypassState::FadingToActive;
                bypassFadePos = 0;
            }
            break;
        case BypassState::FadingToBypass:
            if (bypassEdge && !wantsBypass) {
                bypassState   = BypassState::FadingToActive;
                bypassFadePos = juce::jmax(0, bypassFadeLen - bypassFadePos);
            }
            break;
        case BypassState::FadingToActive:
            if (bypassEdge && wantsBypass) {
                bypassState   = BypassState::FadingToBypass;
                bypassFadePos = juce::jmax(0, bypassFadeLen - bypassFadePos);
            }
            break;
    }
    
    const bool fading = (bypassState == BypassState::FadingToBypass || bypassState == BypassState::FadingToActive);
    if (!fading)
    {
        if (bypassState == BypassState::Active)
        {
            // WET tal cual (ya está en buffer)
        }
        else // Bypassed
        {
            for (int n = 0; n < numSamples; ++n) { 
                wetL[n] = dryL[n]; 
                if (numChannels > 1) wetR[n] = dryR[n]; 
            }
        }
    }
    else
    {
        // --- Alineación opcional a cruce por cero (reduce micro-clicks en subgraves)
        int fadeStartOffset = 0;
        const bool startingFadeThisBlock =
            ((bypassEdge && wantsBypass  && bypassState == BypassState::FadingToBypass) ||
             (bypassEdge && !wantsBypass && bypassState == BypassState::FadingToActive))
            && (bypassFadePos == 0);
        if (startingFadeThisBlock && ringReady)
        {
            // Señal de referencia: hacia bypass -> alínea a DRY; hacia activo -> alínea a WET
            const float* refL = (bypassState == BypassState::FadingToBypass) ? dryL : wetL;
            const float* refR = (numChannels > 1)
                                    ? ((bypassState == BypassState::FadingToBypass) ? dryR : wetR)
                                    : refL;
            auto nearZero = [](float x) noexcept { return std::abs(x) < 1.0e-5f; };
            const int searchMax = juce::jmin(32, numSamples - 1);
            for (int i = 1; i <= searchMax; ++i)
            {
                const float l0 = refL[i-1], l1 = refL[i];
                const float r0 = refR[i-1], r1 = refR[i];
                const bool zcL = (nearZero(l0) || nearZero(l1) || (l0 * l1 <= 0.0f));
                const bool zcR = (nearZero(r0) || nearZero(r1) || (r0 * r1 <= 0.0f));
                if (zcL || zcR) { fadeStartOffset = i; break; }
            }
        }
        
        for (int n = 0; n < numSamples; ++n)
        {
            float wWet = 0.0f, wDry = 0.0f;
            // Antes del offset elegido, fija pesos y NO avances el fade
            if (n < fadeStartOffset)
            {
                if (bypassState == BypassState::FadingToBypass) { wWet = 1.0f; wDry = 0.0f; }
                else                                            { wWet = 0.0f; wDry = 1.0f; }
                const float outL = wWet * wetL[n] + wDry * dryL[n];
                const float outR = wWet * wetR[n] + wDry * (numChannels > 1 ? dryR[n] : dryL[n]);
                wetL[n] = outL; 
                if (numChannels > 1) wetR[n] = outR;
                continue;
            }
            
            if (! ringReady)
            {
                // Aún no hay historia suficiente en el ring: fija pesos y NO avances el fade
                if (bypassState == BypassState::FadingToBypass) { wWet = 1.0f; wDry = 0.0f; }
                else                                            { wWet = 0.0f; wDry = 1.0f; }
            }
            else
            {
                const float t = juce::jlimit(0.0f, 1.0f,
                                             (float) bypassFadePos / (float) juce::jmax(1, bypassFadeLen));
                // Ventana Hann (sin^2 / cos^2) para mantener amplitud estable
                const float s = std::sin(t * juce::MathConstants<float>::halfPi);
                const float c = std::cos(t * juce::MathConstants<float>::halfPi);
                
                if (bypassState == BypassState::FadingToBypass) {
                    wWet = c * c;   // cos^2 (de 1 a 0)
                    wDry = s * s;   // sin^2 (de 0 a 1)
                } else {
                    wWet = s * s;   // sin^2 (de 0 a 1)
                    wDry = c * c;   // cos^2 (de 1 a 0)
                }
            }
            
            const float outL = wWet * wetL[n] + wDry * dryL[n];
            const float outR = wWet * wetR[n] + wDry * (numChannels > 1 ? dryR[n] : dryL[n]);
            wetL[n] = outL; 
            if (numChannels > 1) wetR[n] = outR;
            
            // Avanzar el fade SOLO si el ring está listo
            if (ringReady)
            {
                ++bypassFadePos;
                if (bypassFadePos >= bypassFadeLen)
                {
                    bypassState = (bypassState == BypassState::FadingToBypass) ? BypassState::Bypassed
                                                                               : BypassState::Active;
                    // Resto del bloque ya en estado final
                    if (bypassState == BypassState::Bypassed) {
                        for (int k = n + 1; k < numSamples; ++k) { 
                            wetL[k] = dryL[k]; 
                            if (numChannels > 1) wetR[k] = dryR[k]; 
                        }
                    }
                    break;
                }
            }
        }
    }
    
    // Safety: sanitize final output and recover Gen state if needed
#if !defined(JCB_DISABLE_SANITIZER)
    sanitizeStereo(wetL, (numChannels > 1 ? wetR : nullptr), numSamples, nanTripped);
#endif

    if (nanTripped.exchange(false, std::memory_order_acq_rel))
    {
        if (m_PluginState != nullptr)
        {
            JCBMaximizer::reset(m_PluginState);

            for (int i = 0; i < JCBMaximizer::num_params(); ++i)
            {
                const char* raw = JCBMaximizer::getparametername(m_PluginState, i);
                const juce::String paramId(raw ? raw : "");

                if (auto* param = apvts.getRawParameterValue(paramId))
                    JCBMaximizer::setparameter(m_PluginState, i, param->load(), nullptr);
            }
        }
    }

    // === Meters y capturas (como al final de tu processBlock) ===
    captureInputWaveformData(buffer, numSamples);
    captureOutputWaveformData(numSamples);
    updateClipDetection(buffer, buffer);
    updateInputMeters(buffer);
    updateOutputMeters(buffer);
    updateGainReductionMeter();
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
    
    // ULTRA-OPTIMIZADO: outlet 3 → average~ 1200 1 → linear2db → slider (era 4800→2400, ahora ultra-responsivo)
    float finalAveragedLinear = 1.0f; // Inicializar a 1.0 (sin reducción)
    
    // Verificar si hay muestras para procesar
    if (numSamples > 0 && grData != nullptr) {
        // Procesar cada muestra con promedio móvil de 1200 muestras (modo absoluto, ultra-responsivo)
        for (int i = 0; i < numSamples; ++i) {
            float grLinear = static_cast<float>(grData[i]);
            // Aplicar promedio móvil con modo absoluto (average~ 1200 1)
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



//==============================================================================
// CONFIGURACIÓN DE BUSES Y PARÁMETROS
//==============================================================================
juce::AudioProcessor::BusesProperties JCBMaximizerAudioProcessor::createBusesProperties()
{
    auto propBuses = juce::AudioProcessor::BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        .withInput("Input", juce::AudioChannelSet::stereo(), true);

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

   // a_GAIN @min 0 @max 24 @default 0 (Maximizer: ahora a_GAIN, antes a_THD, con valores positivos)
   auto thd = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("a_GAIN", versionHint),
                                                          juce::CharPointer_UTF8("Gain"),
                                                          juce::NormalisableRange<float>(0.f, 24.f, 0.1f, 1.0f),
                                                          0.f);

   // b_CELLING @min -60 @max 0 @default -0.3 (NUEVO - específico del Maximizer)
   auto ceiling = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("b_CELLING", versionHint),
                                                              juce::CharPointer_UTF8("Ceiling"),
                                                              juce::NormalisableRange<float>(-60.f, 0.f, 0.1f, 1.0f),
                                                              -0.3f);

   // d_ATK @min 0.01 @max 750 @default 100 (mantener mapeo exponencial)
   auto atkRange = juce::NormalisableRange<float>(
       0.01f, 750.f,
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
       100.0f,
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

   // e_REL @min 1 @max 1000 @default 200 (cambiar rango y default para Maximizer)
   auto relRange = juce::NormalisableRange<float>(
       1.f, 1000.f,
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
       200.0f,  // Cambiar default de 120 a 200
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

   // i_MAKEUP @min -12 @max 12 @default 0 (POST processing makeup gain)
   auto makeup = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("i_MAKEUP", versionHint),
                                                             juce::CharPointer_UTF8("Makeup"),
                                                             juce::NormalisableRange<float>(-12.f, 12.f, 0.1f, 1.f),
                                                             0.f);

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

   // l_DETECT @min 0 @max 1 @default 1 (NUEVO - Detection mode Peak/RMS, corregido a RMS por defecto)
   auto detect = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("l_DETECT", versionHint),
                                                             juce::CharPointer_UTF8("Detection"),
                                                             juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                             1.f,  // Default corregido: 0 → 1 (RMS)
                                                             juce::String(),
                                                             juce::AudioParameterFloat::genericParameter,
                                                             [](float value, int){
                                                                 if (value < 0.01f)
                                                                     return juce::String("Peak");
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

   // n_LOOKAHEAD @min 0 @max 5 @default 0 (corregido a especificación del usuario)
   auto lookahead = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("n_LOOKAHEAD", versionHint),
                                                                juce::CharPointer_UTF8("Lookahead"),
                                                                juce::NormalisableRange<float>(0.f, 5.f, 0.1f, 1.f),
                                                                0.f);  // Default corregido: 2 → 0


   // o_DCFILT @min 0 @max 1 @default 0 (Filtro DC offset post-procesamiento)
   auto dcfilter = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("o_DCFILT", versionHint),
                                                              juce::CharPointer_UTF8("DC Filter"),
                                                              juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                              0.f,
                                                              juce::String(),
                                                              juce::AudioParameterFloat::genericParameter,
                                                              [](float value, int) { return value < 0.5f ? "OFF" : "ON"; },
                                                              nullptr);

   // Añadir parámetros en orden alfabético (exactamente como Gen~)
   params.push_back(std::move(thd));            // a_THD
   params.push_back(std::move(ceiling));        // b_CELLING
   params.push_back(std::move(atk));            // d_ATK
   params.push_back(std::move(rel));            // e_REL
   params.push_back(std::move(dither));         // g_DITHER
   params.push_back(std::move(bypass));         // h_BYPASS
   params.push_back(std::move(makeup));         // i_MAKEUP
   params.push_back(std::move(trim));           // j_TRIM
   params.push_back(std::move(delta));          // k_DELTA
   params.push_back(std::move(detect));         // l_DETECT
   params.push_back(std::move(autorel));        // m_AUTOREL
   params.push_back(std::move(lookahead));      // n_LOOKAHEAD
   params.push_back(std::move(dcfilter));       // o_DCFILT

   // NOTA: El parámetro aax_gr_meter se añade directamente en el constructor
   // para evitar que forme parte del APVTS y el sistema undo/redo

   return { params.begin(), params.end() };
}


//==============================================================================
// ASYNC UPDATE HANDLER (para actualizaciones de latencia thread-safe)
//==============================================================================
void JCBMaximizerAudioProcessor::handleAsyncUpdate()
{
    if (isBeingDestroyed.load()) return;
    
    const int L = pendingLatency.exchange(-1, std::memory_order_acq_rel);
    if (L < 0 || L == currentLatency) return;
    
    // Notificar al host
    setLatencySamples(L);
    currentLatency = L;

    // NO redimensionar el ring buffer aquí - mantener capacidad fija
    const juce::SpinLock::ScopedLockType sl(bypassDelayLock);
    if (bypassDelayCapacity > 0)
    {
        bypassDelayBuffer.clear();
        bypassDelayWritePos = 0;
        dryPrimedSamples = 0;
    }
}

//==============================================================================
// GESTIÓN DE PARÁMETROS
//==============================================================================
void JCBMaximizerAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    
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
    
    // Caso especial: LOOKAHEAD necesita actualizar la latencia del host con debounce
    if (parameterID == "n_LOOKAHEAD")
    {
        // Stage para audio thread (con debounce)
        stagedLookaheadMs.store(newValue, std::memory_order_relaxed);
        lastLAChangeMs.store(juce::Time::getMillisecondCounter(), std::memory_order_relaxed);
        laCommitPending.store(true, std::memory_order_release);
        // NO aplicar inmediatamente a Gen~, esperar al debounce en processBlockCommon
    }
    else
    {
        // Para todos los demás parámetros, aplicar directamente
        JCBMaximizer::setparameter(m_PluginState, genIndex, newValue, nullptr);
    }
}

//==============================================================================
// Métodos de programa (presets)
int JCBMaximizerAudioProcessor::getNumPrograms()
{
    return 0;
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

void JCBMaximizerAudioProcessor::resetClipIndicators()
{
    for (int channel = 0; channel < 2; ++channel) {
        inputClipDetected[channel] = false;
        outputClipDetected[channel] = false;
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