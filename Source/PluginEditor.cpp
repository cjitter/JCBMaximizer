//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBMaximizer and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
// Módulos JUCE
#include <juce_graphics/juce_graphics.h>

// Librerías estándar C++
#include <mutex>
#include <algorithm>

// Archivos del proyecto
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Components/Windows/CreditsWindow.h"
#include "Components/Windows/CodeWindow.h"
#include "Helpers/UTF8Helper.h"
#include "BinaryData.h"

//==============================================================================
// CONSTRUCTOR Y DESTRUCTOR
//==============================================================================
JCBMaximizerAudioProcessorEditor::JCBMaximizerAudioProcessorEditor (JCBMaximizerAudioProcessor& p, juce::UndoManager& um)
    : AudioProcessorEditor (&p), 
      processor (p), 
      undoManager (um),
      // CRASH FIX: Initialize with safe dummy functions, configure real ones later
      inputMeterL([]() { return -100.0f; }),     // Safe dummy value for input meters
      inputMeterR([]() { return -100.0f; }),     // Safe dummy value for input meters  
      grMeter([]() { return 0.0f; }),            // Safe dummy value for gain reduction
      outputMeterL([]() { return -100.0f; }),    // Safe dummy value for output meters
      outputMeterR([]() { return -100.0f; })
{
    // Configurar todos los componentes
    setupBackground();
    setupKnobs();
    setupMeters();
    setupPresetArea();
    setupUtilityButtons();
    setupParameterButtons();
    
    // Agregar display principal
    addAndMakeVisible(transferDisplay);
    
    // Agregar título y versión
    auto titleFont = juce::Font(juce::FontOptions(22.0f));
    titleLink.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    titleLink.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    titleLink.setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.withAlpha(0.7f));
    titleLink.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    
    // Detectar si estamos en Logic Pro para mostrar solo la versión
    juce::String titleText;
    
    // Verificar si el host es Logic Pro
    juce::PluginHostType hostInfo;
    if (hostInfo.isLogic()) {
        titleText = "v1.0.0";  // Solo versión para Logic Pro
    } else {
        titleText = "JCBMaximizer v1.0.0";  // Nombre completo para otros DAWs
    }
    
    titleLink.setButtonText(titleText);
    
    // El tooltip se actualiza dinámicamente en updateTooltips() usando getTooltipText("title")
    titleLink.setTooltip("");
    
    // Configurar el click para mostrar créditos
    titleLink.onClick = [this]() {
        showCredits();
    };
    
    addAndMakeVisible(titleLink);

    // Agregar tooltip
    addAndMakeVisible(tooltipComponent);
    tooltipComponent.setAlwaysOnTop(true);
    tooltipComponent.showTip(JUCE_UTF8("Pasa el mouse sobre los controles\npara ver ayuda detallada"));

    // Establecer tamaño inicial con restricciones
    auto savedSize = processor.getSavedSize();
    int initialWidth = savedSize.x > 0 ? savedSize.x : DEFAULT_WIDTH;
    int initialHeight = savedSize.y > 0 ? savedSize.y : DEFAULT_HEIGHT;
    
    setSize(initialWidth, initialHeight);
    setResizable(true, true);
    
    // Inicializar el estado del tamaño basado en las dimensiones iniciales
    if (initialWidth == DEFAULT_WIDTH && initialHeight == DEFAULT_HEIGHT) {
        currentSizeState = GuiSizeState::Current;
    } else if (initialWidth == MIN_WIDTH && initialHeight == MIN_HEIGHT) {
        currentSizeState = GuiSizeState::Minimum;
    } else if (initialWidth == MAX_WIDTH && initialHeight == MAX_HEIGHT) {
        currentSizeState = GuiSizeState::Maximum;
    } else {
        // Para tamaños personalizados, usar estado Current por defecto
        currentSizeState = GuiSizeState::Current;
        lastCustomSize = {initialWidth, initialHeight};
    }

    // Establecer límites de redimensionado
    setResizeLimits(MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
    getConstrainer()->setFixedAspectRatio(ASPECT_RATIO);
    
    // Inicializar cache de contenido de código para thread safety
    initializeCodeContentCache();
    
    // Timer para resetear indicadores de clip cada cierto tiempo
    clipResetCounter = 0;
    
    // Inicializar transfer display con valores actuales
    float thresholdDB = leftTopKnobs.thdSlider.getValue();
    transferDisplay.setThreshold(thresholdDB);
    
    // Inicializar ceiling desde parámetro b_CELLING
    if (auto* ceilingParam = processor.apvts.getRawParameterValue("b_CELLING")) {
        transferDisplay.setCeiling(ceilingParam->load());
    }

    // Conectar callbacks del transfer display para actualizar knobs
    transferDisplay.onThresholdChange = [this](float newGain) {
        leftTopKnobs.thdSlider.setValue(newGain, juce::sendNotification);
        handleParameterChange();  // Marcar preset como modificado
    };
    transferDisplay.onCeilingChange = [this](float newCeiling) {
        leftTopKnobs.ceilingSlider.setValue(newCeiling, juce::sendNotification);
        handleParameterChange();  // Marcar preset como modificado
    };
    
    // Updates iniciales
    updateTransferDisplay();
    
    // Crear y registrar parameter listener para updates de automatización
    transferFunctionListener = std::make_unique<TransferFunctionParameterListener>(this);
    processor.apvts.addParameterListener("a_GAIN", transferFunctionListener.get());
    processor.apvts.addParameterListener("b_CELLING", transferFunctionListener.get());
    // Crear y registrar parameter listener para alpha del REL slider
    autorelParameterListener = std::make_unique<AutorelParameterListener>(this);
    processor.apvts.addParameterListener("m_AUTOREL", autorelParameterListener.get());
    
    // Configurar estado inicial del idioma
    if (processor.getTooltipLanguageEnglish()) {
        currentLanguage = TooltipLanguage::English;
        utilityButtons.tooltipLangButton.setButtonText("eng");
        utilityButtons.tooltipLangButton.setColour(juce::TextButton::textColourOffId, DarkTheme::accentSecondary);
    } else {
        currentLanguage = TooltipLanguage::Spanish;
        utilityButtons.tooltipLangButton.setButtonText("esp");
        utilityButtons.tooltipLangButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    }

    // Actualizar todos los tooltips basado en idioma inicial
    updateAllTooltips();

    // Configurar estados iniciales
    bool initialDeltaState = processor.apvts.getRawParameterValue("k_DELTA")->load() > 0.5f;
    parameterButtons.deltaButton.setButtonText("DELTA");
    
    // REFACTORIZADO: Usar método centralizado para aplicar estado DELTA inicial
    if (initialDeltaState) {
        applyDeltaModeToAllControls(true);
    }
    
    // Registrar listener para sincronización automática de estado DELTA
    deltaParameterListener = std::make_unique<DeltaParameterListener>(this);
    processor.apvts.addParameterListener("k_DELTA", deltaParameterListener.get());
    
    updateButtonStates();
    updateFilterButtonText();  // Establecer texto inicial de botones de filtro

    // Configurar estado inicial del botón run graphics basado en el processor
    bool initialEnvelopeState = processor.getEnvelopeVisualEnabled();
    // Graphics ON = envolventes VISIBLES (mostrar formas de onda en tiempo real)
    // Graphics OFF = envolventes OCULTAS (mejor rendimiento)
    bool graphicsButtonState = initialEnvelopeState;  // Directo, sin inversión
    utilityButtons.runGraphicsButton.setToggleState(graphicsButtonState, juce::dontSendNotification);
    utilityButtons.runGraphicsButton.setButtonText("graphics");
    transferDisplay.setEnvelopeVisible(initialEnvelopeState);
    // Configurar visibilidad inicial del grMeter (visible cuando envolventes están visibles)
    grMeter.setVisible(initialEnvelopeState);
    // Forzar repaint inicial para asegurar consistencia visual
    transferDisplay.repaint();
    grMeter.repaint();
    
    // Actualizar valores de sliders desde APVTS para evitar problemas al cargar sesión
    // Usar MessageManager::callAsync para ejecución thread-safe sin delay
    juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<JCBMaximizerAudioProcessorEditor>(this)]() {
        if (safeThis) {
            safeThis->updateSliderValues();
        }
    });
    
    // Configurar estado inicial del tooltip toggle y language button
    bool initialTooltipState = processor.getTooltipEnabled();
    tooltipsEnabled = initialTooltipState;
    tooltipComponent.setTooltipsEnabled(tooltipsEnabled);
    utilityButtons.tooltipToggleButton.setToggleState(tooltipsEnabled, juce::dontSendNotification);
    
    // Habilitar/deshabilitar botón de idioma según estado de tooltips
    utilityButtons.tooltipLangButton.setEnabled(tooltipsEnabled);
    utilityButtons.tooltipLangButton.setAlpha(tooltipsEnabled ? 1.0f : 0.25f);
    
    // Aplicar estados iniciales de todos los modos en el orden correcto
    updateButtonStates();

    // Inicializar texto del botón AR basado en el estado actual del parámetro
    
    // CRASH FIX: Configurar las funciones reales de los medidores DESPUÉS de toda la inicialización
    // Esto evita accesos prematuros a valores atómicos del processor durante la construcción
    inputMeterL.setValueFunction([this](){ return processor.isInitialized() ? processor.getRmsInputValue(0) : -100.0f; });
    inputMeterR.setValueFunction([this](){ return processor.isInitialized() ? processor.getRmsInputValue(1) : -100.0f; });
    grMeter.setValueFunction([this](){ return processor.isInitialized() ? processor.getGainReductionValue(0) : 0.0f; });
    outputMeterL.setValueFunction([this](){ return processor.isInitialized() ? processor.getRmsOutputValue(0) : -100.0f; });
    outputMeterR.setValueFunction([this](){ return processor.isInitialized() ? processor.getRmsOutputValue(1) : -100.0f; });
    
    // CRASH FIX: Iniciar timer AL FINAL para evitar acceso prematuro a valores atómicos
    // El timer debe iniciarse después de que todo esté completamente inicializado
    startTimerHz(TIMER_HZ);
}

JCBMaximizerAudioProcessorEditor::~JCBMaximizerAudioProcessorEditor()
{
    // 1. CRÍTICO: Detener timer PRIMERO para prevenir crashes durante destrucción
    stopTimer();
    
    // 2. Eliminar parameter listeners
    if (deltaParameterListener)
    {
        processor.apvts.removeParameterListener("k_DELTA", deltaParameterListener.get());
    }
    
    if (transferFunctionListener)
    {
        processor.apvts.removeParameterListener("a_GAIN", transferFunctionListener.get());
        processor.apvts.removeParameterListener("b_CELLING", transferFunctionListener.get());
    }
    
    if (autorelParameterListener)
    {
        processor.apvts.removeParameterListener("m_AUTOREL", autorelParameterListener.get());
    }
    
    // 3. CRÍTICO: Limpiar LookAndFeels de TODOS los componentes para evitar assertion
    // Sliders con LookAndFeel personalizado
    leftTopKnobs.thdSlider.setLookAndFeel(nullptr);
    leftTopKnobs.ceilingSlider.setLookAndFeel(nullptr);
    rightTopControls.lookaheadSlider.setLookAndFeel(nullptr);
    rightTopControls.detSlider.setLookAndFeel(nullptr);
    rightBottomKnobs.atkSlider.setLookAndFeel(nullptr);
    rightBottomKnobs.relSlider.setLookAndFeel(nullptr);
    
    // Botones con LookAndFeel personalizado
    rightBottomKnobs.ditherButton.setLookAndFeel(nullptr);
    rightBottomKnobs.dcFilterButton.setLookAndFeel(nullptr);
    rightBottomKnobs.autorelButton.setLookAndFeel(nullptr);
    
    // 4. Limpiar LookAndFeel del editor principal
    setLookAndFeel(nullptr);
}

//==============================================================================
// OVERRIDES DE JUCE  
//==============================================================================
void JCBMaximizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // El background se maneja por el componente backgroundImage
}

void JCBMaximizerAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    // Usar las mismas coordenadas que la transfer function display
    float x = 460.0f * 700.0f / 1247.0f;  // ≈ 258
    float y = 73.0f * 200.0f / 353.0f + 4.0f;    // ≈ 41 + 4 píxeles
    float w = 335.0f * 700.0f / 1247.0f;  // ≈ 188
    float h = 205.0f * 200.0f / 353.0f - 5.0f;   // ≈ 116 - 5 píxeles
    auto transferBounds = getScaledBounds(x, y, w, h).toFloat();
    
    // Texto BYPASS
    if (isBypassed && bypassTextVisible) {
        g.setColour(juce::Colours::transparentWhite.withAlpha(0.85f));
        g.setFont(juce::Font(juce::FontOptions(transferBounds.getHeight() * 0.4f))
                  .withStyle(juce::Font::bold));
        g.drawText("BYPASS", transferBounds, juce::Justification::centred);
    }
    
    // Texto DELTA (cuando está activo)
    if (parameterButtons.deltaButton.getToggleState()) {
        g.setColour(juce::Colours::transparentWhite.withAlpha(0.85f));
        g.setFont(juce::Font(juce::FontOptions(transferBounds.getHeight() * 0.4f))
                  .withStyle(juce::Font::bold));
        g.drawText("DELTA", transferBounds, juce::Justification::centred);
    }
}

