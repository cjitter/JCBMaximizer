#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
JCBMaximizerAudioProcessor::JCBMaximizerAudioProcessor() : apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
, m_CurrentBufferSize(0)
{
    // use a default samplerate and vector size here, reset it later
    m_PluginState = (CommonState *)MAXIMIZER_GEN::create(44100, 64);
    MAXIMIZER_GEN::reset(m_PluginState);
    
    m_InputBuffers = new t_sample *[MAXIMIZER_GEN::num_inputs()];
    m_OutputBuffers = new t_sample *[MAXIMIZER_GEN::num_outputs()];
    
    for (int i = 0; i < MAXIMIZER_GEN::num_inputs(); i++) {
        m_InputBuffers[i] = NULL;
    }
    for (int i = 0; i < MAXIMIZER_GEN::num_outputs(); i++) {
        m_OutputBuffers[i] = NULL;
    }
    
    // MUY IMPORTNTE: vincular parámetros de gen~ con el APVTS
    for (int i = 0; i < MAXIMIZER_GEN::num_params(); i++)
    {
        auto name = juce::String (MAXIMIZER_GEN::getparametername (m_PluginState, i));
        apvts.addParameterListener (name, this);
    }
    
}

JCBMaximizerAudioProcessor::~JCBMaximizerAudioProcessor()
{
    MAXIMIZER_GEN::destroy(m_PluginState);
    
    // destruir parámetros del apvts
    for (int i = 0; i < MAXIMIZER_GEN::num_params(); i++)
    {
    auto name = juce::String (MAXIMIZER_GEN::getparametername(m_PluginState, i));
    apvts.removeParameterListener(name, this);
    }
    
}

//==============================================================================
const String JCBMaximizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int JCBMaximizerAudioProcessor::getNumParameters()
{
    return MAXIMIZER_GEN::num_params();
}

////////////////////////////////

float JCBMaximizerAudioProcessor::getParameter (int index)
{
    t_param value;
    t_param min = MAXIMIZER_GEN::getparametermin(m_PluginState, index);
    t_param range = fabs(MAXIMIZER_GEN::getparametermax(m_PluginState, index) - min);
    
    MAXIMIZER_GEN::getparameter(m_PluginState, index, &value);
    
    value = (value - min) / range;
    
    return value;
}

void JCBMaximizerAudioProcessor::setParameter (int index, float newValue)
{
    t_param min = MAXIMIZER_GEN::getparametermin(m_PluginState, index);
    t_param range = fabs(MAXIMIZER_GEN::getparametermax(m_PluginState, index) - min);
    t_param value = newValue * range + min;

    MAXIMIZER_GEN::setparameter(m_PluginState, index, value, NULL);
}

////////////////////////////////

const String JCBMaximizerAudioProcessor::getParameterName (int index)
{
    return String(MAXIMIZER_GEN::getparametername(m_PluginState, index));
}

const String JCBMaximizerAudioProcessor::getParameterText (int index)
{
    String text = String(getParameter(index));
    text += String(" ");
    text += String(MAXIMIZER_GEN::getparameterunits(m_PluginState, index));
    
    return text;
}

const String JCBMaximizerAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String JCBMaximizerAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool JCBMaximizerAudioProcessor::isInputChannelStereoPair (int index) const
{
    return MAXIMIZER_GEN::num_inputs() == 2;
}

bool JCBMaximizerAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return MAXIMIZER_GEN::num_outputs() == 2;
}

bool JCBMaximizerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool JCBMaximizerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool JCBMaximizerAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double JCBMaximizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JCBMaximizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int JCBMaximizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JCBMaximizerAudioProcessor::setCurrentProgram (int index)
{
}

const String JCBMaximizerAudioProcessor::getProgramName (int index)
{
    return String();
}

void JCBMaximizerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void JCBMaximizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    // initialize samplerate and vectorsize with the correct values
    m_PluginState->sr = sampleRate;
    m_PluginState->vs = samplesPerBlock;
    
    assureBufferSize(samplesPerBlock);
    setLatencySamples(mLatencia*sampleRate);
}

void JCBMaximizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


void JCBMaximizerAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    processBlockBypassed(buffer, midiMessages);

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    assureBufferSize(buffer.getNumSamples());
    

    
    // fill input buffers
    for (int i = 0; i < MAXIMIZER_GEN::num_inputs(); i++) {
        if (i < totalNumInputChannels) {
            for (int j = 0; j < m_CurrentBufferSize; j++) {
                m_InputBuffers[i][j] = buffer.getReadPointer(i)[j];
            }
        } else {
            memset(m_InputBuffers[i], 0, m_CurrentBufferSize *  sizeof(double));
        }
    }
    
    // process audio
    MAXIMIZER_GEN::perform(m_PluginState,
                           m_InputBuffers,
                           MAXIMIZER_GEN::num_inputs(),
                           m_OutputBuffers,
                           MAXIMIZER_GEN::num_outputs(),
                           buffer.getNumSamples());
    
    // fill output buffers
    for (int i = 0; i < totalNumOutputChannels; i++) {
        if (i < MAXIMIZER_GEN::num_outputs()) {
            for (int j = 0; j < buffer.getNumSamples(); j++) {
                buffer.getWritePointer(i)[j] = m_OutputBuffers[i][j];
            }
        } else {
            buffer.clear (i, 0, buffer.getNumSamples());
        }
    }
}

