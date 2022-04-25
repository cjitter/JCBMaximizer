#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JCBMaximizerAudioProcessorEditor::JCBMaximizerAudioProcessorEditor (JCBMaximizerAudioProcessor& p)
: AudioProcessorEditor (&p), processor (p)
{
    
    auto backgroundImage = ImageCache::getFromMemory(BinaryData::morado_jpg,
                                                     BinaryData::morado_jpgSize);
    
    if (!backgroundImage.isNull())
    mFondo.setImage(backgroundImage, RectanglePlacement::stretchToFit);
    else
        jassert(!backgroundImage.isNull());
    
    addAndMakeVisible(mFondo);

    addAndMakeVisible (titleLink);
    titleLink.setURL(juce::URL("https://github.com/cjitter"));
    titleLink.setColour (juce::HyperlinkButton::textColourId, juce::Colours::azure);
    titleLink.changeWidthToFitText();
    titleLink.setFont(juce::Font(4.f), true);
    titleLink.setTooltip (TRANS("https://github.com/cjitter"));
    titleLink.setButtonText (TRANS("JCBMaximizer 0.0.2"));
    

    // MAXIMIZER
    addAndMakeVisible(thdSlider);
    thdSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    thdSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 75, 20);
    thdSlider.setColour(juce::Slider::textBoxOutlineColourId, Colour(0,0,0));
    
    thdSlider.setTextValueSuffix(" dB");
    thdSlider.setTextBoxIsEditable(true);
            
    thdSlider.setColour (juce::Slider::thumbColourId, juce::Colours::darkviolet);
    thdSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::darkviolet);
    thdSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    thdSlider.setDoubleClickReturnValue (true, -10);
    thdSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,
                                                                                                 "a_THD",
                                                                                                 thdSlider);
    
    addAndMakeVisible(cellingSlider);
    cellingSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    cellingSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 75, 20);
    cellingSlider.setColour(juce::Slider::textBoxOutlineColourId, Colour(0,0,0));
    
    cellingSlider.setTextValueSuffix(" dB");
    cellingSlider.setTextBoxIsEditable(true);

    cellingSlider.setColour (juce::Slider::thumbColourId, juce::Colours::darkviolet);
    cellingSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::darkviolet);
    cellingSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    cellingSlider.setDoubleClickReturnValue (true, 1.4);
    cellingSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,
                                                                                                   "b_CELLING",
                                                                                                   cellingSlider);
    
    
    addAndMakeVisible(atkSlider);
    atkSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    atkSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 75, 20);
    atkSlider.setColour(juce::Slider::textBoxOutlineColourId, Colour(0,0,0));

    atkSlider.setTextValueSuffix(" ms");
    atkSlider.setTextBoxIsEditable(true);
    
    atkSlider.setColour (juce::Slider::thumbColourId, juce::Colours::darkviolet);
    atkSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::darkviolet);
    atkSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    atkSlider.setDoubleClickReturnValue (true, 10);
    atkSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,
                                                                                                 "d_ATK",
                                                                                                 atkSlider);
    
    addAndMakeVisible(relSlider);
    relSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    relSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 75, 20);
    relSlider.setColour(juce::Slider::textBoxOutlineColourId, Colour(0,0,0));

    relSlider.setTextValueSuffix(" ms");
    relSlider.setTextBoxIsEditable(true);
    
    relSlider.setColour (juce::Slider::thumbColourId, juce::Colours::darkviolet);
    relSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::darkviolet);
    relSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    relSlider.setDoubleClickReturnValue (true, 80);
    relSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,
                                                                                                 "e_REL",
                                                                                                 relSlider);
    
    addAndMakeVisible(rel2Slider);
    rel2Slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    rel2Slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 75, 20);
    rel2Slider.setColour(juce::Slider::textBoxOutlineColourId, Colour(0,0,0));

    rel2Slider.setTextValueSuffix(" ms");
    rel2Slider.setTextBoxIsEditable(true);
    
    rel2Slider.setColour (juce::Slider::thumbColourId, juce::Colours::darkviolet);
    rel2Slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::darkviolet);
    rel2Slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    rel2Slider.setDoubleClickReturnValue (true, 50);
    rel2SliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,
                                                                                                   "f_REL2",
                                                                                                  rel2Slider);
    
    addAndMakeVisible(ditherSlider);
    ditherSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    ditherSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 75, 20);
    ditherSlider.setColour(juce::Slider::textBoxOutlineColourId, Colour(0,0,0));

    ditherSlider.setColour (juce::Slider::thumbColourId, juce::Colours::darkviolet);
    ditherSlider.setColour(juce::Slider::trackColourId, juce::Colours::darkviolet);
    ditherSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::whitesmoke);
    ditherSlider.setDoubleClickReturnValue (true, 0);
    ditherSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,
                                                                                                    "g_DITHER",
                                                                                                    ditherSlider);
    
    addAndMakeVisible(unitySlider);
    unitySlider.setSliderStyle (juce::Slider::LinearHorizontal);
    unitySlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 75, 20);
    unitySlider.setColour(juce::Slider::textBoxOutlineColourId, Colour(0,0,0));
    
    unitySlider.addListener(this);


    unitySlider.setColour (juce::Slider::thumbColourId, juce::Colours::darkviolet);
    unitySlider.setColour(juce::Slider::trackColourId, juce::Colours::darkviolet);
    unitySlider.setColour(juce::Slider::backgroundColourId, juce::Colours::whitesmoke);
    unitySlider.setDoubleClickReturnValue (true, 0);
    unitySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,
                                                                                                   "i_UNITY",
                                                                                                   unitySlider);
    
    
    addAndMakeVisible(bypassSlider);
    bypassSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bypassSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 75, 20);
    bypassSlider.setColour(juce::Slider::textBoxOutlineColourId, Colour(0,0,0));

    bypassSlider.setColour (juce::Slider::thumbColourId, juce::Colours::darkviolet);
    bypassSlider.setColour(juce::Slider::trackColourId, juce::Colours::darkviolet);
    bypassSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::whitesmoke);
    bypassSlider.setDoubleClickReturnValue (true, 0);
    bypassSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,
                                                                                                    "h_BYPASS",
                                                                                                    bypassSlider);
    

    
    // Labels
    addAndMakeVisible(labelThd);
    labelThd.setText(juce::CharPointer_UTF8("THD"), juce::dontSendNotification);
    labelThd.attachToComponent(&thdSlider, false);
    labelThd.setJustificationType(juce::Justification(36));
    
    addAndMakeVisible(labelCelling);
    labelCelling.setText(juce::CharPointer_UTF8("CELLING"), juce::dontSendNotification);
    labelCelling.attachToComponent(&cellingSlider, false);
    labelCelling.setJustificationType(juce::Justification(36));

    
    addAndMakeVisible(labelAtk);
    labelAtk.setText(juce::CharPointer_UTF8("ATK"), juce::dontSendNotification);
    labelAtk.attachToComponent(&atkSlider, false);
    labelAtk.setJustificationType(juce::Justification(36));
    
    addAndMakeVisible(labelRel);
    labelRel.setText(juce::CharPointer_UTF8("REL"), juce::dontSendNotification);
    labelRel.attachToComponent(&relSlider, false);
    labelRel.setJustificationType(juce::Justification(36));
    
    addAndMakeVisible(labelRel2);
    labelRel2.setText(juce::CharPointer_UTF8("REL2"), juce::dontSendNotification);
    labelRel2.attachToComponent(&rel2Slider, false);
    labelRel2.setJustificationType(juce::Justification(36));
    
    addAndMakeVisible(labelDither);
    labelDither.setText(juce::CharPointer_UTF8("DITHER 16"), juce::dontSendNotification);
    labelDither.attachToComponent(&ditherSlider, false);
    labelDither.setJustificationType(juce::Justification(36));
    
    addAndMakeVisible(labelUnity);
    labelUnity.setText(juce::CharPointer_UTF8("UNITY GAIN"), juce::dontSendNotification);
    labelUnity.attachToComponent(&unitySlider, false);
    labelUnity.setJustificationType(juce::Justification(36));

    addAndMakeVisible(labelBypass);
    labelBypass.setText(juce::CharPointer_UTF8("BYPASS"), juce::dontSendNotification);
    labelBypass.attachToComponent(&bypassSlider, false);
    labelBypass.setJustificationType(juce::Justification(36));
    
    // Tamaño reajustable
    auto size = processor.getSavedSize();
    setResizable(true, true);
    setSize(size.x, size.y);
    setResizeLimits(1050, 350, ancho*2.75, alto*2.75);
    getConstrainer()->setFixedAspectRatio(ancho/alto);
}