void JCBMaximizerAudioProcessorEditor::resized()
{
    // Guardar tamaño de ventana
    processor.setSavedSize({getWidth(), getHeight()});
    
    // Si el usuario redimensiona manualmente (no es uno de los tamaños predefinidos),
    // guardar como tamaño personalizado para poder volver a él
    int currentWidth = getWidth();
    int currentHeight = getHeight();

    if (currentWidth != DEFAULT_WIDTH && currentWidth != MIN_WIDTH && currentWidth != MAX_WIDTH &&
        currentHeight != DEFAULT_HEIGHT && currentHeight != MIN_HEIGHT && currentHeight != MAX_HEIGHT) {
        // Es un tamaño personalizado - guardarlo
        lastCustomSize = {currentWidth, currentHeight};
    }
    
    // El background llena toda el área
    backgroundImage.setBounds(getLocalBounds());
    
    // === METERS CON SLIDERS DE TRIM/MAKEUP ===
    // Meters de entrada (lado izquierdo)
    inputMeterL.setBounds(getScaledBounds(2, 42, 12, 117));
    inputMeterR.setBounds(getScaledBounds(12, 42, 12, 117));
    
    // scMeterL.setBounds(getScaledBounds(24, 42, 12, 117));
    // scMeterR.setBounds(getScaledBounds(34, 42, 12, 117));
    
    // Sliders de trim superpuestos a los meters
    trimSlider.setBounds(getScaledBounds(2, 40, 22, 130));  // Altura expandida para TextBox integrado

    // Medidor GR (centro-derecha) - más delgado y alto, posición final ajustada
    grMeter.setBounds(getScaledBounds(437, 48, 5, 105));
    
    // Medidores de salida (lado derecho)
    outputMeterL.setBounds(getScaledBounds(677, 42, 12, 117));
    outputMeterR.setBounds(getScaledBounds(687, 42, 12, 117));
    
    // Sliders de makeup superpuestos a los medidores de salida
    makeupSlider.setBounds(getScaledBounds(677, 40, 22, 130));  // Centrado sobre outputMeterL y outputMeterR
    
    // === TRANSFER FUNCTION DISPLAY (CENTER) ===
    float x = 460.0f * 700.0f / 1247.0f;
    float y = 73.0f * 200.0f / 353.0f + 4.0f;
    float w = 335.0f * 700.0f / 1247.0f;
    float h = 205.0f * 200.0f / 353.0f - 5.0f;
    transferDisplay.setBounds(getScaledBounds(x, y, w, h));
    
    // === PARAMETER BUTTONS (ENCIMA DE TRANSFER FUNCTION) ===
    // Botones DC FILTER y DITHER centrados sobre transfer function
    // Transfer function centrada en X=387, colocar botones centrados
    rightBottomKnobs.dcFilterButton.setBounds(getScaledBounds(300, 15, 50, 15));  // DC FILTER centrado
    rightBottomKnobs.ditherButton.setBounds(getScaledBounds(360, 15, 40, 15));    // DITHER a la derecha
    
    // === LEFT SIDE KNOBS === (Between SC meters and transfer function)
    // Top row - THD, CEILING (MAXIMIZER-specific parameters)
    leftTopKnobs.thdSlider.setBounds(getScaledBounds(35, 75, 70, 70));
    leftTopKnobs.ceilingSlider.setBounds(getScaledBounds(160, 50, 70, 70
        ));  // NUEVO - b_CELLING slider

    // === RIGHT SIDE CONTROLS ===
    // NUEVO: DET knob - área derecha superior
    rightTopControls.detSlider.setBounds(getScaledBounds(505, 48, 53, 53));
    
    // MOVIDO: LOOKAHEAD knob - centro de la parte derecha superior 
    rightTopControls.lookaheadSlider.setBounds(getScaledBounds(560, 48, 53, 53));

    // Bottom row - Attack, Release, Hold
    rightBottomKnobs.atkSlider.setBounds(getScaledBounds(473, 100, 53, 53));
    rightBottomKnobs.relSlider.setBounds(getScaledBounds(592, 100, 53, 53));
    
    // NUEVO: AUTOREL button - área derecha inferior, junto al REL
    rightBottomKnobs.autorelButton.setBounds(getScaledBounds(538, 117, 42, 16));

    // === PRESET AREA (TOP LEFT) ===
    presetArea.saveButton.setBounds(getScaledBounds(5, 15, 20, 12));  // Desplazado 10px a la izquierda
    presetArea.saveAsButton.setBounds(getScaledBounds(27, 15, 25, 12));
    presetArea.deleteButton.setBounds(getScaledBounds(54, 15, 25, 12));
    presetArea.backButton.setBounds(getScaledBounds(81, 15, 14, 12));  // Reducido de 18 a 14px
    presetArea.nextButton.setBounds(getScaledBounds(98, 15, 14, 12));  // Reducido de 18 a 14px
    presetArea.presetMenu.setBounds(getScaledBounds(114, 15, 95, 12));  // Ampliado de 65 a 95px
    
    // Botones A/B junto al menú de preset
    topButtons.abStateButton.setBounds(getScaledBounds(213, 15, 18, 12));
    topButtons.abCopyButton.setBounds(getScaledBounds(233, 15, 22, 12));
    
    // === BOTONES DE UTILIDAD (INFERIOR IZQUIERDA) ===
    utilityButtons.undoButton.setBounds(getScaledBounds(30, 175, 22, 12));
    utilityButtons.redoButton.setBounds(getScaledBounds(52, 175, 22, 12));
    utilityButtons.resetGuiButton.setBounds(getScaledBounds(76, 175, 30, 12));
    utilityButtons.runGraphicsButton.setBounds(getScaledBounds(108, 175, 30, 12));
    utilityButtons.zoomButton.setBounds(getScaledBounds(140, 175, 30, 12));
    // Mover botones de tooltip y lenguaje donde estaba el diagram
    utilityButtons.tooltipToggleButton.setBounds(getScaledBounds(172, 175, 30, 12));
    utilityButtons.tooltipLangButton.setBounds(getScaledBounds(204, 175, 22, 12));
    
    // Botones DELTA, DIAGRAM y BYPASS - en la parte inferior (como en JCBCompressor)
    const int centerButtonsY = 163;
    const int diagramCenterX = 355;
    const int buttonSpacing = 45;   // Espaciado entre DELTA-DIAGRAM y DIAGRAM-BYPASS
    
    // DIAGRAM centrado
    centerButtons.diagramButton.setBounds(getScaledBounds(diagramCenterX - 22, centerButtonsY, 44, 12));
    // DELTA a la izquierda de DIAGRAM
    parameterButtons.deltaButton.setBounds(getScaledBounds(diagramCenterX - buttonSpacing - 20, centerButtonsY, 40, 12));
    // BYPASS a la derecha de DIAGRAM
    parameterButtons.bypassButton.setBounds(getScaledBounds(diagramCenterX + buttonSpacing - 20, centerButtonsY, 40, 12));
    
    // Botones TODO movidos abajo a la derecha, centrados en el rectángulo
    // Calcular posición central para el grupo de botones TODO
    const int todoStartX = 503;    // Movido a la derecha para mejor centrado en el rectángulo
    const int todoY = 175;       // Mismo Y que botones de utilidad
    utilityButtons.hqButton.setBounds(getScaledBounds(todoStartX, todoY, 18, 12));
    utilityButtons.dualMonoButton.setBounds(getScaledBounds(todoStartX + 20, todoY, 23, 12));
    utilityButtons.stereoLinkedButton.setBounds(getScaledBounds(todoStartX + 45, todoY, 23, 12));
    utilityButtons.msButton.setBounds(getScaledBounds(todoStartX + 70, todoY, 18, 12));
    // Botón MIDI posicionado a la derecha del botón M/S
    utilityButtons.midiLearnButton.setBounds(getScaledBounds(todoStartX + 90, todoY, 23, 12));
    
    // === TÍTULO Y VERSIÓN (CENTRO INFERIOR) ===
    // Ancho similar a botones DIAGRAM + GEN DSP combinados
    titleLink.setBounds(getScaledBounds(304, 179, 95, 15));
    
    // Tooltip en esquina superior derecha - ajustado al rectángulo visible
    tooltipComponent.setBounds(getScaledBounds(450, 0, 228, 42));
    
    // Actualizar factor de escala del tooltip basado en el tamaño de la ventana
    float scaleFactor = (float)getWidth() / (float)DEFAULT_WIDTH;
    tooltipComponent.setScaleFactor(scaleFactor);
    
    // Redimensionar el overlay del diagrama si está visible
    if (diagramOverlay != nullptr)
    {
        diagramOverlay->setBounds(getLocalBounds());
        diagramOverlay->invalidateClickableAreasCache(); // Invalidar cache para regenerar con nuevas coordenadas escaladas
    }
    
    // Redimensionar el overlay de créditos si está visible
    if (creditsOverlay != nullptr)
    {
        creditsOverlay->setBounds(getLocalBounds());
    }
    
    // Redimensionar y reposicionar la ventana de código si está visible
    if (codeWindow != nullptr && codeWindow->isVisible())
    {
        // Recalcular tamaño responsivo
        int windowWidth = static_cast<int>(getWidth() * 0.35f);
        int windowHeight = static_cast<int>(getHeight() * 0.50f);
        
        // Limitar tamaños
        windowWidth = juce::jlimit(350, 600, windowWidth);
        windowHeight = juce::jlimit(250, 450, windowHeight);
        
        // Recentrar la ventana
        int x = getWidth() / 2 - windowWidth / 2;
        int y = getHeight() / 2 - windowHeight / 2;
        
        codeWindow->setBounds(x, y, windowWidth, windowHeight);
    }
}

void JCBMaximizerAudioProcessorEditor::timerCallback()
{
    // CRASH FIX: Verificar que el processor esté completamente inicializado
    // antes de acceder a valores atómicos
    if (!processor.isInitialized()) {
        return;
    }
    
    // Sistema universal de decay para todos los DAWs
    applyMeterDecayIfNeeded();
    
    updateMeters();
    
    // Actualizar estado visual de botones undo/redo
    bool canUndo = undoManager.canUndo();
    bool canRedo = undoManager.canRedo();
    
    // Actualizar apariencia del botón undo
    utilityButtons.undoButton.setEnabled(canUndo);
    utilityButtons.undoButton.setAlpha(canUndo ? 1.0f : 0.3f);
    utilityButtons.undoButton.setColour(juce::TextButton::textColourOffId, 
                                       canUndo ? DarkTheme::textPrimary : DarkTheme::textSecondary);
    
    // Actualizar apariencia del botón redo
    utilityButtons.redoButton.setEnabled(canRedo);
    utilityButtons.redoButton.setAlpha(canRedo ? 1.0f : 0.3f);
    utilityButtons.redoButton.setColour(juce::TextButton::textColourOffId, 
                                       canRedo ? DarkTheme::textPrimary : DarkTheme::textSecondary);
    
    // Actualizar texto y tooltip del botón A/B copy dinámicamente
    if (processor.getIsStateA()) {
        topButtons.abCopyButton.setButtonText("A-B");
        topButtons.abCopyButton.setTooltip(getTooltipText("abcopyatob"));
    } else {
        topButtons.abCopyButton.setButtonText("B-A");
        topButtons.abCopyButton.setTooltip(getTooltipText("abcopybtoa"));
    }
    
    // Resetear indicadores de clip cada 3 segundos (180 frames a 60Hz)
    clipResetCounter++;
    if (clipResetCounter >= 180) {
        processor.resetClipIndicators();
        clipResetCounter = 0;
    }

    // Pasar el estado de Logic parado al display
    // Determinar si el procesamiento está inactivo
    // Usar sistema híbrido optimizado (timestamp + playhead + audio tail)
    bool isProcessingInactive = !processor.isPlaybackActive();
    
    // Sistema original comentado en processBlock:
    // bool isProcessingInactive = processor.getIsLogicStopped();
    transferDisplay.setLogicStoppedState(isProcessingInactive);
    
    // Actualizar datos de waveform y obtener gain reduction para el medidor
    if (!isBypassed && !isProcessingInactive)
    {
        // Obtener datos de waveform si las envolventes están visibles O si estamos en modo DELTA
        bool deltaActive = parameterButtons.deltaButton.getToggleState();
        if (transferDisplay.isEnvelopeVisible() || deltaActive)
        {
            std::vector<float> inputSamples, processedSamples, gainReductionSamples;
            processor.getWaveformDataWithGR(inputSamples, processedSamples, gainReductionSamples);
            
            if (!gainReductionSamples.empty())
            {
                // Encontrar la mayor reducción de ganancia en el buffer para el medidor GR
                // Los valores vienen en dB: negativos = reducción, positivos = amplificación
                float maxReduction = 0.0f;  // Start with no reduction
                for (float grDb : gainReductionSamples) {
                    // For expander, find most negative value (greatest reduction)
                    if (grDb < maxReduction) {
                        maxReduction = grDb;
                    }
                }
                maxGainReductionFromBuffer = maxReduction;
                
                // Actualizar transfer display solo con datos de waveform (sin gain reduction)
                if (!inputSamples.empty() && !processedSamples.empty())
                {
                    // transferDisplay.setExtKeyActive(keyEnabled);
                    
                    // float scLevel = (processor.getSCValue(0) + processor.getSCValue(1)) * 0.5f;
                    float scLevel = -100.0f;
                    transferDisplay.setSidechainLevel(scLevel);
                    
                    // Enviar solo waveforms (el parámetro GR se ignora ahora)
                    transferDisplay.updateWaveformDataWithGR(&inputSamples[0], &processedSamples[0], &gainReductionSamples[0], 1);
                    
                    // NUEVO: Enviar valor actual de gain reduction para visualización en DELTA
                    float currentGR = processor.getGainReductionValue(0);
                    transferDisplay.setCurrentGainReduction(currentGR);

                    // Forzar repintado para mostrar envolvente actualizada
                    transferDisplay.repaint();
                }
            }
        }
        else
        {
            // Solo obtener el valor máximo de gain reduction para el medidor
            maxGainReductionFromBuffer = processor.getMaxGainReduction();
        }
    }
    else
    {
        // Si está en bypass o Logic está parado, no hay reducción
        maxGainReductionFromBuffer = 0.0f;
        
        // Forzar repaint del transfer display cuando Logic está parado
        // para que las envolventes y el histograma desaparezcan
        if (isProcessingInactive)
        {
            transferDisplay.repaint();
        }
    }
}

void JCBMaximizerAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    // Simplificado - sin manejo automático de cambios de parámetros para undo/redo
    if (button == &utilityButtons.undoButton)
    {
        undoManager.undo();
    }
    else if (button == &utilityButtons.redoButton)
    {
        undoManager.redo();
    }
    else if (button == &utilityButtons.resetGuiButton)
    {
        // Resetear tamaño de GUI, no parámetros
        resetGuiSize();
    }
    else if (button == &parameterButtons.bypassButton)
    {
        if (parameterButtons.bypassButton.getToggleState()) {
            // BYPASS desactiva DELTA, SOLO SC y DIAGRAM
            parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
            if (centerButtons.diagramButton.getToggleState()) {
                centerButtons.diagramButton.setToggleState(false, juce::dontSendNotification);
                hideDiagram(); // Cerrar DIAGRAM si está abierto
            }
        }
        updateButtonStates();
        
        // Actualizar transfer display para ocultar/mostrar envolventes basado en estado de bypass
        bool bypassActive = parameterButtons.bypassButton.getToggleState();
        transferDisplay.setBypassMode(bypassActive);
        
        // Si salimos de bypass y graphics está activo, restaurar envolventes
        if (!bypassActive && utilityButtons.runGraphicsButton.getToggleState()) {
            transferDisplay.setEnvelopeVisible(true);
            grMeter.setVisible(true);
            transferDisplay.repaint();
            grMeter.repaint();
        }
        
        // Actualizar output meters para usar gradient de entrada cuando bypass está activo
        outputMeterL.setBypassMode(bypassActive);
        outputMeterR.setBypassMode(bypassActive);
    }
    else if (button == &utilityButtons.runGraphicsButton)
    {
        bool newState = utilityButtons.runGraphicsButton.getToggleState();
        // Graphics ON = mostrar envolventes en tiempo real
        // Graphics OFF = ocultar envolventes (mejor rendimiento)
        transferDisplay.setEnvelopeVisible(newState);  // Directo, sin inversión
        processor.setEnvelopeVisualEnabled(newState);  // Directo, sin inversión
        grMeter.setVisible(newState);  // Directo, sin inversión
        // Mantener el texto siempre como "graphics"
        // Forzar repaint para asegurar actualización visual inmediata
        transferDisplay.repaint();
        grMeter.repaint();
        handleParameterChange();  // Marcar preset como modificado
    }
    // Botones de gestión de presets
    else if (button == &presetArea.saveButton)
    {
        // Desactivar botones momentáneos antes de guardar
        parameterButtons.bypassButton.setToggleState(false, juce::sendNotification);
        parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        savePresetFile();
    }
    else if (button == &presetArea.saveAsButton)
    {
        // Desactivar botones momentáneos antes de guardar
        parameterButtons.bypassButton.setToggleState(false, juce::sendNotification);
        parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        saveAsPresetFile();
    }
    else if (button == &presetArea.deleteButton)
    {
        // Desactivar botones momentáneos antes de mostrar el diálogo
        parameterButtons.bypassButton.setToggleState(false, juce::sendNotification);
        parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        deletePresetFile();
    }
    else if (button == &presetArea.backButton)
    {
        // Desactivar botones momentáneos antes de cambiar preset
        parameterButtons.bypassButton.setToggleState(false, juce::sendNotification);
        parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        selectPreviousPreset();
    }
    else if (button == &presetArea.nextButton)
    {
        // Desactivar botones momentáneos antes de cambiar preset
        parameterButtons.bypassButton.setToggleState(false, juce::sendNotification);
        parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        selectNextPreset();
    }
    else if (button == &utilityButtons.tooltipToggleButton)
    {
        // Alternar visibilidad del tooltip
        tooltipsEnabled = utilityButtons.tooltipToggleButton.getToggleState();
        tooltipComponent.setTooltipsEnabled(tooltipsEnabled);
        
        // Guardar estado del tooltip al processor
        processor.setTooltipEnabled(tooltipsEnabled);
        
        // Habilitar/deshabilitar botón de idioma basado en estado del tooltip
        utilityButtons.tooltipLangButton.setEnabled(tooltipsEnabled);
        utilityButtons.tooltipLangButton.setAlpha(tooltipsEnabled ? 1.0f : 0.25f);
    }
    else if (button == &utilityButtons.tooltipLangButton)
    {
        // Alternar idioma
        currentLanguage = (currentLanguage == TooltipLanguage::Spanish) ? 
                         TooltipLanguage::English : TooltipLanguage::Spanish;
        
        // Actualizar texto del botón
        utilityButtons.tooltipLangButton.setButtonText(
            currentLanguage == TooltipLanguage::Spanish ? "esp" : "eng");
        
        // Actualizar color del botón basado en el idioma
        if (currentLanguage == TooltipLanguage::English) {
            // Color azul para inglés
            utilityButtons.tooltipLangButton.setColour(juce::TextButton::textColourOffId, DarkTheme::accentSecondary);
        } else {
            // Color por defecto para español
            utilityButtons.tooltipLangButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
        }
        
        // Guardar preferencia de idioma
        processor.setTooltipLanguageEnglish(currentLanguage == TooltipLanguage::English);
        
        // Actualizar todos los tooltips con el nuevo idioma
        updateAllTooltips();
        
        // Actualizar textos de botones TODO con el nuevo idioma
        updateTodoButtonTexts();
    }
    else if (button == &utilityButtons.hqButton)
    {
        // Desactivar DELTA antes de activar HQ
        if (parameterButtons.deltaButton.getToggleState()) {
            parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        }
        
        // TODO: Implementar oversampling HQ
    }
    else if (button == &utilityButtons.dualMonoButton)
    {
        // Desactivar DELTA antes de activar Dual Mono
        if (parameterButtons.deltaButton.getToggleState()) {
            parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        }
        
        // Manejar exclusividad de botones de modo estéreo
        if (utilityButtons.dualMonoButton.getToggleState())
        {
            // utilityButtons.stereoLinkedButton.setToggleState(false, juce::dontSendNotification); // LINK siempre está activo
            utilityButtons.msButton.setToggleState(false, juce::dontSendNotification);
        }
    }
    else if (button == &utilityButtons.msButton)
    {
        // Desactivar DELTA antes de activar M/S
        if (parameterButtons.deltaButton.getToggleState()) {
            parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        }
        
        // Manejar exclusividad de botones de modo estéreo
        if (utilityButtons.msButton.getToggleState())
        {
            utilityButtons.dualMonoButton.setToggleState(false, juce::dontSendNotification);
            // utilityButtons.stereoLinkedButton.setToggleState(false, juce::dontSendNotification); // LINK siempre está activo
        }
    }
    else if (button == &topButtons.abStateButton)
    {
        // Desactivar DELTA antes de alternar estado A/B
        if (parameterButtons.deltaButton.getToggleState()) {
            parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        }
        
        // Alternar estado A/B
        processor.toggleAB();
        
        const bool isStateA = processor.getIsStateA();
        const auto abStateColour = isStateA ? juce::Colour(0xff9E35B0) : juce::Colour(0xff2196f3);

        // Actualizar texto del botón para mostrar estado actual
        topButtons.abStateButton.setButtonText(isStateA ? "A" : "B");
        topButtons.abStateButton.setColour(juce::TextButton::buttonColourId, abStateColour);
        topButtons.abStateButton.setColour(juce::TextButton::buttonOnColourId, abStateColour);
        topButtons.abStateButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        topButtons.abStateButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);

        topButtons.abCopyButton.setButtonText(isStateA ? "A-B" : "B-A");
        topButtons.abCopyButton.setTooltip(getTooltipText(isStateA ? "abcopyatob" : "abcopybtoa"));
        
        repaint();
    }
    else if (button == &topButtons.abCopyButton)
    {
        // Desactivar DELTA antes de copiar estado A/B
        if (parameterButtons.deltaButton.getToggleState()) {
            parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        }
        
        // Copiar estado actual al otro
        if (processor.getIsStateA()) {
            processor.copyAtoB();
            // Retroalimentación visual - destello púrpura a azul
            topButtons.abCopyButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2196f3));  // Destello azul
            topButtons.abCopyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            // Usar SafePointer para prevenir crashes si el editor se destruye antes de que se dispare el timer
            juce::Component::SafePointer<JCBMaximizerAudioProcessorEditor> safeThis(this);
            juce::Timer::callAfterDelay(300, [safeThis]() {
                if (safeThis) {
                    safeThis->topButtons.abCopyButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
                    safeThis->topButtons.abCopyButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
                }
            });
        } else {
            processor.copyBtoA();
            // Retroalimentación visual - destello azul a púrpura
            topButtons.abCopyButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff9E35B0));  // Destello púrpura
            topButtons.abCopyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            // Usar SafePointer para prevenir crashes si el editor se destruye antes de que se dispare el timer
            juce::Component::SafePointer<JCBMaximizerAudioProcessorEditor> safeThis(this);
            juce::Timer::callAfterDelay(300, [safeThis]() {
                if (safeThis) {
                    safeThis->topButtons.abCopyButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
                    safeThis->topButtons.abCopyButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
                }
            });
        }
    }
    else if (button == &utilityButtons.midiLearnButton)
    {
        // Desactivar DELTA antes de activar MIDI Learn
        if (parameterButtons.deltaButton.getToggleState()) {
            parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        }
        
        // TODO: Implementar modo MIDI learn
    }
    else if (button == &utilityButtons.zoomButton)
    {
        // Ciclar entre dos niveles de zoom
        auto currentZoom = transferDisplay.getZoomLevel();
        TransferFunctionDisplay::ZoomLevel newZoom;
        
        switch (currentZoom)
        {
            case TransferFunctionDisplay::ZoomLevel::Normal:
                // Normal -> Enfoque en techo
                newZoom = TransferFunctionDisplay::ZoomLevel::Zoomed;
                utilityButtons.zoomButton.setToggleState(true, juce::dontSendNotification);
                grMeter.setZoomLevel(true);  // Sincronizar medidor GR con vista ampliada
                break;
                
            case TransferFunctionDisplay::ZoomLevel::Zoomed:
                // Ampliado -> Normal
                newZoom = TransferFunctionDisplay::ZoomLevel::Normal;
                utilityButtons.zoomButton.setToggleState(false, juce::dontSendNotification);
                grMeter.setZoomLevel(false);
                break;
                
            default:
                // Por defecto a Normal si algo sale mal
                newZoom = TransferFunctionDisplay::ZoomLevel::Normal;
                utilityButtons.zoomButton.setToggleState(false, juce::dontSendNotification);
                grMeter.setZoomLevel(false);
                break;
        }

        utilityButtons.zoomButton.setButtonText("zoom");
        transferDisplay.setZoomLevel(newZoom);
    }
    else if (button == &centerButtons.diagramButton)
    {
        // Anti-rebote para evitar clicks múltiples
        juce::uint32 currentTime = juce::Time::getMillisecondCounter();
        if (currentTime - lastDiagramButtonTime < DIAGRAM_BUTTON_DEBOUNCE_MS)
        {
            return;  // Ignorar clicks muy rápidos
        }
        lastDiagramButtonTime = currentTime;
        
        // Alternar diagrama basado en visibilidad actual
        bool diagramWillBeActivated = (diagramOverlay == nullptr || !diagramOverlay->isVisible());
        
        // Comportamiento exclusivo: solo desactivar otros cuando DIAGRAM se va a activar
        if (diagramWillBeActivated) {
            // DIAGRAM desactiva BYPASS, DELTA y SOLO SC cuando se activa
            parameterButtons.bypassButton.setToggleState(false, juce::sendNotification);
            parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        }
        
        // Ejecutar el alternado
        if (diagramOverlay != nullptr && diagramOverlay->isVisible())
        {
            hideDiagram();
        }
        else
        {
            showDiagram();
        }
    }
    // Manejador del botón CODE removido - funcionalidad manejada por botón DIAGRAM
    else if (button == &parameterButtons.deltaButton)
    {
        bool deltaActive = parameterButtons.deltaButton.getToggleState();
        
        if (deltaActive) {
            // DELTA desactiva BYPASS, SOLO SC y DIAGRAM
            parameterButtons.bypassButton.setToggleState(false, juce::sendNotification);
            if (centerButtons.diagramButton.getToggleState()) {
                centerButtons.diagramButton.setToggleState(false, juce::dontSendNotification);
                hideDiagram(); // Cerrar DIAGRAM si está abierto
            }
        }
        
        // REFACTORIZADO: Usar método centralizado para aplicar/restaurar estado DELTA
        applyDeltaModeToAllControls(deltaActive);
        
        updateButtonStates();
    }
}

