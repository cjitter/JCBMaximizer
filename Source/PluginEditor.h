#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class JCBMaximizerAudioProcessorEditor : public AudioProcessorEditor,
                                         public juce::Slider::Listener
{
public:
    JCBMaximizerAudioProcessorEditor (JCBMaximizerAudioProcessor&);
   ~JCBMaximizerAudioProcessorEditor();

    // =============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void sliderValueChanged (juce::Slider* slider) override;
    
private:
    
    juce::TooltipWindow tooltipVentana;
    
    // Declarar variables
    int const ancho {700};
    int const alto  {200};
    
    juce::Label labelThd;
    juce::Label labelCelling;
    juce::Label labelAtk;
    juce::Label labelRel;
    juce::Label labelRel2;
    juce::Label labelBypass;
    juce::Label labelDither;
    
    juce::Label labelUnity;
;
   // Texto adicional 
    juce::DrawableText txtExtra;
    // Link

    juce::HyperlinkButton titleLink;
    
    // Imagen de fondo como componente
    juce::ImageComponent mFondo;

    // Sliders
    juce::Slider thdSlider;
    juce::Slider cellingSlider;

    juce::Slider atkSlider;
    juce::Slider relSlider;
    juce::Slider rel2Slider;
    juce::Slider bypassSlider;
    juce::Slider ditherSlider;
    
    juce::Slider unitySlider;

    // Sliders attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thdSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cellingSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> atkSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> relSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rel2SliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bypassSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ditherSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unitySliderAttachment;
 
   JCBMaximizerAudioProcessor& processor;
;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JCBMaximizerAudioProcessorEditor)
};

#endif  // PLUGINEDITOR_H_INCLUDED