//==============================================================================
bool JCBMaximizerAudioProcessor::hasEditor() const
{
    return true; // (change this to true if you choose to supply an editor)
}

AudioProcessorEditor* JCBMaximizerAudioProcessor::createEditor()
{
    return new JCBMaximizerAudioProcessorEditor (*this);
    //return new GenericAudioProcessorEditor (*this);
}

//==============================================================================
void JCBMaximizerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    juce::MemoryOutputStream memoria (destData, true);
    apvts.state.writeToStream (memoria);
}

void JCBMaximizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, sizeInBytes);
    if (tree.isValid()) {
        apvts.state = tree;
    }
}

Point<int> JCBMaximizerAudioProcessor::getSavedSize() const
{
    return editorSize;
}

void JCBMaximizerAudioProcessor::setSavedSize (const Point<int>& size)
{
    editorSize = size;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JCBMaximizerAudioProcessor();
}

//==============================================================================
// C74 added methods

void JCBMaximizerAudioProcessor::assureBufferSize(long bufferSize)
{
    if (bufferSize > m_CurrentBufferSize) {
        for (int i = 0; i < MAXIMIZER_GEN::num_inputs(); i++) {
            if (m_InputBuffers[i]) delete m_InputBuffers[i];
            m_InputBuffers[i] = new t_sample[bufferSize];
        }
        for (int i = 0; i < MAXIMIZER_GEN::num_outputs(); i++) {
            if (m_OutputBuffers[i]) delete m_OutputBuffers[i];
            m_OutputBuffers[i] = new t_sample[bufferSize];
        }
        
        m_CurrentBufferSize = bufferSize;
    }
}

//==============================================================================
// Implementación  para APVTS: createParameterLayout y parameterChanged

juce::AudioProcessorValueTreeState::ParameterLayout JCBMaximizerAudioProcessor::createParameterLayout()
{
    const int versionHint = 0;

    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    
    auto maxi_thd = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("a_THD", versionHint),
                                                              juce::CharPointer_UTF8("THD"),
                                                              NormalisableRange<float>(-20.f,
                                                                                       0.f,
                                                                                       0.1f,
                                                                                       1.0f),
                                                              0.f);
    
    auto maxi_celling = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("b_CELLING", versionHint),
                                                              juce::CharPointer_UTF8("Celling"),
                                                              NormalisableRange<float>(-20.f,
                                                                                       0.f,
                                                                                       0.1f,
                                                                                       0.5f),
                                                              0.f);
    
    
    auto maxi_atk = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("d_ATK", versionHint),
                                                              juce::CharPointer_UTF8("Atack"),
                                                              NormalisableRange<float>(0.01f,
                                                                                       500.f,
                                                                                       0.01f,
                                                                                       0.5f),
                                                              0.1f);
    
    auto maxi_rel = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("e_REL", versionHint),
                                                              juce::CharPointer_UTF8("Release"),
                                                              NormalisableRange<float>(1.f,
                                                                                       1500.f,
                                                                                       0.1f,
                                                                                       0.5f),
                                                              10.f);
    
    auto maxi_rel2 = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("f_REL2", versionHint),
                                                              juce::CharPointer_UTF8("Release 2"),
                                                              NormalisableRange<float>(5.f,
                                                                                       500.f,
                                                                                       0.1f,
                                                                                       0.5f),
                                                              5.f);
    
    auto maxi_dither = std::make_unique<juce::AudioParameterInt>(juce::ParameterID("g_DITHER", versionHint),
                                                               juce::CharPointer_UTF8("Dither 16 bits"),
                                                               0,
                                                               1,
                                                               0);
    
    auto maxi_bypass = std::make_unique<juce::AudioParameterInt>(juce::ParameterID("h_BYPASS", versionHint),
                                                               juce::CharPointer_UTF8("Bypass"),
                                                               0,
                                                               1,
                                                               0);
    
    auto maxi_unity = std::make_unique<juce::AudioParameterInt>(juce::ParameterID("i_UNITY", versionHint),
                                                               juce::CharPointer_UTF8("Unity Gain"),
                                                               0,
                                                               1,
                                                               0);
    
    
    params.push_back(std::move(maxi_thd));
    params.push_back(std::move(maxi_celling));
    params.push_back(std::move(maxi_atk));
    params.push_back(std::move(maxi_rel));
    params.push_back(std::move(maxi_rel2));
    params.push_back(std::move(maxi_dither));
    params.push_back(std::move(maxi_bypass));
    
    params.push_back(std::move(maxi_unity));

    
    return { params.begin(), params.end() };
}

void JCBMaximizerAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    auto index = apvts.getParameter(parameterID)->getParameterIndex();
    MAXIMIZER_GEN::setparameter(m_PluginState, index, newValue, nullptr);
}