//==============================================================================
// MANEJO DE PARÁMETROS Y EVENTOS
//==============================================================================
//==============================================================================
// GESTIÓN DE ESTADO DE PARÁMETROS
//==============================================================================
void JCBMaximizerAudioProcessorEditor::handleParameterChange()
{
    // No hacer nada si estamos cargando un preset
    if (isLoadingPreset) {
        return;
    }
    
    if (presetArea.presetMenu.getSelectedId() > 0) {
        // Un preset está seleccionado - deseleccionar y mostrar con asterisco
        juce::String currentPresetName = presetArea.presetMenu.getText();
        
        presetArea.presetMenu.setSelectedId(0);
        processor.setLastPreset(0);
        
        // Mostrar el nombre del preset con asterisco en cursiva
        auto modifiedText = currentPresetName + "*";
        presetArea.presetMenu.setTextWhenNothingSelected(modifiedText);
        presetArea.presetMenu.setTextItalic(true);
        
        // Guardar el estado visual en el processor
        processor.setPresetDisplayText(modifiedText);
        processor.setPresetTextItalic(true);
        
    } else {
        // No hay preset seleccionado - verificar texto actual
        juce::String currentText = presetArea.presetMenu.getTextWhenNothingSelected();
        
        if (currentText == "DEFAULT") {
            // DEFAULT ahora SÍ puede mostrarse como modificado
            // Esto permite detectar cuando graphics u otros elementos no-APVTS han cambiado
            presetArea.presetMenu.setTextWhenNothingSelected("DEFAULT*");
            presetArea.presetMenu.setTextItalic(true);
            
            // Guardar el estado visual en el processor
            processor.setPresetDisplayText("DEFAULT*");
            processor.setPresetTextItalic(true);
        }
        else if (currentText.isEmpty()) {
            // Si está vacío, no hacer nada
            presetArea.presetMenu.setTextWhenNothingSelected("");
            presetArea.presetMenu.setTextItalic(false);
            
            // Guardar el estado visual en el processor
            processor.setPresetDisplayText("");
            processor.setPresetTextItalic(false);
        }
        else if (!currentText.endsWith("*")) {
            // Si hay un preset cargado (no vacío, no DEFAULT y sin asterisco)
            // Añadir asterisco y poner en itálica
            presetArea.presetMenu.setTextWhenNothingSelected(currentText + "*");
            presetArea.presetMenu.setTextItalic(true);
            processor.setPresetDisplayText(currentText + "*");
            processor.setPresetTextItalic(true);
        }
        // Si ya tiene un asterisco, no hacer nada
    }

    // NUEVO: Actualizar alpha del REL slider basado en estado de AUTOREL
    updateRelSliderAlpha();
}

//==============================================================================
// MÉTODOS DE SETUP Y CONFIGURACIÓN
//==============================================================================
void JCBMaximizerAudioProcessorEditor::setupKnobs()
{
    // Establecer undo manager y editor para todos los sliders primero
    // === LEFT TOP KNOBS ===
    // THD
    leftTopKnobs.thdSlider.setComponentID("thd");
    leftTopKnobs.thdSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    leftTopKnobs.thdSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 70, 16);
    leftTopKnobs.thdSlider.setLookAndFeel(&sliderLAFBig);
    leftTopKnobs.thdSlider.setTextBoxIsEditable(true);
    leftTopKnobs.thdSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    leftTopKnobs.thdSlider.setDoubleClickReturnValue(true, 0.0);
    leftTopKnobs.thdSlider.setPopupDisplayEnabled(false, false, this);
    leftTopKnobs.thdSlider.setNumDecimalPlacesToDisplay(1);
    leftTopKnobs.thdSlider.setTextValueSuffix(" dB");
    // Configurar rango para gain positivo: 0 a 24dB
    leftTopKnobs.thdSlider.setRange(0.0, 24.0, 0.1);
    leftTopKnobs.thdSlider.setSkewFactorFromMidPoint(12.0);  // 12dB en el centro
    addAndMakeVisible(leftTopKnobs.thdSlider);
    if (auto* param = processor.apvts.getParameter("a_GAIN"))
    {
        leftTopKnobs.thdAttachment = std::make_unique<CustomSliderAttachment>(
            *param, leftTopKnobs.thdSlider, &undoManager);
        leftTopKnobs.thdAttachment->onParameterChange = [this]() { handleParameterChange(); };
    }
    // Tooltip actualizado via getTooltipText("thd") en updateAllTooltips()
    
    // CEILING (b_CELLING) - NUEVO slider específico del Maximizer
    leftTopKnobs.ceilingSlider.setComponentID("ceiling");
    leftTopKnobs.ceilingSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    leftTopKnobs.ceilingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    leftTopKnobs.ceilingSlider.setLookAndFeel(&sliderLAFBig);
    leftTopKnobs.ceilingSlider.setTextBoxIsEditable(true);
    leftTopKnobs.ceilingSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    leftTopKnobs.ceilingSlider.setDoubleClickReturnValue(true, -0.3);  // Default: -0.3 dB
    leftTopKnobs.ceilingSlider.setPopupDisplayEnabled(false, false, this);
    leftTopKnobs.ceilingSlider.setNumDecimalPlacesToDisplay(1);
    // Custom text formatting para mostrar "-12 dB" en el medio
    leftTopKnobs.ceilingSlider.textFromValueFunction = [](double value) {
        // Mostrar "-12 dB" cuando el valor esté cerca de -12 dB
        if (std::abs(value - (-12.0)) < 0.05)  // Tolerancia de ±0.05 dB
            return juce::String("-12 dB");
        else
            return juce::String(value, 1) + " dB";  // Formato normal con 1 decimal
    };
    // Rango: -60 dB a 0 dB (según PluginProcessor.cpp)
    leftTopKnobs.ceilingSlider.setRange(-60.0, 0.0, 0.1);
    // Centrar control visual en -12 dB para mejor resolución
    leftTopKnobs.ceilingSlider.setSkewFactorFromMidPoint(-12.0);
    addAndMakeVisible(leftTopKnobs.ceilingSlider);
    if (auto* param = processor.apvts.getParameter("b_CELLING"))
    {
        leftTopKnobs.ceilingAttachment = std::make_unique<CustomSliderAttachment>(
            *param, leftTopKnobs.ceilingSlider, &undoManager);
        leftTopKnobs.ceilingAttachment->onParameterChange = [this]() { handleParameterChange(); };
    }
    // Tooltip actualizado via getTooltipText("ceiling") en updateAllTooltips()
    
    // LA (Anticipación) - MOVIDO a rightTopControls
    rightTopControls.lookaheadSlider.setComponentID("lookahead");
    rightTopControls.lookaheadSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rightTopControls.lookaheadSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 70, 16);
    rightTopControls.lookaheadSlider.setLookAndFeel(&sliderLAFBig);
    rightTopControls.lookaheadSlider.setTextBoxIsEditable(true);
    rightTopControls.lookaheadSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    rightTopControls.lookaheadSlider.setTextValueSuffix(" ms");
    rightTopControls.lookaheadSlider.setRange(0.0, 10.0, 0.1);
    rightTopControls.lookaheadSlider.setValue(0.0);
    rightTopControls.lookaheadSlider.setDoubleClickReturnValue(true, 0.0);
    rightTopControls.lookaheadSlider.setPopupDisplayEnabled(false, false, this);
    rightTopControls.lookaheadSlider.setNumDecimalPlacesToDisplay(1);
    addAndMakeVisible(rightTopControls.lookaheadSlider);
    if (auto* param = processor.apvts.getParameter("n_LOOKAHEAD"))
    {
        rightTopControls.lookaheadAttachment = std::make_unique<CustomSliderAttachment>(
            *param, rightTopControls.lookaheadSlider, &undoManager);
        rightTopControls.lookaheadAttachment->onParameterChange = [this]() { handleParameterChange(); };
    }
    // Tooltip actualizado via getTooltipText("lookahead") en updateAllTooltips()

    // === PERILLAS INFERIORES DERECHAS ===
    // Configurar slider de ataque
    rightBottomKnobs.atkSlider.setComponentID("attack");
    rightBottomKnobs.atkSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rightBottomKnobs.atkSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    rightBottomKnobs.atkSlider.setLookAndFeel(&sliderLAFBig);
    rightBottomKnobs.atkSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    rightBottomKnobs.atkSlider.setDoubleClickReturnValue(true, 100.0);  // Valor por defecto 100ms
    rightBottomKnobs.atkSlider.setPopupDisplayEnabled(false, false, this);
    rightBottomKnobs.atkSlider.setTextBoxIsEditable(true);
    // Custom text formatting con decimales progresivos
    rightBottomKnobs.atkSlider.textFromValueFunction = [](double value) {
        if (value < 1.0)
            return juce::String(value, 3) + " ms";  // 3 decimales: 0.001
        else if (value < 10.0)
            return juce::String(value, 2) + " ms";  // 2 decimales: 1.50
        else if (value < 100.0)
            return juce::String(value, 1) + " ms";  // 1 decimal: 10.5
        else
            return juce::String(value, 1) + " ms";  // 1 decimal: 100.5
    };
    // Configurar rango con compensación ligera para mejor balance visual
    rightBottomKnobs.atkSlider.setRange(0.1, 250.0, 0.01);
    rightBottomKnobs.atkSlider.setSkewFactorFromMidPoint(20.0);  // Compensación ligera para balance visual
    addAndMakeVisible(rightBottomKnobs.atkSlider);
    if (auto* param = processor.apvts.getParameter("d_ATK"))
    {
        rightBottomKnobs.atkAttachment = std::make_unique<CustomSliderAttachment>(
            *param, rightBottomKnobs.atkSlider, &undoManager);
        rightBottomKnobs.atkAttachment->onParameterChange = [this]() { handleParameterChange(); };
    }
    // Tooltip actualizado via getTooltipText("attack") en updateAllTooltips()
    
    // REL - modificado con rango mínimo de 1ms y formato de decimales progresivo
    rightBottomKnobs.relSlider.setComponentID("release");
    rightBottomKnobs.relSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rightBottomKnobs.relSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    rightBottomKnobs.relSlider.setLookAndFeel(&sliderLAFBig);
    rightBottomKnobs.relSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    rightBottomKnobs.relSlider.setRange(0.1, 1000.0, 0.01);  // Coherente con parámetro
    rightBottomKnobs.relSlider.setSkewFactorFromMidPoint(30.0);  // Centrar rango operativo 1-50ms
    rightBottomKnobs.relSlider.setDoubleClickReturnValue(true, 200.0);  // Valor por defecto 200ms
    rightBottomKnobs.relSlider.setPopupDisplayEnabled(false, false, this);
    rightBottomKnobs.relSlider.setTextBoxIsEditable(true);
    // Custom text formatting con decimales progresivos - coherente con parámetro
    rightBottomKnobs.relSlider.textFromValueFunction = [](double value) {
        if (value < 1.0)
            return juce::String(value, 2) + " ms";  // 2 decimales: 0.10
        else if (value < 10.0)
            return juce::String(value, 1) + " ms";  // 1 decimal: 1.5
        else if (value < 100.0)
            return juce::String(value, 1) + " ms";  // 1 decimal: 10.5
        else
            return juce::String(value, 0) + " ms";  // Sin decimales >= 100ms
    };
    addAndMakeVisible(rightBottomKnobs.relSlider);
    if (auto* param = processor.apvts.getParameter("e_REL"))
    {
        rightBottomKnobs.relAttachment = std::make_unique<CustomSliderAttachment>(
            *param, rightBottomKnobs.relSlider, &undoManager);
        rightBottomKnobs.relAttachment->onParameterChange = [this]() { handleParameterChange(); };
    }
    // === NUEVOS CONTROLES MAXIMIZER ===
    
    // DITHER button - área izquierda
    rightBottomKnobs.ditherButton.setComponentID("dither");
    rightBottomKnobs.ditherButton.setLookAndFeel(&smallButtonLAF);
    rightBottomKnobs.ditherButton.setButtonText("DITHER");
    rightBottomKnobs.ditherButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    rightBottomKnobs.ditherButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF46224F));
    rightBottomKnobs.ditherButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    rightBottomKnobs.ditherButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    rightBottomKnobs.ditherButton.setClickingTogglesState(true);
    addAndMakeVisible(rightBottomKnobs.ditherButton);
    if (auto* param = processor.apvts.getParameter("g_DITHER"))
    {
        rightBottomKnobs.ditherAttachment = std::make_unique<UndoableButtonAttachment>(
            *param, rightBottomKnobs.ditherButton, &undoManager);
        rightBottomKnobs.ditherAttachment->onParameterChange = [this]() { handleParameterChange(); };
    }
    // Tooltip actualizado via getTooltipText("dither") en updateAllTooltips()
    
    // DC FILTER button - centro superior
    rightBottomKnobs.dcFilterButton.setComponentID("dcfilter");
    rightBottomKnobs.dcFilterButton.setLookAndFeel(&smallButtonLAF);
    rightBottomKnobs.dcFilterButton.setButtonText("DC FILTER");
    rightBottomKnobs.dcFilterButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    rightBottomKnobs.dcFilterButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF46224F));
    rightBottomKnobs.dcFilterButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    rightBottomKnobs.dcFilterButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    rightBottomKnobs.dcFilterButton.setClickingTogglesState(true);
    addAndMakeVisible(rightBottomKnobs.dcFilterButton);
    if (auto* param = processor.apvts.getParameter("o_DCFILT"))
    {
        rightBottomKnobs.dcFilterAttachment = std::make_unique<UndoableButtonAttachment>(
            *param, rightBottomKnobs.dcFilterButton, &undoManager);
        rightBottomKnobs.dcFilterAttachment->onParameterChange = [this]() { handleParameterChange(); };
    }
    // Tooltip actualizado via getTooltipText("dcfilter") en updateAllTooltips()
    
    // DET knob - área derecha superior  
    rightTopControls.detSlider.setComponentID("detect");
    rightTopControls.detSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rightTopControls.detSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 70, 16);
    rightTopControls.detSlider.setLookAndFeel(&sliderLAFBig);
    rightTopControls.detSlider.setTextBoxIsEditable(true);
    rightTopControls.detSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    rightTopControls.detSlider.setRange(0.0, 1.0, 0.01);
    rightTopControls.detSlider.setValue(0.0);
    rightTopControls.detSlider.setDoubleClickReturnValue(true, 1.0);  // Corregido: 0.0 → 1.0 (RMS)
    rightTopControls.detSlider.setPopupDisplayEnabled(false, false, this);
    rightTopControls.detSlider.setNumDecimalPlacesToDisplay(2);
    // Custom text formatting para mostrar Peak/RMS
    rightTopControls.detSlider.textFromValueFunction = [](double value) {
        if (value < 0.01) return juce::String("Peak");
        else if (value > 0.99) return juce::String("RMS");
        else return juce::String(value, 2);
    };
    addAndMakeVisible(rightTopControls.detSlider);
    if (auto* param = processor.apvts.getParameter("l_DETECT"))
    {
        rightTopControls.detAttachment = std::make_unique<CustomSliderAttachment>(
            *param, rightTopControls.detSlider, &undoManager);
        rightTopControls.detAttachment->onParameterChange = [this]() { handleParameterChange(); };
    }
    
    // AUTOREL button - área derecha inferior, junto a REL
    rightBottomKnobs.autorelButton.setComponentID("autorel");
    rightBottomKnobs.autorelButton.setLookAndFeel(&smallButtonLAF);
    rightBottomKnobs.autorelButton.setButtonText("AUTOREL");
    rightBottomKnobs.autorelButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    rightBottomKnobs.autorelButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF2F594A));
    rightBottomKnobs.autorelButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    rightBottomKnobs.autorelButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    rightBottomKnobs.autorelButton.setClickingTogglesState(true);
    addAndMakeVisible(rightBottomKnobs.autorelButton);
    if (auto* param = processor.apvts.getParameter("m_AUTOREL"))
    {
        rightBottomKnobs.autorelAttachment = std::make_unique<UndoableButtonAttachment>(
            *param, rightBottomKnobs.autorelButton, &undoManager);
        rightBottomKnobs.autorelAttachment->onParameterChange = [this]() { handleParameterChange(); };
    } 
    // Tooltip actualizado via getTooltipText("autorel") en updateAllTooltips()

    // DELTA - botón con texto "DELTA" (estilo SC)
    parameterButtons.deltaButton.setButtonText("DELTA");
    parameterButtons.deltaButton.setClickingTogglesState(true);
    parameterButtons.deltaButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    parameterButtons.deltaButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0, 180, 140).withAlpha(0.3f)); // Verde teal como los medidores delta
    parameterButtons.deltaButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.withAlpha(0.7f));
    parameterButtons.deltaButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    parameterButtons.deltaButton.addListener(this);
    addAndMakeVisible(parameterButtons.deltaButton);
    // deltaAttachment movido a parameterButtons - manejado en setupParameterButtons()

    // Conectar perillas al transfer display
    leftTopKnobs.thdSlider.onValueChange = [this]() {
        // THD ahora está en rango de dB (-60 a 0)
        float thresholdDB = static_cast<float>(leftTopKnobs.thdSlider.getValue());
        transferDisplay.setThreshold(thresholdDB);
        updateTransferDisplay();
    };
}

