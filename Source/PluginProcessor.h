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
// Módulos JUCE
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

// Librerías estándar C++
#include <mutex>
#include <vector>
#include <unordered_map>

// Archivos del proyecto
#include "JCBMaximizer.h"
#include "Helpers/MovingAverage4800.h"

using namespace juce;

//==============================================================================
// CLASE PRINCIPAL DEL PROCESADOR
//==============================================================================
class JCBMaximizerAudioProcessor : public juce::AudioProcessor,
                                    public juce::AudioProcessorValueTreeState::Listener,
                                    private juce::Timer
{
public:
    //==============================================================================
    // Constructor y destructor
    JCBMaximizerAudioProcessor();
    ~JCBMaximizerAudioProcessor() override;
    
    //==============================================================================
    // Métodos principales del AudioProcessor
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
    bool isBusesLayoutSupported(const juce::AudioProcessor::BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //==============================================================================
    // Gestión del editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const noexcept override { return true; }
    
    //==============================================================================
    // Propiedades básicas del plugin
    const juce::String getName() const noexcept override { return JucePlugin_Name; }
    
    bool acceptsMidi() const noexcept override { return false; }
    bool producesMidi() const noexcept override { return false; }
    bool silenceInProducesSilenceOut() const noexcept override { return false; }
    double getTailLengthSeconds() const noexcept override { return 5.0; }
    
    //==============================================================================
    // Gestión de presets/programs
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    
    //==============================================================================
    // Serialización del estado
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    //==============================================================================
    // Reportes de reducción de ganancia específicos por formato
    #if JucePlugin_Build_AAX
    float getAAXMeterValue(int meterID);
    #endif
    
    #if JucePlugin_Build_VST3
    void updateVST3GainReduction();
    #endif
    
    #if JucePlugin_Build_AU
    // AU: reducción de ganancia expuesta a través del wrapper
    #endif
    
    //==============================================================================
    // APVTS y gestión de parámetros
    juce::AudioProcessorValueTreeState apvts;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    //==============================================================================
    // Gestión de estado de la interfaz
    juce::Point<int> getSavedSize() const noexcept { return editorSize; }
    void setSavedSize(const juce::Point<int>& size) noexcept { editorSize = size; }
    
    int getLastPreset() const noexcept { return lastPreset; }
    void setLastPreset(const int& presetID) noexcept { lastPreset = presetID; }
    
    // Estado del menú de presets
    juce::String getPresetDisplayText() const noexcept { return presetDisplayText; }
    void setPresetDisplayText(const juce::String& text) noexcept { presetDisplayText = text; }
    bool getPresetTextItalic() const noexcept { return presetTextItalic; }
    void setPresetTextItalic(bool italic) noexcept { presetTextItalic = italic; }
    
    // Detección de formato del DAW
    bool isProTools() const noexcept;
    juce::String getPluginFormat() const noexcept;
    
    // Estado de tooltips e idioma
    bool getTooltipEnabled() const noexcept { return tooltipEnabled; }
    void setTooltipEnabled(bool enabled) noexcept { tooltipEnabled = enabled; }
    bool getEnvelopeVisualEnabled() const noexcept { return envelopeVisualEnabled; }
    void setEnvelopeVisualEnabled(bool enabled) noexcept { envelopeVisualEnabled = enabled; }
    bool getTooltipLanguageEnglish() const noexcept { return tooltipLanguageEnglish; }
    void setTooltipLanguageEnglish(bool english) noexcept { tooltipLanguageEnglish = english; }
    
    // Métodos de comparación A/B
    void saveCurrentStateToActive();
    void toggleAB();
    void copyAtoB();
    void copyBtoA();
    bool getIsStateA() const noexcept { return isStateA; }
    
    // Acceso al UndoManager de GUI
    juce::UndoManager& getGuiUndoManager() noexcept { return guiUndoManager; }
    
    //==============================================================================
    // Medidores de audio
    float getRmsInputValue(const int channel) const noexcept;
    float getRmsOutputValue(const int channel) const noexcept;
    float getGainReductionValue(const int channel) const noexcept;
    // MAXIMIZER: No sidechain - removed getSCValue
    
    // Detección de clipping
    bool getInputClipDetected(const int channel) const noexcept;
    bool getOutputClipDetected(const int channel) const noexcept;
    // MAXIMIZER: No sidechain - removed getSidechainClipDetected
    void resetClipIndicators();
    
    // Reducción de ganancia para hosts/DAWs
    float getGainReductionForHost() const noexcept;
    
    // Datos de forma de onda
    void getWaveformData(std::vector<float>& inputSamples, std::vector<float>& processedSamples) const;
    void getWaveformDataWithGR(std::vector<float>& inputSamples, std::vector<float>& processedSamples, std::vector<float>& gainReductionSamples) const;
    float getMaxGainReduction() const noexcept;
    
    // Sistema híbrido: timestamp + playhead para detección más robusta
    bool isPlaybackActive() const noexcept;
    
    //==============================================================================
    // Métodos de parámetros legacy
    int getNumParameters() override;
    float getParameter(int index) override;
    void setParameter(int index, float newValue) override;
    const juce::String getParameterName(int index) override;
    const juce::String getParameterText(int index) override;
    
    //==============================================================================
    // Métodos de canales
    const juce::String getInputChannelName(int channelIndex) const override;
    const juce::String getOutputChannelName(int channelIndex) const override;
    bool isInputChannelStereoPair(int index) const override;
    bool isOutputChannelStereoPair(int index) const override;
    
    // Acceso al estado de Gen~ (para sincronización directa)
    CommonState* getPluginState() const { return m_PluginState; }
    
    // Verificar si el processor está completamente inicializado
    bool isInitialized() const noexcept { return m_PluginState != nullptr; }
    
private:
    //==============================================================================
    // UndoManager separado para GUI
    juce::UndoManager guiUndoManager;
    
    //==============================================================================
    // Integración Gen~
    void assureBufferSize(long bufferSize);
    void fillGenInputBuffers(const juce::AudioBuffer<float>& buffer);
    void processGenAudio(int numSamples);
    void fillOutputBuffers(juce::AudioBuffer<float>& buffer);
    
    // Actualizaciones de medidores
    void updateInputMeters(const juce::AudioBuffer<float>& buffer);
    void updateOutputMeters(const juce::AudioBuffer<float>& buffer);
    void updateGainReductionMeter();
    void updateSidechainMeters(const juce::AudioBuffer<float>& buffer);
    void captureInputWaveformData(const juce::AudioBuffer<float>& inputBuffer, int numSamples);
    void captureOutputWaveformData(int numSamples);
    
    // Detección de clipping
    void updateClipDetection(const juce::AudioBuffer<float>& inputBuffer, const juce::AudioBuffer<float>& outputBuffer);
    
    // Gestión de layout y buses
    juce::AudioProcessor::BusesProperties createBusesProperties();
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    //==============================================================================
    // Estado Gen~
    CommonState* m_PluginState;
    long m_CurrentBufferSize;
    t_sample** m_InputBuffers;
    t_sample** m_OutputBuffers;
    
    // Valores de medidores thread-safe
    std::atomic<float> leftInputRMS{-100.0f};
    std::atomic<float> rightInputRMS{-100.0f};
    std::atomic<float> leftOutputRMS{-100.0f};
    std::atomic<float> rightOutputRMS{-100.0f};
    std::atomic<float> gainReduction{0.0f};
    
    // Promedio móvil para gain reduction (replica average~ 4800 1 de Max)
    MovingAverage4800 grMovingAverage;
    // MAXIMIZER: No sidechain - removed leftSC, rightSC, sidechainBuffer
    
    // Buffers auxiliares
    juce::AudioBuffer<float> grBuffer;
    juce::AudioBuffer<float> trimInputBuffer;
    
    // Buffers para forma de onda
    mutable std::vector<float> currentInputSamples;
    mutable std::vector<float> currentProcessedSamples;
    mutable std::vector<float> currentGainReductionSamples;
    mutable std::mutex waveformMutex;
    
    // Gestión de estado
    juce::Point<int> editorSize{1250, 350};
    int lastPreset{0};
    int currentProgram{-1};
    
    // Estado del menú de presets
    juce::String presetDisplayText{"DEFAULT"};
    bool presetTextItalic{false};
    
    bool tooltipEnabled{true};
    bool envelopeVisualEnabled{true};
    bool tooltipLanguageEnglish{false};
    
    // Estado de detección de clipping
    std::atomic<bool> inputClipDetected[2] = {false, false};
    std::atomic<bool> outputClipDetected[2] = {false, false};
    // MAXIMIZER: No sidechain - removed sidechainClipDetected
    
    // Valor de reducción de ganancia para hosts
    mutable std::atomic<float> currentGainReductionDb{0.0f};
    mutable std::atomic<float> currentGainReductionLinear{1.0f};
    
    // Parámetro AAX de reducción de ganancia
    #if JucePlugin_Build_AAX
    juce::AudioParameterFloat* aaxGainReductionParam = nullptr;
    #endif
    
    
    // Flag para indicar destrucción del plugin
    std::atomic<bool> isBeingDestroyed{false};
    
    //==============================================================================
    // Timer callback para actualizaciones fuera del audio thread
    void timerCallback() override;
    
    // Sistema de comparación A/B
    struct ParameterState {
        std::unordered_map<juce::String, float> values;
        
        void captureFrom(const juce::AudioProcessorValueTreeState& apvts) {
            values.clear();
            auto& params = apvts.processor.getParameters();
            for (auto* param : params) {
                if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(param)) {
                    values[p->paramID] = p->getValue();
                }
            }
        }
        
        void applyTo(juce::AudioProcessorValueTreeState& apvts) const {
            for (const auto& [id, value] : values) {
                if (auto* param = apvts.getParameter(id)) {
                    param->setValueNotifyingHost(value);
                }
            }
        }
    };
    
    ParameterState stateA;
    ParameterState stateB;
    bool isStateA{true};
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JCBMaximizerAudioProcessor)
};