JCBMaximizerAudioProcessorEditor::~JCBMaximizerAudioProcessorEditor()
{
}

//==============================================================================
void JCBMaximizerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void JCBMaximizerAudioProcessorEditor::resized()
{
    // MAXI
    
    auto correctX = 0;
    auto correctY = 0;
    
    auto xThd = (correctX+getWidth())    * 285/ancho;
    auto yThd = (correctY+getHeight())   * 50/alto;
    auto wThd = (correctX+getWidth())    * 130/ancho;
    auto hThd = (correctY+getHeight())   * 130/alto;
    
//    auto xRatio = getWidth()  * 110/ancho;
//    auto yRatio = getHeight() * 70/alto;
//    auto wRatio = getWidth()  * 80/ancho;
//    auto hRatio = getHeight() * 80/alto;
    
//    auto xCelling= getWidth()   * 210/ancho;
//    auto yCelling= getHeight()  * 70/alto;
//    auto wCelling= getWidth()   * 80/ancho;
//    auto hCelling= getHeight()  * 80/alto;
    
    auto xCelling= (correctX+getWidth())   * 70/ancho;
    auto yCelling= (correctY+getHeight())  * 70/alto;
    auto wCelling= (correctX+getWidth())   * 80/ancho;
    auto hCelling= (correctY+getHeight())  * 80/alto;
    
//    auto xAtk = getWidth()    * 310/ancho;
//    auto yAtk = getHeight()   * 70/alto;
//    auto wAtk = getWidth()    * 80/ancho;
//    auto hAtk = getHeight()   * 80/alto;
    
    auto xAtk = (correctX+getWidth())    * 170/ancho;
    auto yAtk = (correctY+getHeight())   * 70/alto;
    auto wAtk = (correctX+getWidth())    * 80/ancho;
    auto hAtk = (correctY+getHeight())   * 80/alto;
    
    auto xRel = (correctX+getWidth())    * 450/ancho;
    auto yRel = (correctY+getHeight())   * 70/alto;
    auto wRel = (correctX+getWidth())    * 80/ancho;
    auto hRel = (correctY+getHeight())   * 80/alto;
    
    auto xRel2 = (correctX+getWidth())   * 550/ancho;
    auto yRel2 = (correctY+getHeight())  * 70/alto;
    auto wRel2 = (correctX+getWidth())   * 80/ancho;
    auto hRel2 = (correctY+getHeight())  * 80/alto;
    
//    auto xDWExp = getWidth()  * 610/ancho;
//    auto yDWExp = getHeight() * 70/alto;
//    auto wDWExp = getWidth()  * 80/ancho;
//    auto hDWExp = getHeight() * 80/alto;
    
    // Horizontal Sliders
    
    auto xBypa =  (correctX+getWidth())  * 326/ancho;
    auto yBypa =  (correctY+getHeight()) * 20/alto;
    auto wBypa =  (correctX+getWidth())  * 50/ancho;
    auto hBypa =  (correctY+getHeight()) * 15/alto;
    
    auto xDither =  (correctX+getWidth())  * 385/ancho;
    auto yDither =  (correctY+getHeight()) * 25/alto;
    auto wDither =  (correctX+getWidth())  * 50/ancho;
    auto hDither =  (correctY+getHeight()) * 15/alto;
    
    auto xUnity =  (correctX+getWidth())  * 265/ancho;
    auto yUnity =  (correctY+getHeight()) * 25/alto;
    auto wUnity =  (correctX+getWidth())  * 50/ancho;
    auto hUnity =  (correctY+getHeight()) * 15/alto;
    
    
    // Side Chain Sliders
    
//    auto xHPF = getWidth()    * 260/ancho;
//    auto yHPF = getHeight()    * 10/alto;
//    auto wHPF = getWidth()    * 50/ancho;
//    auto hHPF = getHeight()    * 50/alto;
//
//    auto xLPF = getWidth()    * 385/ancho;
//    auto yLPF = getHeight()    * 10/alto;
//    auto wLPF = getWidth()    * 50/ancho;
//    auto hLPF = getHeight()    * 50/alto;
    
//    auto xOnLPF = getWidth()  * 270/ancho;
//    auto yOnLPF = getHeight()  * 3/alto;
//    auto wOnLPF = getWidth()  * 30/ancho;
//    auto hOnLPF = getHeight()  * 10/alto;
//
//    auto xOnHPF = getWidth()  * 395/ancho;
//    auto yOnHPF = getHeight()  * 3/alto;
//    auto wOnHPF = getWidth()  * 30/ancho;
//    auto hOnHPF = getHeight()  * 10/alto;
    
    // URL
    auto xUrl   = (correctX+getWidth())  * 480/ancho;
    auto yUrl   = (correctY+getHeight()) * 170/alto;
    auto wUrl   = (correctX+getHeight()) * 600/ancho;
    auto hUrl   = (correctY+getHeight()) * 25/alto;
    
    // set Bounds
    processor.setSavedSize( {getWidth(), getHeight()} );
    mFondo.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    
    thdSlider.setBounds       (xThd, yThd, wThd, hThd);
    cellingSlider.setBounds   (xCelling, yCelling, wCelling, hCelling);
    //rangeSlider.setBounds     (xRange, yRange, wRange, hRange);

    atkSlider.setBounds       (xAtk, yAtk, wAtk, hAtk);
    relSlider.setBounds       (xRel, yRel, wRel, hRel);
    rel2Slider.setBounds      (xRel2, yRel2, wRel2, hRel2);
    
//    drywetSlider.setBounds    (xDWExp, yDWExp, wDWExp, hDWExp);
    bypassSlider.setBounds    (xBypa, yBypa, wBypa, hBypa);
    ditherSlider.setBounds    (xDither, yDither, wDither, hDither);
    
    unitySlider.setBounds     (xUnity, yUnity, wUnity, hUnity);
    
//    hpfSlider.setBounds       (xHPF, yHPF, wHPF, hHPF);
//    lpfSlider.setBounds       (xLPF, yLPF, wLPF, hLPF);
//    onLoSlider.setBounds      (xOnLPF, yOnLPF, wOnLPF, hOnLPF);
//    onHiSlider.setBounds      (xOnHPF, yOnHPF, wOnHPF, hOnHPF);
    
    titleLink.setBounds       (xUrl, yUrl, wUrl, hUrl);
    
//    txtExtra.setFontHeight (getHeight()/8.f);
//    txtExtra.setBoundingBox(juce::Parallelogram<float>(juce::Rectangle<float>(getWidth()/2.150f,
//                                                                                  getHeight()/9.f,
//                                                                                  getWidth(),
//                                                                                  getHeight()/23.f)));

}

void JCBMaximizerAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    
    if (slider == &unitySlider && slider->getValue() == 1)
    {
        unitySlider.setColour(juce::Slider::trackColourId, juce::Colours::red);

        thdSlider.setColour (juce::Slider::thumbColourId, juce::Colours::whitesmoke);
        thdSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red);
        thdSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::red);
        
    } else if (slider == &unitySlider && slider->getValue() == 0)
    {
        
        unitySlider.setColour(juce::Slider::trackColourId, juce::Colours::darkviolet);

        thdSlider.setColour (juce::Slider::thumbColourId, juce::Colours::darkviolet);
        thdSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::darkviolet);
        thdSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    }
}