void JCBMaximizerAudioProcessorEditor::setupMeters()
{
    // Medidores de entrada
    addAndMakeVisible(inputMeterL);
    addAndMakeVisible(inputMeterR);
    // Medidor GR
    addAndMakeVisible(grMeter);
    
    // Medidores de salida
    addAndMakeVisible(outputMeterL);
    addAndMakeVisible(outputMeterR);

    // Slider de trim
    trimSlider.setComponentID("trim");
    addAndMakeVisible(trimSlider);
    
    // Vincular slider de trim al parámetro - ahora usando attachment thread-safe
    if (auto* param = processor.apvts.getParameter("j_TRIM"))
    {
        trimAttachment = std::make_unique<CustomSliderAttachment>(
            *param, trimSlider, &undoManager);
        trimAttachment->onParameterChange = [this]() { handleParameterChange(); };
    }
    
    makeupSlider.setComponentID("makeup");
    addAndMakeVisible(makeupSlider);
    
    // Vincular slider de makeup al parámetro i_MAKEUP - usando attachment thread-safe
    if (auto* param = processor.apvts.getParameter("i_MAKEUP"))
    {
        makeupAttachment = std::make_unique<CustomSliderAttachment>(
            *param, makeupSlider, &undoManager);
        makeupAttachment->onParameterChange = [this]() { handleParameterChange(); };
    }
}

void JCBMaximizerAudioProcessorEditor::setupPresetArea()
{
    // Botón Save - estilo transparente
    presetArea.saveButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetArea.saveButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    presetArea.saveButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    presetArea.saveButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    presetArea.saveButton.addListener(this);
    addAndMakeVisible(presetArea.saveButton);
    // Tooltip actualizado via getTooltipText("save") en updateAllTooltips()
    
    // Botón Save As - estilo transparente
    presetArea.saveAsButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetArea.saveAsButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    presetArea.saveAsButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    presetArea.saveAsButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    presetArea.saveAsButton.addListener(this);
    addAndMakeVisible(presetArea.saveAsButton);
    // Tooltip actualizado via getTooltipText("saveas") en updateAllTooltips()
    
    // Botón Delete - estilo transparente
    presetArea.deleteButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetArea.deleteButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    presetArea.deleteButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    presetArea.deleteButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    presetArea.deleteButton.addListener(this);
    addAndMakeVisible(presetArea.deleteButton);
    // Tooltip actualizado via getTooltipText("delete") en updateAllTooltips()
    
    // Botón Back - estilo transparente
    presetArea.backButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetArea.backButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    presetArea.backButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    presetArea.backButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    presetArea.backButton.addListener(this);
    addAndMakeVisible(presetArea.backButton);
    // Tooltip actualizado via getTooltipText("back") en updateAllTooltips()
    
    // Botón Next - estilo transparente
    presetArea.nextButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetArea.nextButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    presetArea.nextButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    presetArea.nextButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    presetArea.nextButton.addListener(this);
    addAndMakeVisible(presetArea.nextButton);
    // Tooltip actualizado via getTooltipText("next") en updateAllTooltips()
    
    // Menú de preset utilizando CustomComboBox
    presetArea.presetMenu.setJustificationType(juce::Justification::centred);
    presetArea.presetMenu.setTextWhenNothingSelected("");
    presetArea.presetMenu.setTextWhenNoChoicesAvailable("No presets");
    
    // Configurar onChange para cargar presets
    presetArea.presetMenu.onChange = [this]() {
        int selectedId = presetArea.presetMenu.getSelectedId();
        if (selectedId == 0) return;
        
        // Salir de DELTA al seleccionar un preset
        if (parameterButtons.deltaButton.getToggleState()) {
            parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
        }
        
        // Salir de BYPASS al seleccionar un preset
        if (parameterButtons.bypassButton.getToggleState()) {
            parameterButtons.bypassButton.setToggleState(false, juce::sendNotification);
        }
        
        // CRÍTICO: Establecer flag de carga PRIMERO para prevenir transacciones de undo
        isLoadingPreset = true;
        
        // Guard de scope para asegurar que el flag siempre se resetea
        struct LoadingGuard {
            bool& flag;
            LoadingGuard(bool& f) : flag(f) { flag = true; }
            ~LoadingGuard() { flag = false; }
        } guard(isLoadingPreset);
        
        // NOTA: El historial de undo se borrará al final para evitar grabar cambios de parámetros
        
        // Buscar nombre real del preset usando el mapeo
        juce::String presetName;
        if (presetIdToNameMap.find(selectedId) != presetIdToNameMap.end()) {
            presetName = presetIdToNameMap[selectedId];
        } else {
            // Fallback por si acaso (no debería pasar)
            presetName = presetArea.presetMenu.getText();
        }
        
        // Ignorar separadores
        if (presetName.startsWith("---")) {
            presetArea.presetMenu.setSelectedId(0);
            return;
        }
        
        if (presetName == "DEFAULT") {
            // Preset DEFAULT: Resetear todos los parámetros a sus valores por defecto definidos
            // Usar acceso directo a parámetros para actualizaciones inmediatas y confiables
            
            // Deshabilitar undo durante la carga de preset
            isLoadingPreset = true;
            
            // Establecer todos los parámetros a sus valores por defecto usando gesture mechanism
            // Esto replica el mismo mecanismo usado por CustomSliderAttachment (doble-click)
            if (auto* param = processor.apvts.getParameter("j_TRIM")) {
                float defaultValue = param->getDefaultValue();
                float realValue = param->getNormalisableRange().convertFrom0to1(defaultValue);
                trimSlider.setValue(realValue, juce::sendNotificationSync);
            }
            if (auto* param = processor.apvts.getParameter("a_GAIN")) {
                float defaultValue = param->getDefaultValue();
                float realValue = param->getNormalisableRange().convertFrom0to1(defaultValue);
                leftTopKnobs.thdSlider.setValue(realValue, juce::sendNotificationSync);
            }
            if (auto* param = processor.apvts.getParameter("b_CELLING")) {
                float defaultValue = param->getDefaultValue();
                float realValue = param->getNormalisableRange().convertFrom0to1(defaultValue);
                leftTopKnobs.ceilingSlider.setValue(realValue, juce::sendNotificationSync);
            }
            
            if (auto* param = processor.apvts.getParameter("d_ATK")) {
                float defaultValue = param->getDefaultValue();
                float realValue = param->getNormalisableRange().convertFrom0to1(defaultValue);
                rightBottomKnobs.atkSlider.setValue(realValue, juce::sendNotificationSync);
            }

            if (auto* param = processor.apvts.getParameter("e_REL")) {
                float defaultValue = param->getDefaultValue();
                float realValue = param->getNormalisableRange().convertFrom0to1(defaultValue);
                rightBottomKnobs.relSlider.setValue(realValue, juce::sendNotificationSync);
            }

            if (auto* param = processor.apvts.getParameter("i_MAKEUP")) {
                float defaultValue = param->getDefaultValue();
                float realValue = param->getNormalisableRange().convertFrom0to1(defaultValue);
                makeupSlider.setValue(realValue, juce::sendNotificationSync);
            }
            
            if (auto* param = processor.apvts.getParameter("n_LOOKAHEAD")) {
                float defaultValue = param->getDefaultValue();
                float realValue = param->getNormalisableRange().convertFrom0to1(defaultValue);
                rightTopControls.lookaheadSlider.setValue(realValue, juce::sendNotificationSync);
            }

            if (auto* param = processor.apvts.getParameter("h_BYPASS")) {
                float defaultValue = param->getDefaultValue();
                bool toggleState = defaultValue >= 0.5f;
                parameterButtons.bypassButton.setToggleState(toggleState, juce::sendNotificationSync);
            }
            
            if (auto* param = processor.apvts.getParameter("k_DELTA")) {
                float defaultValue = param->getDefaultValue();
                bool toggleState = defaultValue >= 0.5f;
                parameterButtons.deltaButton.setToggleState(toggleState, juce::sendNotificationSync);
            }
            if (auto* param = processor.apvts.getParameter("g_DITHER")) {
                float defaultValue = param->getDefaultValue();
                bool toggleState = defaultValue >= 0.5f;
                rightBottomKnobs.ditherButton.setToggleState(toggleState, juce::sendNotificationSync);
            }
            if (auto* param = processor.apvts.getParameter("o_DCFILT")) {
                float defaultValue = param->getDefaultValue();
                bool toggleState = defaultValue >= 0.5f;
                rightBottomKnobs.dcFilterButton.setToggleState(toggleState, juce::sendNotificationSync);
            }
            if (auto* param = processor.apvts.getParameter("l_DETECT")) {
                float defaultValue = param->getDefaultValue();
                float realValue = param->getNormalisableRange().convertFrom0to1(defaultValue);
                rightTopControls.detSlider.setValue(realValue, juce::sendNotificationSync);
            }
            if (auto* param = processor.apvts.getParameter("m_AUTOREL")) {
                float defaultValue = param->getDefaultValue();
                bool toggleState = defaultValue >= 0.5f;
                rightBottomKnobs.autorelButton.setToggleState(toggleState, juce::sendNotificationSync);
            }

            // Reactivar undo después carga de preset
            isLoadingPreset = false;
            
            // Establecer el botón graphics en ON para el preset DEFAULT
            utilityButtons.runGraphicsButton.setToggleState(true, juce::dontSendNotification);
            // Actualizar visualización
            transferDisplay.setEnvelopeVisible(true);
            
            // IMPORTANTE: Forzar la sincronización directa de Gen~ para asegurar valores correctos de los parámetros
            // Esto replica la misma sincronización realizada durante la instanciación del plugin
            for (int i = 0; i < JCBMaximizer::num_params(); i++) {
                auto paramName = juce::String(JCBMaximizer::getparametername(processor.getPluginState(), i));
                if (auto* param = processor.apvts.getRawParameterValue(paramName)) {
                    float value = param->load();
                    
                    // Aplicar la misma validación que en parameterChanged() (MAXIMIZER)
                    if (paramName == "d_ATK" && value < 0.01f) {
                        value = 0.01f;  // Maximizer permite hasta 0.01ms
                    }
                    if (paramName == "e_REL" && value < 1.0f) {
                        value = 1.0f;   // Maximizer: mínimo 1ms
                    }
                    
                    JCBMaximizer::setparameter(processor.getPluginState(), i, value, nullptr);
                }
            }
        } 
        else if (presetName.startsWith("General_") ||
                 presetName.startsWith("Master_") ||
                 presetName.startsWith("Guitar_") ||
                 presetName.startsWith("Bass_") ||
                 presetName.startsWith("Drums_") ||
                 presetName.startsWith("Mastering_")) {
            // Es un factory preset - cargar desde BinaryData
            // presetName ya tiene el formato correcto (e.g., "General_Day_Menu")
            
            // Convertir el nombre a formato de recurso BinaryData
            juce::String resourceName = presetName + "_preset";
            
            // Buscar el recurso en BinaryData
            for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
                if (resourceName == BinaryData::namedResourceList[i]) {
                    // Cargar el preset desde BinaryData
                    int dataSize = 0;
                    const char* data = BinaryData::getNamedResource(BinaryData::namedResourceList[i], dataSize);
                    
                    if (data != nullptr && dataSize > 0) {
                        // Parsear el XML desde memoria
                        juce::String xmlContent(data, dataSize);
                        juce::XmlDocument xmlDoc(xmlContent);
                        std::unique_ptr<juce::XmlElement> xmlState(xmlDoc.getDocumentElement());
                        
                        if (xmlState != nullptr && xmlState->hasTagName(processor.apvts.state.getType())) {
                            auto stateTree = juce::ValueTree::fromXml(*xmlState);
                            processor.apvts.replaceState(stateTree);
                            
                            // Restaurar estado del botón graphics si está presente
                            auto uiSettings = stateTree.getChildWithName("UISettings");
                            if (uiSettings.isValid()) {
                                bool showGraphics = uiSettings.getProperty("displayGraphicsEnvelopes", true);
                                utilityButtons.runGraphicsButton.setToggleState(showGraphics, juce::dontSendNotification);
                                // Actualizar componentes visuales
                                transferDisplay.setEnvelopeVisible(showGraphics);
                            } else {
                                // Si no hay UISettings, los Factory presets activan graphics por defecto
                                utilityButtons.runGraphicsButton.setToggleState(true, juce::dontSendNotification);
                                transferDisplay.setEnvelopeVisible(true);
                            }
                            
                            // Queue las actualizaciones de parámetros para botones momentáneos (MAXIMIZER)
                            queueParameterUpdate("h_BYPASS", 0.0f);
                            queueParameterUpdate("k_DELTA", 0.0f);
                        }
                    }
                    break;
                }
            }
        }
        else {
            // Es un user preset - cargar desde archivo
            juce::File presetFile = getPresetsFolder().getChildFile(presetName + ".preset");
            
            if (presetFile.existsAsFile()) {
                juce::XmlDocument xmlDoc(presetFile);
                std::unique_ptr<juce::XmlElement> xmlState(xmlDoc.getDocumentElement());
                
                if (xmlState != nullptr && xmlState->hasTagName(processor.apvts.state.getType())) {
                    auto stateTree = juce::ValueTree::fromXml(*xmlState);
                    processor.apvts.replaceState(stateTree);
                    
                    // Restaurar estado del botón graphics si está presente
                    auto uiSettings = stateTree.getChildWithName("UISettings");
                    if (uiSettings.isValid()) {
                        bool showGraphics = uiSettings.getProperty("displayGraphicsEnvelopes", true);
                        utilityButtons.runGraphicsButton.setToggleState(showGraphics, juce::dontSendNotification);
                        // Actualizar componentes visuales
                        transferDisplay.setEnvelopeVisible(showGraphics);
                    }
                    
                    // Queue actualizaciones de parámetros para botones momentáneos (MAXIMIZER)
                    queueParameterUpdate("h_BYPASS", 0.0f);
                    queueParameterUpdate("k_DELTA", 0.0f);
                }
            }
        }
        
        // Formatear nombre para display (quitar prefijo de categoría si es factory preset)
        juce::String displayName = presetName;
        if (presetName.startsWith("General_")) {
            displayName = "[F] " + presetName.substring(8).replace("_", " ");
        }
        else if (presetName.startsWith("Master_")) {
            displayName = "[F] " + presetName.substring(7).replace("_", " ");
        }
        else if (presetName.startsWith("Guitar_")) {
            displayName = "[F] " + presetName.substring(7).replace("_", " ");
        }
        else if (presetName.startsWith("Bass_")) {
            displayName = "[F] " + presetName.substring(5).replace("_", " ");
        }
        else if (presetName.startsWith("Drums_")) {
            displayName = "[F] " + presetName.substring(6).replace("_", " ");  // "Drums_" tiene 6 caracteres
        }
        // Añadir más categorías cuando se necesiten
        
        // Actualizar estado en processor
        processor.setLastPreset(selectedId);
        processor.setPresetDisplayText(displayName);
        processor.setPresetTextItalic(false);
        presetArea.presetMenu.setTextItalic(false);
        presetArea.presetMenu.setTextWhenNothingSelected(displayName);
        
        // Actualizar sliders desde APVTS
        updateSliderValues();
        
        // Actualizar la gráfica de transferencia con los valores actuales
        // Es necesario obtener los valores directamente de los parámetros
        // porque los sliders pueden no estar actualizados todavía
        if (auto* thdParam = processor.apvts.getRawParameterValue("a_GAIN")) {
            transferDisplay.setThreshold(thdParam->load());
        }
        if (auto* ceilingParam = processor.apvts.getRawParameterValue("b_CELLING")) {
            transferDisplay.setCeiling(ceilingParam->load());
        }
        updateTransferDisplay();
        
        // Borrar el historial de undos DESPUÉS de haber establecido todos los valores.
        // Esto previene que los cambios de parámetros se registren en el historial de undo
        undoManager.clearUndoHistory();
        
        // Asegurar que el transfer display se actualice después de limpiar el historial de undo
        updateTransferDisplay();

        // Nota: el flag isLoadingPreset se resetea automáticamente por el destructor LoadingGuard
    };
    
    addAndMakeVisible(presetArea.presetMenu);
    
    // Inicializar menú de presets
    refreshPresetMenu();
    
    // Configurar el texto inicial del menú según el estado guardado
    auto savedText = processor.getPresetDisplayText();
    auto isItalic = processor.getPresetTextItalic();
    if (!savedText.isEmpty()) {
        presetArea.presetMenu.setTextWhenNothingSelected(savedText);
        presetArea.presetMenu.setTextItalic(isItalic);
    }
}

void JCBMaximizerAudioProcessorEditor::setupUtilityButtons()
{
    // Undo - estilo transparente
    utilityButtons.undoButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.undoButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    utilityButtons.undoButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    utilityButtons.undoButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.undoButton.addListener(this);
    addAndMakeVisible(utilityButtons.undoButton);
    
    // Redo - estilo transparente
    utilityButtons.redoButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.redoButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    utilityButtons.redoButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    utilityButtons.redoButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.redoButton.addListener(this);
    addAndMakeVisible(utilityButtons.redoButton);
    
    // Establecer el estado inicial de los botones de undo/redo (normalmente desactivados al inicio)
    utilityButtons.undoButton.setAlpha(0.3f);
    utilityButtons.undoButton.setEnabled(false);
    utilityButtons.redoButton.setAlpha(0.3f);
    utilityButtons.redoButton.setEnabled(false);
    
    // Reset GUI - estilo transparente
    utilityButtons.resetGuiButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.resetGuiButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    utilityButtons.resetGuiButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    utilityButtons.resetGuiButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.resetGuiButton.addListener(this);
    addAndMakeVisible(utilityButtons.resetGuiButton);
    // Tooltip actualizado via getTooltipText("resetgui") en updateAllTooltips()

    // Ejecutar gráficos
    utilityButtons.runGraphicsButton.setClickingTogglesState(true);
    utilityButtons.runGraphicsButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);  // OFF: transparente
    utilityButtons.runGraphicsButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));  // ON: con fondo azul
    utilityButtons.runGraphicsButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    utilityButtons.runGraphicsButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.runGraphicsButton.addListener(this);
    addAndMakeVisible(utilityButtons.runGraphicsButton);
    // Tooltip actualizado via getTooltipText("graphics") en updateAllTooltips()
    
    // Botón toggle de tooltip - estilo transparente
    utilityButtons.tooltipToggleButton.setClickingTogglesState(true);
    utilityButtons.tooltipToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.tooltipToggleButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    utilityButtons.tooltipToggleButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    utilityButtons.tooltipToggleButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.tooltipToggleButton.setToggleState(true, juce::dontSendNotification); // ON por defecto
    utilityButtons.tooltipToggleButton.addListener(this);
    addAndMakeVisible(utilityButtons.tooltipToggleButton);
    // Tooltip actualizado via getTooltipText("tooltiptoggle") en updateAllTooltips()
    
    // Botón de idioma - estilo transparente
    utilityButtons.tooltipLangButton.setClickingTogglesState(false);  // No toggle, solo botón normal
    utilityButtons.tooltipLangButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.tooltipLangButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    utilityButtons.tooltipLangButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    utilityButtons.tooltipLangButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.tooltipLangButton.setButtonText("esp");  // Texto inicial para español
    utilityButtons.tooltipLangButton.addListener(this);
    addAndMakeVisible(utilityButtons.tooltipLangButton);
    // Tooltip actualizado via getTooltipText("tooltiplang") en updateAllTooltips()
    
    // Botón HQ - oversampling
    utilityButtons.hqButton.setClickingTogglesState(true);
    utilityButtons.hqButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.hqButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    utilityButtons.hqButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.withAlpha(0.7f));
    utilityButtons.hqButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.hqButton.addListener(this);
    addAndMakeVisible(utilityButtons.hqButton);
    utilityButtons.hqButton.setEnabled(false);  // TODO: Implementar
    
    // Dual Mono button
    utilityButtons.dualMonoButton.setClickingTogglesState(true);
    utilityButtons.dualMonoButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.dualMonoButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    utilityButtons.dualMonoButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.withAlpha(0.7f));
    utilityButtons.dualMonoButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.dualMonoButton.addListener(this);
    addAndMakeVisible(utilityButtons.dualMonoButton);
    utilityButtons.dualMonoButton.setEnabled(false);  // TODO: Implementar
    
    // Stereo Linked button (ALWAYS ON - plugin only works in stereo linked mode)
    utilityButtons.stereoLinkedButton.setClickingTogglesState(false);  // No toggle
    utilityButtons.stereoLinkedButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.stereoLinkedButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    utilityButtons.stereoLinkedButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);  // Siempre texto blanco
    utilityButtons.stereoLinkedButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.stereoLinkedButton.setToggleState(true, juce::dontSendNotification); // Siempre ON
    // No listener necesario ya que siempre está activo
    addAndMakeVisible(utilityButtons.stereoLinkedButton);
    // Tooltip actualizado via getTooltipText("link") en updateAllTooltips()
    utilityButtons.stereoLinkedButton.setEnabled(false);  // Disabled - can't be changed
    
    // Botón M/S
    utilityButtons.msButton.setClickingTogglesState(true);
    utilityButtons.msButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.msButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    utilityButtons.msButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.withAlpha(0.7f));
    utilityButtons.msButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.msButton.addListener(this);
    addAndMakeVisible(utilityButtons.msButton);
    utilityButtons.msButton.setEnabled(false);  // TODO: Implementar
    
    // A/B State button
    const bool isStateA = processor.getIsStateA();
    const auto abStateColour = isStateA ? juce::Colour(0xff9E35B0) : juce::Colour(0xff2196f3);
    topButtons.abStateButton.setClickingTogglesState(false);  // No es toggle, es un indicador
    topButtons.abStateButton.setColour(juce::TextButton::buttonColourId, abStateColour);
    topButtons.abStateButton.setColour(juce::TextButton::buttonOnColourId, abStateColour);
    topButtons.abStateButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    topButtons.abStateButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    topButtons.abStateButton.addListener(this);
    topButtons.abStateButton.setButtonText(isStateA ? "A" : "B");
    addAndMakeVisible(topButtons.abStateButton);
    topButtons.abStateButton.setEnabled(true);  // Now implemented!
    
    // Botón copiar A/B
    topButtons.abCopyButton.setClickingTogglesState(false);
    topButtons.abCopyButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    topButtons.abCopyButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
    topButtons.abCopyButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    topButtons.abCopyButton.addListener(this);
    topButtons.abCopyButton.setButtonText(isStateA ? "A-B" : "B-A");
    topButtons.abCopyButton.setTooltip(getTooltipText(isStateA ? "abcopyatob" : "abcopybtoa"));
    addAndMakeVisible(topButtons.abCopyButton);
    // Tooltip actualizado dinámicamente en timerCallback()
    
    // MIDI Learn button
    utilityButtons.midiLearnButton.setClickingTogglesState(true);
    utilityButtons.midiLearnButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.midiLearnButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::purple.withAlpha(0.3f));
    utilityButtons.midiLearnButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.withAlpha(0.7f));
    utilityButtons.midiLearnButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.midiLearnButton.addListener(this);
    addAndMakeVisible(utilityButtons.midiLearnButton);
    utilityButtons.midiLearnButton.setEnabled(false);  // TODO: Implementar
    
    // Botón Zoom
    utilityButtons.zoomButton.setClickingTogglesState(true);
    utilityButtons.zoomButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    utilityButtons.zoomButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    utilityButtons.zoomButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);  // Cambiado para igualar otros botones
    utilityButtons.zoomButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    utilityButtons.zoomButton.addListener(this);
    addAndMakeVisible(utilityButtons.zoomButton);
    utilityButtons.zoomButton.setButtonText("zoom"); // Estado inicial: Normal
    utilityButtons.zoomButton.setToggleState(false, juce::dontSendNotification); // Toggle OFF para normal
    // Tooltip actualizado via getTooltipText("zoom") en updateAllTooltips()
    
    // Botón Diagram
    centerButtons.diagramButton.setClickingTogglesState(true);
    centerButtons.diagramButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    centerButtons.diagramButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    centerButtons.diagramButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.withAlpha(0.7f));
    centerButtons.diagramButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    centerButtons.diagramButton.addListener(this);
    addAndMakeVisible(centerButtons.diagramButton);
    // Tooltip actualizado via getTooltipText("diagram") en updateAllTooltips()

}

void JCBMaximizerAudioProcessorEditor::setupParameterButtons()
{
    // Botón DELTA - movido desde rightBottomKnobs
    parameterButtons.deltaButton.setClickingTogglesState(true);
    parameterButtons.deltaButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    parameterButtons.deltaButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::teal.withAlpha(0.3f));
    parameterButtons.deltaButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.withAlpha(0.7f));
    parameterButtons.deltaButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    parameterButtons.deltaButton.addListener(this);
    addAndMakeVisible(parameterButtons.deltaButton);
    // Attachment de DELTA - excluido del undo (sin UndoManager) y no automatizable
    parameterButtons.deltaAttachment = std::make_unique<UndoableButtonAttachment>(
        *processor.apvts.getParameter("k_DELTA"), parameterButtons.deltaButton, nullptr);
    parameterButtons.deltaAttachment->onParameterChange = [this]() { handleParameterChange(); };
    // Tooltip actualizado via getTooltipText("delta") en updateAllTooltips()
    
    // Botón BYPASS - movido desde utilityButtons
    parameterButtons.bypassButton.setClickingTogglesState(true);
    parameterButtons.bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    parameterButtons.bypassButton.setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.withAlpha(0.3f));
    parameterButtons.bypassButton.setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.withAlpha(0.7f));
    parameterButtons.bypassButton.setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
    parameterButtons.bypassButton.addListener(this);
    addAndMakeVisible(parameterButtons.bypassButton);
    // Attachment de BYPASS - excluido del undo (sin UndoManager) y no automatizable
    parameterButtons.bypassAttachment = std::make_unique<UndoableButtonAttachment>(
        *processor.apvts.getParameter("h_BYPASS"), parameterButtons.bypassButton, nullptr);
    parameterButtons.bypassAttachment->onParameterChange = [this]() { handleParameterChange(); };
    // Tooltip actualizado via getTooltipText("bypass") en updateAllTooltips()
}

void JCBMaximizerAudioProcessorEditor::setupBackground()
{
    // Cargar imágenes de fondo
    normalBackground = juce::ImageCache::getFromMemory(BinaryData::fondo_png, BinaryData::fondo_pngSize);
    bypassBackground = juce::ImageCache::getFromMemory(BinaryData::bypass_png, BinaryData::bypass_pngSize);
    deltaBackground = juce::ImageCache::getFromMemory(BinaryData::delta_png, BinaryData::delta_pngSize);
    diagramBackground = juce::ImageCache::getFromMemory(BinaryData::diagramaFondo_png, BinaryData::diagramaFondo_pngSize);
    
    
    // Establecer background inicial
    if (normalBackground.isValid())
    {
        backgroundImage.setImage(normalBackground, juce::RectanglePlacement::stretchToFit);
        addAndMakeVisible(backgroundImage);
        backgroundImage.toBack();
    }
    
}

//==============================================================================
// MÉTODOS DE UPDATE Y REFRESH
//==============================================================================

void JCBMaximizerAudioProcessorEditor::updateButtonStates()
{
    // Función maestra que actualiza todos los estados de UI
    updateBasicButtonStates();
    updateBackgroundState();
    updateMeterStates();
    
    // MEJORADO: Asegurar que el estado DELTA se aplica correctamente
    // Esto es crítico para resolver el bug de sincronización al reabrir la ventana
    bool currentDeltaState = processor.apvts.getRawParameterValue("k_DELTA")->load() > 0.5f;
    applyDeltaModeToAllControls(currentDeltaState);
}

void JCBMaximizerAudioProcessorEditor::updateBasicButtonStates()
{
    // Obtener estados actuales
    const bool bypassActive = parameterButtons.bypassButton.getToggleState();
    
    // Actualizar estados visuales básicos
    isBypassed = bypassActive;
    bypassTextVisible = bypassActive;  // Siempre mostrar texto cuando bypass está activo
}



void JCBMaximizerAudioProcessorEditor::updateBackgroundState()
{
    // Obtener estados actuales
    const bool bypassActive = parameterButtons.bypassButton.getToggleState();
    const bool deltaActive = parameterButtons.deltaButton.getToggleState();
    
    // Actualizar fondo según prioridad: bypass > delta > normal
    if (bypassActive) {
        backgroundImage.setImage(bypassBackground, juce::RectanglePlacement::stretchToFit);
    }
    else if (deltaActive) {
        backgroundImage.setImage(deltaBackground, juce::RectanglePlacement::stretchToFit);
    }
    else {
        backgroundImage.setImage(normalBackground, juce::RectanglePlacement::stretchToFit);
    }
}

void JCBMaximizerAudioProcessorEditor::updateFilterButtonText()
{
}

void JCBMaximizerAudioProcessorEditor::updateMeterStates()
{
    // Obtener estados actuales
    const bool bypassActive = parameterButtons.bypassButton.getToggleState();
    const bool deltaActive = parameterButtons.deltaButton.getToggleState();
    const bool soloScActive = false;
    bool deltaMode = deltaActive && !bypassActive;
    bool soloMode = soloScActive && !bypassActive && !deltaActive;
    
    inputMeterL.setDeltaMode(deltaMode);
    inputMeterR.setDeltaMode(deltaMode);
    outputMeterL.setDeltaMode(deltaMode);
    outputMeterR.setDeltaMode(deltaMode);
    grMeter.setDeltaMode(deltaMode);  // Actualizar color del grMeter en modo DELTA
    
    inputMeterL.setSoloScMode(soloMode);
    inputMeterR.setSoloScMode(soloMode);
    outputMeterL.setSoloScMode(soloMode);
    outputMeterR.setSoloScMode(soloMode);

    // Simplificar lógica: grMeter visible cuando graphics está ON y no hay bypass ni solo SC
    bool graphicsActive = utilityButtons.runGraphicsButton.getToggleState();
    bool shouldShowGrMeter = graphicsActive && !bypassActive && !soloScActive;
    grMeter.setVisible(shouldShowGrMeter);
    
    // Actualizar gradiente de salida para modo bypass
    outputMeterL.setBypassMode(bypassActive);
    outputMeterR.setBypassMode(bypassActive);
    
    // CORRECCIÓN: Asegurar sincronización estado BYPASS al reabrir plugin
    // Esto resuelve el problema de la función de transferencia que reaparece incorrectamente
    transferDisplay.setBypassMode(bypassActive);
}

//==============================================================================
// MÉTODOS DE MANEJO DE DELTA MODE
//==============================================================================

void JCBMaximizerAudioProcessorEditor::applyDeltaModeToAllControls(bool deltaActive)
{
    // Aplicar modo DELTA a todos los controles de forma centralizada y simétrica
    applyDeltaModeToPresetControls(deltaActive);
    applyDeltaModeToAbControls(deltaActive);
    applyDeltaModeToUndoRedoControls(deltaActive);
    applyDeltaModeToUtilityControls(deltaActive);
    applyDeltaModeToMetersAndDisplay(deltaActive);
}

void JCBMaximizerAudioProcessorEditor::applyDeltaModeToPresetControls(bool deltaActive)
{
    // Todos los controles de preset permanecen enabled y con alpha 1.0
    presetArea.saveButton.setEnabled(true);
    presetArea.saveButton.setAlpha(1.0f);
    presetArea.saveAsButton.setEnabled(true);
    presetArea.saveAsButton.setAlpha(1.0f);
    presetArea.deleteButton.setEnabled(true);
    presetArea.deleteButton.setAlpha(1.0f);
    presetArea.backButton.setEnabled(true);
    presetArea.backButton.setAlpha(1.0f);
    presetArea.nextButton.setEnabled(true);
    presetArea.nextButton.setAlpha(1.0f);
    presetArea.presetMenu.setEnabled(true);
    presetArea.presetMenu.setAlpha(1.0f);
}

void JCBMaximizerAudioProcessorEditor::applyDeltaModeToAbControls(bool deltaActive)
{
    // Todos los controles A/B permanecen enabled y con alpha 1.0
    topButtons.abStateButton.setEnabled(true);
    topButtons.abStateButton.setAlpha(1.0f);
    topButtons.abCopyButton.setEnabled(true);
    topButtons.abCopyButton.setAlpha(1.0f);
}

void JCBMaximizerAudioProcessorEditor::applyDeltaModeToUndoRedoControls(bool deltaActive)
{
    bool canUndo = undoManager.canUndo();
    bool canRedo = undoManager.canRedo();
    
    utilityButtons.undoButton.setEnabled(canUndo);
    utilityButtons.undoButton.setAlpha(canUndo ? 1.0f : 0.3f);
    utilityButtons.redoButton.setEnabled(canRedo);
    utilityButtons.redoButton.setAlpha(canRedo ? 1.0f : 0.3f);
}

void JCBMaximizerAudioProcessorEditor::applyDeltaModeToUtilityControls(bool deltaActive)
{
    // Botones TODO - mantener alpha normal para permitir salir de DELTA
    utilityButtons.hqButton.setAlpha(1.0f);
    utilityButtons.dualMonoButton.setAlpha(1.0f);
    utilityButtons.stereoLinkedButton.setAlpha(1.0f);
    utilityButtons.msButton.setAlpha(1.0f);
    utilityButtons.midiLearnButton.setAlpha(1.0f);
    
    // Botones exclusivos BYPASS y DIAGRAM - mantener enabled para permitir salir de DELTA
    parameterButtons.bypassButton.setEnabled(true);
    parameterButtons.bypassButton.setAlpha(1.0f);
    
    centerButtons.diagramButton.setEnabled(true);
    centerButtons.diagramButton.setAlpha(1.0f);
}

void JCBMaximizerAudioProcessorEditor::applyDeltaModeToMetersAndDisplay(bool deltaActive)
{
    // Aplicar modo DELTA a medidores y display (solo gradientes, sin alpha)
    inputMeterL.setDeltaMode(deltaActive);
    inputMeterR.setDeltaMode(deltaActive);
    outputMeterL.setDeltaMode(deltaActive);
    outputMeterR.setDeltaMode(deltaActive);
    grMeter.setDeltaMode(deltaActive);
    
    // Configurar TransferDisplay
    transferDisplay.setDeltaMode(deltaActive);
    transferDisplay.setEnvelopeVisible(!deltaActive);
}

void JCBMaximizerAudioProcessorEditor::updateTransferDisplay()
{
    // Actualizar los valores de los parámetros desde los sliders
    float thresholdDB = leftTopKnobs.thdSlider.getValue();
    transferDisplay.setThreshold(thresholdDB);
    
    // Actualizar ceiling desde parámetro b_CELLING
    if (auto* ceilingParam = processor.apvts.getRawParameterValue("b_CELLING")) {
        transferDisplay.setCeiling(ceilingParam->load());
    }

    // Actualizar la curva visual
    transferDisplay.updateCurve();
}

void JCBMaximizerAudioProcessorEditor::updateMeters()
{
    // CORRECCIÓN CRÍTICA: Cachear estados de visibilidad para evitar llamadas repetidas a setVisible
    static bool lastSoloScActive = false;
    static bool lastExtKeyActive = false;
    
    // Verificar si SOLO SC está activo
    const bool soloScActive = false;
    const bool extKeyActive = false;  // Maximizer has no external key
    
    // Solo actualizar visibilidad cuando el estado realmente cambia
    if (soloScActive != lastSoloScActive || extKeyActive != lastExtKeyActive) {
        if (soloScActive) {
            // SOLO SC activo - mostrar meters apropiados basados en EXT KEY
            if (extKeyActive) {
                // Ocultar medidores principales de entrada
                inputMeterL.setVisible(false);
                inputMeterR.setVisible(false);
            } else {
                inputMeterL.setVisible(true);
                inputMeterR.setVisible(true);
            }
        } else {
            // Modo normal - mostrar todos los medidores normalmente
            inputMeterL.setVisible(true);
            inputMeterR.setVisible(true);
        }
        
        lastSoloScActive = soloScActive;
        lastExtKeyActive = extKeyActive;
    }
    
    // Actualizar medidores con control centralizado a 60 Hz
    // Llamar a updateLevel() de cada medidor para procesar valores y animaciones
    if (inputMeterL.isVisible()) {
        inputMeterL.updateLevel();
        inputMeterR.updateLevel();
        inputMeterL.repaint();
        inputMeterR.repaint();
    }

    // Siempre actualizar medidores de salida y reducción de ganancia
    grMeter.updateLevel();
    grMeter.repaint();
    
    outputMeterL.updateLevel();
    outputMeterR.updateLevel();
    outputMeterL.repaint();
    outputMeterR.repaint();
}

void JCBMaximizerAudioProcessorEditor::updateSliderValues()
{
    // Left top knobs - Todos usan CustomSliderAttachment
    if (auto* param = processor.apvts.getRawParameterValue("a_GAIN"))
        leftTopKnobs.thdSlider.setValue(param->load(), juce::dontSendNotification);
    
    if (auto* param = processor.apvts.getRawParameterValue("b_CELLING"))
        leftTopKnobs.ceilingSlider.setValue(param->load(), juce::dontSendNotification);
        
    if (auto* param = processor.apvts.getRawParameterValue("n_LOOKAHEAD"))
        rightTopControls.lookaheadSlider.setValue(param->load(), juce::dontSendNotification);

    // Right bottom knobs - Todos usan CustomSliderAttachment
    if (auto* param = processor.apvts.getRawParameterValue("d_ATK"))
        rightBottomKnobs.atkSlider.setValue(param->load(), juce::dontSendNotification);
    
    if (auto* param = processor.apvts.getRawParameterValue("e_REL"))
        rightBottomKnobs.relSlider.setValue(param->load(), juce::dontSendNotification);

    // Slider de trims (both linked to the same parameter)
    if (auto* param = processor.apvts.getRawParameterValue("j_TRIM")) {
        float trimValue = param->load();
        trimSlider.setValue(trimValue, juce::dontSendNotification);
    }

    // NUEVO: Actualizar alpha del REL slider basado en estado inicial de AUTOREL
    updateRelSliderAlpha();
}

void JCBMaximizerAudioProcessorEditor::resetGuiSize()
{
    // Ciclar al siguiente estado: Current -> Maximum -> Minimum -> Current
    switch (currentSizeState) {
        case GuiSizeState::Current:
            // Current -> Maximum
            processor.setSavedSize({MAX_WIDTH, MAX_HEIGHT});
            setSize(MAX_WIDTH, MAX_HEIGHT);
            currentSizeState = GuiSizeState::Maximum;
            break;

        case GuiSizeState::Maximum:
            // Maximum -> Minimum
            processor.setSavedSize({MIN_WIDTH, MIN_HEIGHT});
            setSize(MIN_WIDTH, MIN_HEIGHT);
            currentSizeState = GuiSizeState::Minimum;
            break;

        case GuiSizeState::Minimum:
            // Minimum -> Current (back to default)
            processor.setSavedSize({DEFAULT_WIDTH, DEFAULT_HEIGHT});
            setSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
            currentSizeState = GuiSizeState::Current;
            break;
    }
    
    // Forzar repaint para asegurar que se actualice correctamente
    repaint();
}

//==============================================================================
// GESTIÓN DE PRESETS
//==============================================================================
juce::File JCBMaximizerAudioProcessorEditor::getPresetsFolder()
{
    juce::File folder = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory)
        .getChildFile("Audio")
        .getChildFile("Presets")
        .getChildFile("Coeval")
        .getChildFile("JCBMaximizer");

    if (!folder.isDirectory() && !folder.existsAsFile()) {
        folder.createDirectory();
    }
    
    return folder;
}

juce::Array<juce::File> JCBMaximizerAudioProcessorEditor::populatePresetFolder()
{
    return getPresetsFolder().findChildFiles(2, false, "*.preset");
}

void JCBMaximizerAudioProcessorEditor::refreshPresetMenu()
{
    presetArea.presetMenu.clear();
    presetIdToNameMap.clear();  // Limpiar mapeo anterior
    
    // Añadir DEFAULT como primer preset siempre
    presetArea.presetMenu.addItem("DEFAULT", 1);
    presetIdToNameMap[1] = "DEFAULT";
    
    // Añadir separador
    presetArea.presetMenu.addItem("---", 2);
    
    // Organizar factory presets por categoría
    std::map<juce::String, juce::StringArray> categorizedPresets;
    std::map<juce::String, juce::StringArray> categorizedFullNames;
    int nextId = 100;
    
    // Procesar factory presets desde BinaryData
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        juce::String resourceName(BinaryData::namedResourceList[i]);
        
        // Buscar archivos que terminen en "_preset"
        if (resourceName.endsWith("_preset"))
        {
            juce::String cleanName = resourceName.replace("_preset", "");

            if (cleanName.contains("FactoryPresets_"))
                cleanName = cleanName.fromLastOccurrenceOf("FactoryPresets_", false, false);
            else if (cleanName.contains("Assets_"))
                cleanName = cleanName.fromLastOccurrenceOf("Assets_", false, false);

            // Detectar categoría del prefijo
            juce::String category = "General";
            juce::String presetName = cleanName;

            if (cleanName.startsWith("General_"))
            {
                category = "General";
                presetName = cleanName.substring(8).replace("_", " ");
            }
            else if (cleanName.startsWith("Master_"))
            {
                category = "Master";
                presetName = cleanName.substring(7).replace("_", " ");
            }
            else if (cleanName.startsWith("Guitar_"))
            {
                category = "Guitar";
                presetName = cleanName.substring(7).replace("_", " ");
            }
            else if (cleanName.startsWith("Bass_"))
            {
                category = "Bass";
                presetName = cleanName.substring(5).replace("_", " ");
            }
            else if (cleanName.startsWith("Drums_"))
            {
                category = "Drums";
                presetName = cleanName.substring(6).replace("_", " ");  // "Drums_" tiene 6 caracteres
            }

            categorizedPresets[category].add("[F] " + presetName);
            categorizedFullNames[category].add(cleanName);  // Guardar nombre completo con prefijo
        }
    }
    
    // Añadir categorías al menú en orden específico
    juce::StringArray categoryOrder = {"General", "Master", "Guitar", "Bass", "Drums"};
    
    for (const auto& category : categoryOrder)
    {
        if (categorizedPresets.find(category) != categorizedPresets.end())
        {
            auto& presets = categorizedPresets[category];
            auto& fullNames = categorizedFullNames[category];
            presets.sort(true);
            fullNames.sort(true);
            
            // Añadir categoría con sus subitems
            presetArea.presetMenu.addCategoryItem(category, presets, nextId);
            
            // Mapear IDs a nombres de presets
            for (int j = 0; j < presets.size(); ++j)
            {
                int presetId = nextId + j + 1;  // +1 porque el ID de la categoría es nextId
                presetIdToNameMap[presetId] = fullNames[j];  // Usar nombre completo con prefijo
            }
            
            nextId += static_cast<int>(presets.size()) + 1;
        }
    }
    
    // Añadir separador antes de user presets
    presetArea.presetMenu.addItem("---", nextId++);
    
    // Añadir user presets (sin categoría)
    juce::StringArray userPresets;
    auto presetsFolder = getPresetsFolder();
    for (auto& file : presetsFolder.findChildFiles(juce::File::findFiles, false, "*.preset"))
    {
        userPresets.add(file.getFileNameWithoutExtension());
    }
    
    userPresets.sort(true);
    for (const auto& preset : userPresets)
    {
        presetArea.presetMenu.addItem(preset, nextId);
        presetIdToNameMap[nextId] = preset;
        nextId++;
    }
    
    // Restaurar selección previa si existe
    if (processor.getLastPreset() > 0)
    {
        presetArea.presetMenu.setSelectedIdWithoutNotification(processor.getLastPreset());
    }
}

void JCBMaximizerAudioProcessorEditor::savePresetFile()
{
    // Obtener estado del menú de presets y del processor
    juce::String currentText = presetArea.presetMenu.getText();
    juce::String processorText = processor.getPresetDisplayText();
    
    // Si no hay texto en el menú, usar el texto del processor
    if (currentText.isEmpty() && !processorText.isEmpty()) {
        currentText = processorText;
    }
    
    // Extraer el nombre del preset sin asterisco
    juce::String currentPresetName = currentText.endsWith("*") ? 
        currentText.dropLastCharacters(1).trimEnd() : currentText;
    
    // Si no hay preset o es DEFAULT, ir a Save As
    if (currentPresetName.isEmpty() || currentPresetName == "DEFAULT") {
        saveAsPresetFile();
        return;
    }
    
    // Verificar si es un factory preset
    bool isFactoryPreset = currentPresetName.startsWith("[F] ");
    if (isFactoryPreset) {
        // Quitar el prefijo [F] para mostrar el nombre limpio
        juce::String cleanName = currentPresetName.substring(4);
        showCustomAlertDialog(JUCE_UTF8("Cannot overwrite"), 
                            JUCE_UTF8("Factory Presets are read-only.\nUse 'Save As' to create a copy."));
        return;
    }
    
    // Si hay un preset cargado (modificado o no), ofrecer opciones
    // Mostrar diálogo si hay un preset cargado (modificado O no modificado)
    if (!currentPresetName.isEmpty()) {
        overwritePresetDialog = std::make_unique<CustomThreeButtonDialog>(
            "Overwrite \"" + currentPresetName + "\"",
            "",  // No mensaje adicional, el título es suficiente
            "Overwrite",
            "Save As",
            "Cancel"
        );
        
        overwritePresetDialog->onDialogClosed = [this, currentPresetName](bool wasAccepted) {
            if (wasAccepted) {
                int result = overwritePresetDialog->getSelectedButton();
                if (result == 1) { // Overwrite
                // Guardar directamente en el archivo existente
                auto presetFile = getPresetsFolder().getChildFile(currentPresetName + ".preset");
                auto state = processor.apvts.copyState();
                
                // Añadir estado del botón graphics (no automatizable)
                auto uiSettings = state.getOrCreateChildWithName("UISettings", nullptr);
                uiSettings.setProperty("displayGraphicsEnvelopes", utilityButtons.runGraphicsButton.getToggleState(), nullptr);
                
                std::unique_ptr<juce::XmlElement> xml(state.createXml());
                
                if (xml != nullptr) {
                    xml->writeTo(presetFile);
                    
                    // Actualizar estado visual
                    processor.setPresetDisplayText(currentPresetName);
                    processor.setPresetTextItalic(false);
                    presetArea.presetMenu.setTextWhenNothingSelected(currentPresetName);
                    presetArea.presetMenu.setTextItalic(false);
                    
                    // Actualizar menú si es necesario
                    if (presetArea.presetMenu.getSelectedId() <= 0) {
                        refreshPresetMenu();
                        // No usar setSelectedId para evitar triggerar onChange
                        presetArea.presetMenu.setSelectedId(0);
                        presetArea.presetMenu.setTextWhenNothingSelected(currentPresetName);
                        presetArea.presetMenu.setTextItalic(false);
                        processor.setPresetDisplayText(currentPresetName);
                        processor.setPresetTextItalic(false);
                    }
                }
                } else if (result == 2) { // Save As...
                    saveAsPresetFile();
                }
                // result == 3 is Cancel, do nothing
            }
            
            // Limpiar el diálogo después de usarlo
            overwritePresetDialog.reset();
        };
        
        addChildComponent(overwritePresetDialog.get());
        overwritePresetDialog->showDialog();
    } else {
        // No hay preset cargado, ir directamente a "Save As..."
        saveAsPresetFile();
    }
}

void JCBMaximizerAudioProcessorEditor::saveAsPresetFile()
{
    // Crear y mostrar el diálogo personalizado
    savePresetDialog = std::make_unique<SavePresetDialog>("");
    savePresetDialog->onDialogClosed = [this](bool shouldSave) {
        if (shouldSave) {
            juce::String presetName = savePresetDialog->getPresetName();
            
            // No permitir usar el nombre DEFAULT
            if (presetName.toUpperCase() == "DEFAULT") {
                showCustomAlertDialog("Error", 
                                    "Cannot use the name DEFAULT for a preset.");
                return;
            }
            
            // Verificar si el nombre coincide con algún factory preset
            for (const auto& factoryName : factoryPresetNames) {
                if (presetName.compareIgnoreCase(factoryName) == 0) {
                    showCustomAlertDialog(JUCE_UTF8("Reserved name"), 
                                        JUCE_UTF8("Name reserved for Factory Preset.\nPlease choose another name."));
                    return;
                }
            }
            
            // Crear archivo con el nombre del preset
            juce::File presetFile = getPresetsFolder().getChildFile(presetName + ".preset");
            
            // Si el archivo ya existe, preguntar si sobrescribir
            if (presetFile.existsAsFile()) {
                showCustomConfirmDialog(JUCE_UTF8("Preset already exists"), 
                                      JUCE_UTF8("Do you want to overwrite the existing preset?"), 
                                      [this, presetFile, presetName](bool shouldOverwrite) {
                    if (shouldOverwrite) {
                        // Guardar el preset
                        auto state = processor.apvts.copyState();
                        
                        // Añadir estado del botón graphics (no automatizable)
                        auto uiSettings = state.getOrCreateChildWithName("UISettings", nullptr);
                        uiSettings.setProperty("displayGraphicsEnvelopes", utilityButtons.runGraphicsButton.getToggleState(), nullptr);
                        
                        std::unique_ptr<juce::XmlElement> xml(state.createXml());
                        
                        if (xml != nullptr) {
                            xml->writeTo(presetFile);
                            
                            // Actualizar el menú
                            refreshPresetMenu();
                            
                            // Mostrar el preset recién guardado sin triggerar onChange
                            presetArea.presetMenu.setSelectedId(0);
                            presetArea.presetMenu.setTextWhenNothingSelected(presetName);
                            presetArea.presetMenu.setTextItalic(false);
                            processor.setPresetDisplayText(presetName);
                            processor.setPresetTextItalic(false);
                            processor.setLastPreset(0);
                        }
                    }
                });
            } else {
                // Guardar directamente si no existe
                auto state = processor.apvts.copyState();
                
                // Añadir estado del botón graphics (no automatizable)
                auto uiSettings = state.getOrCreateChildWithName("UISettings", nullptr);
                uiSettings.setProperty("displayGraphicsEnvelopes", utilityButtons.runGraphicsButton.getToggleState(), nullptr);
                
                std::unique_ptr<juce::XmlElement> xml(state.createXml());
                
                if (xml != nullptr) {
                    xml->writeTo(presetFile);
                    
                    // Actualizar el menú
                    refreshPresetMenu();
                    
                    // Mostrar el preset recién guardado sin triggerar onChange
                    presetArea.presetMenu.setSelectedId(0);
                    presetArea.presetMenu.setTextWhenNothingSelected(presetName);
                    presetArea.presetMenu.setTextItalic(false);
                    processor.setPresetDisplayText(presetName);
                    processor.setPresetTextItalic(false);
                    processor.setLastPreset(0);
                }
            }
        }
        
        // Limpiar el diálogo
        savePresetDialog.reset();
    };
    
    addChildComponent(savePresetDialog.get());
    savePresetDialog->showDialog();
}

void JCBMaximizerAudioProcessorEditor::deletePresetFile()
{
    int selectedId = presetArea.presetMenu.getSelectedId();
    juce::String presetName;
    
    if (selectedId > 0) {
        // Usar getText() primero (funciona con categorías)
        presetName = presetArea.presetMenu.getText();
        
        // Si está vacío, intentar con el mapeo de IDs
        if (presetName.isEmpty() && presetIdToNameMap.find(selectedId) != presetIdToNameMap.end()) {
            presetName = presetIdToNameMap[selectedId];
        }
    } else {
        // No hay selección, verificar si hay un preset modificado
        juce::String displayText = presetArea.presetMenu.getTextWhenNothingSelected();
        if (displayText.isEmpty()) {
            displayText = processor.getPresetDisplayText();
        }
        
        if (displayText.isEmpty()) {
            showCustomAlertDialog(JUCE_UTF8("Error"), 
                                JUCE_UTF8("Ningún preset seleccionado."));
            return;
        }
        
        // Quitar asterisco si lo tiene
        presetName = displayText.endsWith("*") ? 
            displayText.dropLastCharacters(1).trimEnd() : displayText;
    }
    
    // No permitir borrar DEFAULT ni factory presets
    if (presetName == "DEFAULT" || presetName.startsWith("[F] ")) {
        juce::String errorMsg = presetName == "DEFAULT" ? 
            JUCE_UTF8("The DEFAULT preset cannot be deleted.") :
            JUCE_UTF8("Factory Presets cannot be deleted.");
        showCustomAlertDialog(JUCE_UTF8("Error"), errorMsg);
        return;
    }
    
    showCustomConfirmDialog(JUCE_UTF8("Delete Preset"), 
                            JUCE_UTF8("The preset \"") + presetName + JUCE_UTF8("\" will be moved to trash."), 
                            [this, presetName](bool confirmed) {
        if (confirmed) {
            juce::File presetFile = getPresetsFolder().getChildFile(presetName + ".preset");
            
            if (presetFile.existsAsFile()) {
                presetFile.moveToTrash();
                
                // Actualizar el menú
                refreshPresetMenu();
                
                // Dejar el menú sin selección y sin texto
                presetArea.presetMenu.setSelectedId(0);
                presetArea.presetMenu.setTextWhenNothingSelected("");
                presetArea.presetMenu.setTextItalic(false);
                processor.setPresetDisplayText("");
                processor.setPresetTextItalic(false);
                processor.setLastPreset(0);
                
                // NO cargar DEFAULT - dejar los parámetros como están
            }
        }
    });
}

void JCBMaximizerAudioProcessorEditor::selectNextPreset()
{
    // Obtener todos los IDs seleccionables (incluye sub-items de categorías)
    auto allIds = presetArea.presetMenu.getAllSelectableIds();
    if (allIds.empty()) return;
    
    int currentId = presetArea.presetMenu.getSelectedId();
    
    // Si no hay selección actual, seleccionar el primer preset
    if (currentId == 0) {
        presetArea.presetMenu.setSelectedId(allIds[0]);
        if (presetArea.presetMenu.onChange) {
            presetArea.presetMenu.onChange();
        }
        return;
    }
    
    // Buscar el ID actual en la lista de todos los IDs
    auto it = std::find(allIds.begin(), allIds.end(), currentId);
    
    // Si encontramos el ID actual, avanzar al siguiente
    if (it != allIds.end()) {
        ++it;
        // Si llegamos al final, ciclar al principio
        if (it == allIds.end()) {
            it = allIds.begin();
        }
        presetArea.presetMenu.setSelectedId(*it);
        if (presetArea.presetMenu.onChange) {
            presetArea.presetMenu.onChange();
        }
    } else {
        // Si no encontramos el ID actual (puede pasar con presets guardados), seleccionar el primero
        presetArea.presetMenu.setSelectedId(allIds[0]);
        if (presetArea.presetMenu.onChange) {
            presetArea.presetMenu.onChange();
        }
    }
}

void JCBMaximizerAudioProcessorEditor::selectPreviousPreset()
{
    // Obtener todos los IDs seleccionables (incluye sub-items de categorías)
    auto allIds = presetArea.presetMenu.getAllSelectableIds();
    if (allIds.empty()) return;
    
    int currentId = presetArea.presetMenu.getSelectedId();
    
    // Si no hay selección actual, seleccionar el último preset
    if (currentId == 0) {
        presetArea.presetMenu.setSelectedId(allIds.back());
        if (presetArea.presetMenu.onChange) {
            presetArea.presetMenu.onChange();
        }
        return;
    }
    
    // Buscar el ID actual en la lista de todos los IDs
    auto it = std::find(allIds.begin(), allIds.end(), currentId);
    
    // Si encontramos el ID actual, retroceder al anterior
    if (it != allIds.end()) {
        if (it == allIds.begin()) {
            // Si estamos al principio, ciclar al final
            presetArea.presetMenu.setSelectedId(allIds.back());
        } else {
            --it;
            presetArea.presetMenu.setSelectedId(*it);
        }
        if (presetArea.presetMenu.onChange) {
            presetArea.presetMenu.onChange();
        }
    } else {
        // Si no encontramos el ID actual, seleccionar el último
        presetArea.presetMenu.setSelectedId(allIds.back());
        if (presetArea.presetMenu.onChange) {
            presetArea.presetMenu.onChange();
        }
    }
}

//==============================================================================
// DIÁLOGOS Y OVERLAYS
//==============================================================================
void JCBMaximizerAudioProcessorEditor::showCustomConfirmDialog(const juce::String& message, 
                                                          const juce::String& subMessage,
                                                          std::function<void(bool)> callback,
                                                          const juce::String& confirmText,
                                                          const juce::String& cancelText)
{
    // Combinar mensaje principal y submensaje si existe
    juce::String fullMessage = message;
    if (subMessage.isNotEmpty()) {
        fullMessage += "\n\n" + subMessage;
    }
    
    // Usar el nuevo diálogo personalizado
    deleteConfirmDialog = std::make_unique<CustomConfirmDialog>("Confirmation", fullMessage, confirmText, cancelText);
    deleteConfirmDialog->onDialogClosed = callback;
    addChildComponent(deleteConfirmDialog.get());
    deleteConfirmDialog->showDialog();
}

void JCBMaximizerAudioProcessorEditor::showCustomAlertDialog(const juce::String& title, const juce::String& message)
{
    // Crear un diálogo de alerta simple con solo OK
    alertDialog = std::make_unique<CustomAlertDialog>(title, message);
    alertDialog->onDialogClosed = [](bool) {}; // Callback vacío
    addChildComponent(alertDialog.get());
    alertDialog->showDialog();
}

void JCBMaximizerAudioProcessorEditor::showCredits()
{
    // Desactivar estados operacionales antes de mostrar créditos (consistencia con DIAGRAM)
    parameterButtons.bypassButton.setToggleState(false, juce::sendNotification);
    parameterButtons.deltaButton.setToggleState(false, juce::sendNotification);
    
    if (creditsOverlay == nullptr)
    {
        // Obtener el formato del plugin desde el processor
        juce::String format = processor.getPluginFormat();
        
        creditsOverlay = std::make_unique<CreditsOverlay>(format);
        creditsOverlay->setBounds(getLocalBounds());
        creditsOverlay->onClose = [this]() {
            hideCredits();
        };
        
        addAndMakeVisible(creditsOverlay.get());
        creditsOverlay->grabKeyboardFocus();
    }
}

void JCBMaximizerAudioProcessorEditor::hideCredits()
{
    if (creditsOverlay != nullptr)
    {
        removeChildComponent(creditsOverlay.get());
        creditsOverlay.reset();
    }
}

//==============================================================================
// SISTEMA DE TOOLTIPS
//==============================================================================
void JCBMaximizerAudioProcessorEditor::updateTodoButtonTexts()
{
    // Update TODO button tooltips using getTooltipText() for consistency
    utilityButtons.hqButton.setTooltip(getTooltipText("hq"));
    utilityButtons.dualMonoButton.setTooltip(getTooltipText("dualmono"));
    utilityButtons.msButton.setTooltip(getTooltipText("ms"));
    topButtons.abStateButton.setTooltip(getTooltipText("abstate"));
    utilityButtons.midiLearnButton.setTooltip(getTooltipText("midilearn"));
}

void JCBMaximizerAudioProcessorEditor::updateAllTooltips()
{
    // Actualizar todos los tooltips de componentes basado en el idioma actual
    
    // Título
    titleLink.setTooltip(getTooltipText("title"));
    
    // Perillas - superiores izquierdas
    leftTopKnobs.thdSlider.setTooltip(getTooltipText("thd"));
    leftTopKnobs.ceilingSlider.setTooltip(getTooltipText("ceiling"));  // NUEVO - tooltip para b_CELLING
    
    // Perillas - lookahead movido a rightTopControls
    rightTopControls.lookaheadSlider.setTooltip(getTooltipText("lookahead"));
    
    // Perillas - superiores derechas
    rightTopControls.detSlider.setTooltip(getTooltipText("detect"));  // NUEVO - tooltip para DET slider
    
    // Perillas - inferiores derechas
    rightBottomKnobs.atkSlider.setTooltip(getTooltipText("attack"));
    rightBottomKnobs.relSlider.setTooltip(getTooltipText("release"));
    rightBottomKnobs.ditherButton.setTooltip(getTooltipText("dither"));    // NUEVO - tooltip para DITHER button
    rightBottomKnobs.dcFilterButton.setTooltip(getTooltipText("dcfilter")); // NUEVO - tooltip para DC FILTER button
    rightBottomKnobs.autorelButton.setTooltip(getTooltipText("autorel"));  // NUEVO - tooltip para AUTOREL button
    parameterButtons.deltaButton.setTooltip(getTooltipText("delta"));
    
    // Sliders de trim y makeup
    trimSlider.setTooltip(getTooltipText("trim"));
    makeupSlider.setTooltip(getTooltipText("makeup"));
    
    // Área de presets
    presetArea.saveButton.setTooltip(getTooltipText("save"));
    presetArea.saveAsButton.setTooltip(getTooltipText("saveas"));
    presetArea.deleteButton.setTooltip(getTooltipText("delete"));
    presetArea.backButton.setTooltip(getTooltipText("back"));
    presetArea.nextButton.setTooltip(getTooltipText("next"));
    
    // Utility buttons
    utilityButtons.undoButton.setTooltip(getTooltipText("undo"));
    utilityButtons.redoButton.setTooltip(getTooltipText("redo"));
    utilityButtons.resetGuiButton.setTooltip(getTooltipText("resetgui"));
    utilityButtons.runGraphicsButton.setTooltip(getTooltipText("graphics"));
    utilityButtons.zoomButton.setTooltip(getTooltipText("zoom"));
    centerButtons.diagramButton.setTooltip(getTooltipText("diagram"));
    utilityButtons.tooltipToggleButton.setTooltip(getTooltipText("tooltiptoggle"));
    utilityButtons.tooltipLangButton.setTooltip(getTooltipText("tooltiplang"));

    // Botones de parámetros
    parameterButtons.bypassButton.setTooltip(getTooltipText("bypass"));

    
    // Update TODO button tooltips
    updateTodoButtonTexts();
    
    // Actualizar tooltip del transfer display
    transferDisplay.setHelpText(getTooltipText("transfer"));
    
    // Botones de utilidad - fila inferior
    utilityButtons.stereoLinkedButton.setTooltip(getTooltipText("link"));
    // CODE button removed
}

juce::String JCBMaximizerAudioProcessorEditor::getTooltipText(const juce::String& key)
{
    if (currentLanguage == TooltipLanguage::Spanish)
    {
        // Spanish tooltips
        if (key == "title") return JUCE_UTF8("JCBMaximizer: limitador/maximizador de audio v1.0.0\nPlugin de audio open source\nClick para créditos");
        if (key == "thd") return JUCE_UTF8("GANANCIA: ganancia de entrada al limitador\nControla el nivel de drive antes del procesamiento\nRango: 0 a 24 dB | Por defecto: 0 dB");
        if (key == "lookahead") return JUCE_UTF8("LOOKAHEAD: anticipación para evitar distorsión\nEvita overshooting en transitorios rápidos\nRango: 0 a 5 ms | Por defecto: 0 ms");
        if (key == "attack") return JUCE_UTF8("ATTACK: tiempo de ataque del limitador\nControla la respuesta ante aumentos de nivel\nRango: 0.01 a 750 ms | Por defecto: 100 ms");
        if (key == "detect") return JUCE_UTF8("DET: modo de detección del limitador\n0.0 = Peak (rápido) | 1.0 = Sliding RMS (suave)\nValores intermedios mezclan ambos modos\nPor defecto: 1.0 (Sliding RMS)");
        if (key == "ceiling") return JUCE_UTF8("CEILING: nivel máximo de salida\nControla el techo de limitación del maximizer\nRango: -60 a 0 dB | Por defecto: -0.3 dB");
        if (key == "dither") return JUCE_UTF8("DITHER: añade dither TPDF de 16 bits independiente por canal\nReduce distorsión de cuantización en niveles bajos\nRango: 0 a 1 | Por defecto: 0 (OFF)");
        if (key == "dcfilter") return JUCE_UTF8("DC FILTER: filtro HPF de 1er orden a 12 Hz\nElimina offset DC pre-ceiling\nRango: OFF/ON | Por defecto: OFF");
        if (key == "autorel") return JUCE_UTF8("AUTOREL: liberación automática adaptativa\nActiva release inteligente basado en el material\nRango: OFF/ON | Por defecto: OFF");
        if (key == "release") return JUCE_UTF8("RELEASE: tiempo de liberación del limitador\nControla la velocidad de regreso después de limitar\nRango: 1 a 1000 ms | Por defecto: 200 ms");
        if (key == "delta") return JUCE_UTF8("DELTA: escucha solo la diferencia\nReproducir la señal procesada menos la original\nEl volumen está normalizado para evitar cambios drásticos");
        if (key == "trim") return JUCE_UTF8("TRIM: ganancia de entrada al limitador\nAjusta el nivel antes del procesamiento\nRango: -12 a +12 dB | Por defecto: 0 dB");
        if (key == "makeup") return JUCE_UTF8("MAKEUP: ganancia de salida POST procesador\nAjusta el nivel final después del limitador\nRango: -12 a +12 dB | Por defecto: 0 dB");
        if (key == "save") return JUCE_UTF8("SAVE: guarda el preset actual\nSobrescribe el preset seleccionado con valores actuales\nNo funciona con DEFAULT");
        if (key == "saveas") return JUCE_UTF8("SAVE AS: guarda como nuevo preset\nCrea un nuevo archivo de preset con los valores actuales\nPermite crear presets personalizados");
        if (key == "delete") return JUCE_UTF8("BORRAR: elimina el preset seleccionado\nRequiere confirmación antes de borrar");
        if (key == "back") return JUCE_UTF8("ANTERIOR: selecciona el preset previo\nNavega hacia atrás en la lista de presets");
        if (key == "next") return JUCE_UTF8("SIGUIENTE: selecciona el próximo preset\nNavega hacia adelante en la lista de presets");
        if (key == "undo") return JUCE_UTF8("DESHACER: revierte el último cambio\nDeshace modificación realizada manualmente por el usuario\nHistorial: hasta 20 pasos");
        if (key == "redo") return JUCE_UTF8("REHACER: aplica el cambio deshecho\nRehace modificación manual previamente revertida\nHistorial: hasta 20 pasos");
        if (key == "resetgui") return JUCE_UTF8("SIZE: cicla entre tamaños de ventana\nActual → Máximo → Mínimo → Actual\nAjuste rápido del tamaño del plugin");
        if (key == "bypass") return JUCE_UTF8("BYPASS: desactiva el procesamiento del plugin\nParámetro global, no automatizable. Transición suave\nRango: OFF/ON | Por defecto: OFF");
        if (key == "graphics") return JUCE_UTF8("GRAPHICS: muestra envolventes en tiempo real\nVisualiza env entrada/salida e histograma expansión\nDesactivar mejora rendimiento en CPUs lentas");
        if (key == "zoom") return JUCE_UTF8("ZOOM: alterna entre vista completa y enfoque en el techo actual\nNormal muestra -60 a 0 dB; Zoom acerca ~30 dB alrededor del ceiling para ver con precisión la limitación");
        if (key == "diagram") return JUCE_UTF8("DIAGRAM: muestra diagrama de bloques del procesador\nDespliega menú con código GenExpr por bloque para copiar");
        if (key == "transfer") return JUCE_UTF8("GRÁFICA: función de transferencia del limitador\nMuestra la curva de limitación y reducción de ganancia\nClick derecho para opciones adicionales");
        if (key == "tooltiptoggle") return JUCE_UTF8("TOOLTIP: muestra/oculta los tooltips de ayuda\nActiva o desactiva las ventanas de ayuda emergentes");
        if (key == "tooltiplang") return JUCE_UTF8("IDIOMA: cambia entre español e inglés.\nAlterna el idioma de los tooltips.");
        if (key == "link") return JUCE_UTF8("STEREO LINKED: siempre activo.\nEl plugin solo funciona en modo stereo linked.\nAmbos canales siempre están vinculados");
        if (key == "hq") return JUCE_UTF8("POR HACER: Habilita oversampling para mayor calidad.");
        if (key == "dualmono") return JUCE_UTF8("POR HACER: Procesa canales L/R independientemente.");
        if (key == "ms") return JUCE_UTF8("POR HACER: Procesa en formato Mid/Side.");
        if (key == "abstate") return JUCE_UTF8("Alterna entre dos configuraciones A/B para comparar ajustes.");
        if (key == "midilearn") return JUCE_UTF8("POR HACER: Asigna control MIDI.");
        if (key == "abcopyatob") return JUCE_UTF8("Copiar A a B");
        if (key == "abcopybtoa") return JUCE_UTF8("Copiar B a A");
    }
    else
    {
        // English tooltips
        if (key == "title") return "JCBMaximizer: audio limiter/maximizer v1.0.0\nOpen source audio plugin\nClick for credits";
        if (key == "thd") return "GAIN: limiter input gain\nControls the drive level before processing\nRange: 0 to 24 dB | Default: 0 dB";
        if (key == "lookahead") return "LOOKAHEAD: anticipation to prevent distortion\nPrevents overshooting on fast transients\nRange: 0 to 5 ms | Default: 0 ms";
        if (key == "attack") return "ATTACK: limiter attack time\nControls response to level increases\nRange: 0.01 to 750 ms | Default: 100 ms";
        if (key == "detect") return "DET: limiter detection mode\n0.0 = Peak (fast) | 1.0 = Sliding RMS (smooth)\nIntermediate values blend both modes\nDefault: 1.0 (Sliding RMS)";
        if (key == "ceiling") return "CEILING: maximum output level\nControls the maximizer's limiting ceiling\nRange: -60 to 0 dB | Default: -0.3 dB";
        if (key == "dither") return "DITHER: adds 16-bit TPDF dither independent per channel\nReduces quantization distortion at low levels\nRange: 0 to 1 | Default: 0 (OFF)";
        if (key == "dcfilter") return "DC FILTER: 1st order HPF at 12 Hz\nRemoves DC offset pre-ceiling\nRange: OFF/ON | Default: OFF";
        if (key == "autorel") return "AUTOREL: adaptive automatic release\nActivates intelligent release based on material\nRange: OFF/ON | Default: OFF";
        if (key == "release") return "RELEASE: limiter release time\nControls return speed after limiting\nRange: 1 to 1000 ms | Default: 200 ms";
        if (key == "delta") return "DELTA: listen to the difference only\nPlays processed signal minus original\nVolume is normalized to avoid drastic changes";
        if (key == "trim") return "TRIM: limiter input gain\nAdjusts level before processing\nRange: -12 to +12 dB | Default: 0 dB";
        if (key == "makeup") return "MAKEUP: output gain POST processor\nAdjusts final level after limiter\nRange: -12 to +12 dB | Default: 0 dB";
        if (key == "save") return "SAVE: save or overwrite preset\nSave new or update current preset";
        if (key == "saveas") return "SAVE AS: save as new preset.\nCreates new preset file with current values.\nAllows creating custom presets";
        if (key == "delete") return "DELETE: remove selected preset\nRequires confirmation before deleting";
        if (key == "back") return "PREVIOUS: select previous preset\nNavigate backwards through preset list";
        if (key == "next") return "NEXT: select next preset\nNavigate forward through preset list";
        if (key == "undo") return "UNDO: revert last change\nUndo modification made manually by the user\nHistory: up to 20 steps";
        if (key == "redo") return "REDO: reapply undone change\nRedo manually made modification previously reverted\nHistory: up to 20 steps";
        if (key == "resetgui") return JUCE_UTF8("SIZE: cycles through window sizes\nCurrent → Maximum → Minimum → Current\nQuick plugin size adjustment");
        if (key == "bypass") return "BYPASS: disables plugin processing\nGlobal parameter, non-automatable. Smooth transition\nRange: OFF/ON | Default: OFF";
        if (key == "graphics") return "GRAPHICS: shows real-time envelopes\nDisplays input/output env and expansion histogram\nDisable to improve performance on slow CPUs";
        if (key == "zoom") return "ZOOM: toggles between full range and a ceiling-focused view\nNormal shows -60 to 0 dB; Zoom magnifies ~30 dB around the ceiling for precise limiting inspection";
        if (key == "diagram") return "DIAGRAM: shows processor block diagram\nDisplays menu with GenExpr code per block for copying";
        if (key == "transfer") return "GRAPH: limiter transfer function\nShows limiting curve and gain reduction\nRight click for additional options";
        if (key == "tooltiptoggle") return "TOOLTIP: show/hide help tooltips.\nEnables or disables popup help windows.";
        if (key == "tooltiplang") return "LANGUAGE: switch between Spanish and English.\nToggles tooltip language.";
        if (key == "link") return "STEREO LINKED: always active.\nPlugin only works in stereo linked mode.\nBoth channels are always linked";
        if (key == "hq") return "TODO: Enables oversampling for higher quality.";
        if (key == "dualmono") return "TODO: Processes L/R channels independently.";
        if (key == "ms") return "TODO: Processes in M/S format.";
        if (key == "abstate") return "Switches between two A/B configurations to compare settings.";
        if (key == "midilearn") return "TODO: Assigns MIDI control.";
        if (key == "abcopyatob") return "Copy A to B";
        if (key == "abcopybtoa") return "Copy B to A";
    }

    return "";
}

//==============================================================================
// HELPERS DE UI
//==============================================================================
void JCBMaximizerAudioProcessorEditor::applyAlphaToMainControls(float alpha)
{
    // Main knobs
    leftTopKnobs.thdSlider.setAlpha(alpha);
    leftTopKnobs.ceilingSlider.setAlpha(alpha);  // NUEVO - alpha para b_CELLING
    
    rightTopControls.lookaheadSlider.setAlpha(alpha);
    rightBottomKnobs.ditherButton.setAlpha(alpha);  // NUEVO - alpha para DITHER button
    rightBottomKnobs.dcFilterButton.setAlpha(alpha); // NUEVO - alpha para DC FILTER button
    
    rightTopControls.detSlider.setAlpha(alpha);  // NUEVO - alpha para DET knob
    
    rightBottomKnobs.atkSlider.setAlpha(alpha);
    rightBottomKnobs.relSlider.setAlpha(alpha);
    rightBottomKnobs.autorelButton.setAlpha(alpha);  // NUEVO - alpha para AUTOREL button
    
    // Transfer display
    transferDisplay.setAlpha(alpha);
    
    // Parameter buttons (not including BYPASS/DELTA which have special handling)
    parameterButtons.deltaButton.setAlpha(alpha);
    
    // Trim and makeup sliders
    trimSlider.setAlpha(alpha);
}

//==============================================================================
// THREAD SAFETY Y AUTOMATIZACIÓN
//==============================================================================
void JCBMaximizerAudioProcessorEditor::queueParameterUpdate(const juce::String& paramID, float normalizedValue)
{
    {
        std::lock_guard<std::mutex> lock(parameterUpdateMutex);
        
        // Check if this parameter is already queued
        auto it = std::find_if(pendingParameterUpdates.begin(), pendingParameterUpdates.end(),
                               [&paramID](const DeferredParameterUpdate& update) {
                                   return update.paramID == paramID;
                               });
        
        if (it != pendingParameterUpdates.end()) {
            // Actualizar entrada existente
            it->normalizedValue = normalizedValue;
        } else {
            // Add new entry
            pendingParameterUpdates.push_back({paramID, normalizedValue});
        }
    }
    
    hasPendingParameterUpdates.store(true);
}

void JCBMaximizerAudioProcessorEditor::processPendingParameterUpdates()
{
    if (!hasPendingParameterUpdates.exchange(false)) {
        return;
    }
    
    std::vector<DeferredParameterUpdate> updates;
    {
        std::lock_guard<std::mutex> lock(parameterUpdateMutex);
        updates = std::move(pendingParameterUpdates);
        pendingParameterUpdates.clear();
    }
    
    // Establecer flag para prevenir transacciones de undo durante procesamiento de queue
    isProcessingQueue = true;
    
    // Process all updates on the message thread
    for (const auto& update : updates) {
        if (auto* param = processor.apvts.getParameter(update.paramID)) {
            // Usar beginChangeGesture/endChangeGesture para prevenir transacciones de undo
            param->beginChangeGesture();
            param->setValueNotifyingHost(update.normalizedValue);
            param->endChangeGesture();
            
            // CRÍTICO: Sincronización directa con Gen~ DSP
            // Esto asegura que los valores del DEFAULT button lleguen al DSP
            // Convertir valor normalizado a valor real usando el rango del parámetro
            auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param);
            if (floatParam) {
                float realValue = floatParam->getNormalisableRange().convertFrom0to1(update.normalizedValue);
                
                if (update.paramID == "d_ATK" || update.paramID == "e_REL") {
                    // Debug info for DEFAULT preset parameter updates
                }
                
                // Llamar al método parameterChanged del processor para sincronizar con Gen~
                processor.parameterChanged(update.paramID, realValue);
            }
            
            // Manejo de AudioParameterBool para sincronización con Gen~
            auto* boolParam = dynamic_cast<juce::AudioParameterBool*>(param);
            if (boolParam) {
                float realValue = update.normalizedValue >= 0.5f ? 1.0f : 0.0f;

                
                // Llamar al método parameterChanged del processor para sincronizar con Gen~
                processor.parameterChanged(update.paramID, realValue);
            }
        }
    }
    
    // Visual feedback (cambios de alpha) se maneja en timerCallback() para updates consistentes
    
    // Limpiar flag después del procesamiento
    isProcessingQueue = false;
}

//==============================================================================
// DIAGRAM Y CODE WINDOW
//==============================================================================

void JCBMaximizerAudioProcessorEditor::showDiagram()
{
    // Thread-safe: usar MessageManager::callAsync para UI pesadas
    juce::Component::SafePointer<JCBMaximizerAudioProcessorEditor> safeThis(this);
    
    juce::MessageManager::callAsync([safeThis]() {
        if (!safeThis) return;
        
        // Crear overlay si no existe
        if (!safeThis->diagramOverlay)
        {
            safeThis->diagramOverlay = std::make_unique<DiagramOverlay>(*safeThis);
        }
        
        // Configurar y mostrar
        safeThis->addChildComponent(safeThis->diagramOverlay.get());
        safeThis->diagramOverlay->setBounds(safeThis->getLocalBounds());
        safeThis->diagramOverlay->setVisible(true);
        safeThis->diagramOverlay->toFront(true);
        safeThis->centerButtons.diagramButton.setToggleState(true, juce::dontSendNotification);
    });
}

void JCBMaximizerAudioProcessorEditor::hideDiagram()
{
    if (diagramOverlay && diagramOverlay->isVisible())
    {
        diagramOverlay->setVisible(false);
        removeChildComponent(diagramOverlay.get());
    }
    centerButtons.diagramButton.setToggleState(false, juce::dontSendNotification);
}

void JCBMaximizerAudioProcessorEditor::hideCodeWindow()
{
    if (codeWindow && codeWindow->isVisible())
    {
        codeWindow->setVisible(false);
        removeChildComponent(codeWindow.get());
    }
}

juce::String JCBMaximizerAudioProcessorEditor::loadCodeFromFile(const juce::String& blockName)
{
    // Thread-safe: usar cache pre-cargado en lugar de leer BinaryData cada vez
    if (codeContentCacheInitialized && codeContentCache.find(blockName) != codeContentCache.end()) {
        return codeContentCache[blockName];
    }
    
    // Fallback si el cache no está inicializado o no se encuentra el bloque
    return "// Code for " + blockName + "\n\n// Content not found in cache.\n// Please report this issue.\n\n// Basic functionality:\n" + getBasicBlockDescription(blockName);
}

void JCBMaximizerAudioProcessorEditor::initializeCodeContentCache()
{
    if (codeContentCacheInitialized) return;
    
    // Pre-cargar todo el contenido de código al inicializar para thread safety
    struct CodeMapping {
        juce::String blockName;
        const char* binaryData;
        int dataSize;
    };
    
    std::vector<CodeMapping> mappings = {
        {"TRIM IN", BinaryData::InputTrim_txt, BinaryData::InputTrim_txtSize},
        //{"TRIM SC", BinaryData::InputTrim_txt, BinaryData::InputTrim_txtSize},  // Mantener por compatibilidad UI
        {"LOOKAHEAD", BinaryData::InputTrim_txt, BinaryData::InputTrim_txtSize},
        {"DETECTOR", BinaryData::Detector_txt, BinaryData::Detector_txtSize},
        {"GAIN CORE", BinaryData::GainCore_txt, BinaryData::GainCore_txtSize},  // Corregido: era GainCalc
        //{"MAKEUP", BinaryData::Output_txt, BinaryData::Output_txtSize},
        {"OUTPUT", BinaryData::Output_txt, BinaryData::Output_txtSize},
        {"DELTA", BinaryData::GainCore_txt, BinaryData::GainCore_txtSize}
    };

    
    // Cargar todo en cache
    for (const auto& mapping : mappings) {
        if (mapping.binaryData != nullptr && mapping.dataSize > 0) {
            juce::String content = juce::String::createStringFromData(mapping.binaryData, mapping.dataSize);
            if (content.isNotEmpty()) {
                codeContentCache[mapping.blockName] = content;
            } else {
                // Fallback si falla la carga
                codeContentCache[mapping.blockName] = "// Code for " + mapping.blockName + 
                    "\n\n// Error cargando contenido\n\n// Funcionalidad básica:\n" + 
                    getBasicBlockDescription(mapping.blockName);
            }
        }
    }
    
    codeContentCacheInitialized = true;
}

juce::String JCBMaximizerAudioProcessorEditor::getBasicBlockDescription(const juce::String& blockName)
{
    if (blockName == "TRIM IN") {
        return "// Input trim gain applied to main signal\ninput_trimmed = input * dbtoa(trim_db);";
    } else if (blockName == "TRIM SC" || blockName == "FILTERS") {
        return "// Block not used in Maximizer";
    } else if (blockName == "DETECTOR") {
        return "// Level detector producing control envelope\nlevel = detector(filtered_sc, react_param);";
    } else if (blockName == "GAIN CALC") {
        return "// Compressor gain calculation with ratio and knee\ngain_reduction = compressor_curve(level, threshold, ratio, knee);";
    } else if (blockName == "APPLY") {
        return "// Apply gain reduction to main signal\ncompressed = input_trimmed * gain_reduction;";
    } else if (blockName == "MAKEUP") {
        return "// Apply makeup gain to compressed signal\nwith_makeup = compressed * dbtoa(makeup_db);";
    } else if (blockName == "PARALLEL") {
        return "// Parallel compression mixing\noutput = mix(input_trimmed, with_makeup, parallel_amount);";
    } else if (blockName == "OUTPUT") {
        return "// Final output with any additional processing\nfinal_output = output_stage(parallel_output);";
    } else {
        return "// Generic Gen~ block processing\n// See Max patch for detailed implementation";
    }
}

// Función helper para obtener índice de parámetro por ID (robusta, compatible con el futuro)
int JCBMaximizerAudioProcessorEditor::getParameterIndexByID(const juce::String& parameterID)
{
    auto& params = processor.getParameters();
    for (int i = 0; i < params.size(); ++i) {
        if (auto* param = dynamic_cast<juce::AudioProcessorParameterWithID*>(params[i])) {
            if (param->paramID == parameterID) {
                return i;
            }
        }
    }
    return -1; // Parámetro no encontrado
}

int JCBMaximizerAudioProcessorEditor::getControlParameterIndex(juce::Component& control)
{
    juce::String parameterID;

    if (&control == &leftTopKnobs.thdSlider) parameterID = "a_GAIN";
    else if (&control == &leftTopKnobs.ceilingSlider) parameterID = "b_CELLING";
    else if (&control == &rightTopControls.lookaheadSlider) parameterID = "n_LOOKAHEAD";
    else if (&control == &rightTopControls.detSlider) parameterID = "l_DETECT";
    else if (&control == &rightBottomKnobs.atkSlider) parameterID = "d_ATK";
    else if (&control == &rightBottomKnobs.relSlider) parameterID = "e_REL";
    else if (&control == &makeupSlider) parameterID = "i_MAKEUP";
    else if (&control == &trimSlider) parameterID = "j_TRIM";
    else if (&control == &rightBottomKnobs.ditherButton) return -1;
    else if (&control == &rightBottomKnobs.dcFilterButton) return -1;
    else if (&control == &rightBottomKnobs.autorelButton) return -1;
    else if (&control == &parameterButtons.deltaButton) return -1;
    else if (&control == &parameterButtons.bypassButton) return -1;

    if (parameterID.isNotEmpty())
        return getParameterIndexByID(parameterID);

    return -1;
}

void JCBMaximizerAudioProcessorEditor::applyMeterDecayIfNeeded()
{
    // Sistema universal de decay para todos los DAWs
    // Basado en la solución recomendada en los foros de JUCE
    // Implementación profesional: decay siempre activo
    
    // Los medidores decaen naturalmente por inactividad
    // Los repaints se manejan automáticamente por el timer normal del editor
}

void JCBMaximizerAudioProcessorEditor::updateARButtonText()
{
}

void JCBMaximizerAudioProcessorEditor::updateRelSliderAlpha()
{
    // Verificar si AUTOREL está activo
    if (auto* param = processor.apvts.getRawParameterValue("m_AUTOREL"))
    {
        bool autorelActive = param->load() >= 0.5f;
        
        if (autorelActive)
        {
            // AUTOREL activo - reducir alpha del REL slider para indicar que está siendo controlado automáticamente
            rightBottomKnobs.relSlider.setAlpha(0.4f);
        }
        else
        {
            // AUTOREL inactivo - alpha normal para REL slider
            rightBottomKnobs.relSlider.setAlpha(1.0f);
        }
    }
}
