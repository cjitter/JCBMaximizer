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

// Librerías estándar C++
#include <unordered_map>

// Archivos del proyecto
#include "PluginProcessor.h"
#include "BinaryData.h"
#include "Components/UI/TransferFunctionDisplay.h"
#include "Components/UI/GradientMeter.h"
#include "Components/UI/CustomTooltip.h"
#include "Components/UI/TrimSlider.h"
#include "Components/UI/CustomSlider.h"
#include "Components/UI/CustomComboBox.h"
#include "Components/Windows/CustomDialog.h"
#include "Components/Windows/UndoableButtonAttachment.h"
#include "Components/Windows/CodeWindow.h"
#include "Components/Windows/DarkThemeColors.h"

// Forward declaration para CreditsOverlay (usado en std::unique_ptr<CreditsOverlay> creditsOverlay)
class CreditsOverlay;

//==============================================================================
// CLASE PRINCIPAL DEL EDITOR
//==============================================================================
class JCBMaximizerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          public juce::Timer,
                                          public juce::Button::Listener
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    JCBMaximizerAudioProcessorEditor (JCBMaximizerAudioProcessor&, juce::UndoManager& um);
    ~JCBMaximizerAudioProcessorEditor() override;

    //==========================================================================
    // OVERRIDES DE JUCE
    //==========================================================================
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void buttonClicked(juce::Button* button) override;
    
    //==========================================================================
    // SOPORTE DE AUTOMATIZACIÓN PRO TOOLS
    //==========================================================================
    int getControlParameterIndex(juce::Component& control) override;
    int getParameterIndexByID(const juce::String& parameterID);
    
    //==========================================================================
    // INTERFAZ PÚBLICA
    //==========================================================================
    void updateTransferFunctionFromProcessor() { updateTransferDisplay(); }
    bool getIsLoadingPreset() const noexcept { return isLoadingPreset; }
    JCBMaximizerAudioProcessor& getProcessor() const noexcept { return processor; }
    
private:
    //==========================================================================
    // REFERENCIAS PRINCIPALES
    //==========================================================================
    JCBMaximizerAudioProcessor& processor;
    juce::UndoManager& undoManager;
    
    //==========================================================================
    // CONFIGURACIÓN Y CONSTANTES
    //==========================================================================
    
    // Sistema de coordenadas de referencia fijo
    static constexpr int REFERENCE_WIDTH = 700;
    static constexpr int REFERENCE_HEIGHT = 200;
    
    // Constantes de diseño
    static constexpr int DEFAULT_WIDTH = 1260;  // Cambiado para obtener ratio exacto de 3.5
    static constexpr int DEFAULT_HEIGHT = 360;   // 1260/360 = 3.5
    static constexpr float ASPECT_RATIO = static_cast<float>(REFERENCE_WIDTH) / static_cast<float>(REFERENCE_HEIGHT); // 3.5
    static constexpr int MIN_WIDTH = 1187;  // 95% de default
    static constexpr int MIN_HEIGHT = 339;  // 95% de default
    static constexpr int MAX_WIDTH = 1437;  // 115% de default
    static constexpr int MAX_HEIGHT = 410;  // 115% de default
    static constexpr int TIMER_HZ = 60;
    
    // Debouncing y timing
    static constexpr juce::uint32 DIAGRAM_BUTTON_DEBOUNCE_MS = 200;
    static constexpr juce::uint32 MIN_UPDATE_INTERVAL_MS = 16; // ~60fps max update
    
    //==========================================================================
    // ENUMS Y TIPOS
    //==========================================================================
    
    // Estado de reset GUI - ciclar entre 3 tamaños
    enum class GuiSizeState { Current, Maximum, Minimum };
    
    // Sistema de idiomas para tooltips
    enum class TooltipLanguage { Spanish, English };
    
    // Sistema de actualización diferida de parámetros para thread safety con AAX
    struct DeferredParameterUpdate {
        juce::String paramID;
        float normalizedValue;
    };
    
    //==========================================================================
    // CLASES LOOK AND FEEL
    //==========================================================================
    
    // LookAndFeel personalizado para botones pequeños
    class SmallButtonLAF : public juce::LookAndFeel_V4
    {
    public:
        SmallButtonLAF() {}
        
        juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
        {
            // Coincidir con estilo de etiqueta de knob: tamaño dinámico basado en altura del botón + negrita
            float fontSize = static_cast<float>(buttonHeight) * 0.6f;
            return juce::Font(juce::FontOptions(fontSize)).withStyle(juce::Font::bold);
        }
        
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                                const juce::Colour& backgroundColour,
                                bool shouldDrawButtonAsHighlighted,
                                bool shouldDrawButtonAsDown) override
        {
            // Dibujar solo fondo sin bordes
            g.setColour(backgroundColour);
            g.fillRoundedRectangle(button.getLocalBounds().toFloat(), 3.0f);
        }
    };
    
    //==========================================================================
    // LISTENERS ESPECIALIZADOS
    //==========================================================================
    
    // Listener de parámetros para actualizaciones de automatización
    struct TransferFunctionParameterListener : public juce::AudioProcessorValueTreeState::Listener
    {
        TransferFunctionParameterListener(JCBMaximizerAudioProcessorEditor* e) : editor(e) {}
        
        void parameterChanged(const juce::String& parameterID, float /*newValue*/) override
        {
            if (parameterID == "a_GAIN" || parameterID == "b_CELLING")
            {
                // Usar SafePointer para thread safety
                juce::Component::SafePointer<JCBMaximizerAudioProcessorEditor> safeEditor(editor);
                
                juce::MessageManager::callAsync([safeEditor]() {
                    if (safeEditor)
                        safeEditor->updateTransferDisplay();
                });
            }
        }
        
        JCBMaximizerAudioProcessorEditor* editor;
    };
    
    // Parameter listener para actualizar alpha del REL slider cuando AUTOREL cambia
    struct AutorelParameterListener : public juce::AudioProcessorValueTreeState::Listener
    {
        AutorelParameterListener(JCBMaximizerAudioProcessorEditor* e) : editor(e) {}
        
        void parameterChanged(const juce::String& parameterID, float /*newValue*/) override
        {
            if (parameterID == "m_AUTOREL")
            {
                // Usar SafePointer para thread safety
                juce::Component::SafePointer<JCBMaximizerAudioProcessorEditor> safeEditor(editor);
                
                juce::MessageManager::callAsync([safeEditor]() {
                    if (safeEditor)
                        safeEditor->updateRelSliderAlpha();
                });
            }
        }
        
        JCBMaximizerAudioProcessorEditor* editor;
    };
    
    // Parameter listener para manejar cambios de DELTA y sincronizar estado de UI
    struct DeltaParameterListener : public juce::AudioProcessorValueTreeState::Listener
    {
        DeltaParameterListener(JCBMaximizerAudioProcessorEditor* e) : editor(e) {}
        
        void parameterChanged(const juce::String& parameterID, float newValue) override
        {
            if (parameterID == "k_DELTA")
            {
                // Usar SafePointer para thread safety
                juce::Component::SafePointer<JCBMaximizerAudioProcessorEditor> safeEditor(editor);
                bool deltaActive = newValue >= 0.5f;
                
                juce::MessageManager::callAsync([safeEditor, deltaActive]() {
                    if (safeEditor)
                        safeEditor->applyDeltaModeToAllControls(deltaActive);
                });
            }
        }
        
        JCBMaximizerAudioProcessorEditor* editor;
    };
    
    //==========================================================================
    // COMPONENTES DE DISPLAY PRINCIPALES
    //==========================================================================
    
    // Componente de visualización principal
    TransferFunctionDisplay transferDisplay;
    
    //==========================================================================
    // COMPONENTES DE METERS
    //==========================================================================
    
    // Posicionamiento de medidores
    GradientMeter inputMeterL, inputMeterR;
    GradientMeterInv grMeter;  // Mantener GradientMeterInv - muestra reducción desde arriba que es correcto para compresor
    GradientMeterOutput outputMeterL, outputMeterR;
    
    
    //==========================================================================
    // SLIDERS DE TRIM (superpuestos a meters)
    //==========================================================================
    
    // Sliders de Trim que se superponen a los medidores
    TrimSlider trimSlider;      // Slider único para ambos canales L/R
    TrimSlider makeupSlider;    // RESTAURADO: i_MAKEUP - Makeup gain POST procesador
    std::unique_ptr<CustomSliderAttachment> trimAttachment;
    std::unique_ptr<CustomSliderAttachment> makeupAttachment;  // RESTAURADO: para i_MAKEUP
    
    
    //==========================================================================
    // DISPLAYS DE VALOR INDEPENDIENTES
    //==========================================================================
    
    
    //==========================================================================
    // KNOBS DE CONTROL (organizados por ubicación visual)
    //==========================================================================
    
    // Controles izquierdos - fila superior
    struct LeftTopKnobs {
        CustomSlider thdSlider{"thd"};
        CustomSlider ceilingSlider{"ceiling"};  // NUEVO - parámetro b_CELLING específico del Maximizer
        
        std::unique_ptr<CustomSliderAttachment> thdAttachment;
        std::unique_ptr<CustomSliderAttachment> ceilingAttachment;  // NUEVO - attachment para b_CELLING
    } leftTopKnobs;

    
    // Controles derechos - fila superior
    struct RightTopControls {
        CustomSlider detSlider{"detect"};  // NUEVO - parámetro l_DETECT (Peak/RMS detection)
        CustomSlider lookaheadSlider{"lookahead"};  // MOVIDO desde LeftBottomKnobs para mejor distribución visual
        
        std::unique_ptr<CustomSliderAttachment> detAttachment;  // NUEVO - attachment para l_DETECT
        std::unique_ptr<CustomSliderAttachment> lookaheadAttachment;  // MOVIDO desde LeftBottomKnobs
    } rightTopControls;
    
    // Controles derechos, fila inferior
    struct RightBottomKnobs {
        CustomSlider atkSlider{"attack"};
        CustomSlider relSlider{"release"};
        juce::TextButton autorelButton{"AUTOREL"};  // NUEVO - parámetro m_AUTOREL
        juce::TextButton ditherButton{"DITHER"};  // MOVIDO desde LeftBottomKnobs para consistencia con posición visual
        juce::TextButton dcFilterButton{"DC FILTER"};  // NUEVO - filtro DC offset post-procesamiento
        
        std::unique_ptr<CustomSliderAttachment> atkAttachment;
        std::unique_ptr<CustomSliderAttachment> relAttachment;
        std::unique_ptr<UndoableButtonAttachment> autorelAttachment;  // NUEVO - attachment para m_AUTOREL
        std::unique_ptr<UndoableButtonAttachment> ditherAttachment;  // MOVIDO desde LeftBottomKnobs para consistencia con posición visual
        std::unique_ptr<UndoableButtonAttachment> dcFilterAttachment;  // NUEVO - attachment para o_DCFILT
    } rightBottomKnobs;
    
    //==========================================================================
    //==========================================================================
    
    
    
    //==========================================================================
    // GRUPOS DE BUTTONS (organizados por función y ubicación)
    //==========================================================================
    
    // Área de presets en la parte superior
    struct PresetArea {
        juce::TextButton saveButton{"save"};
        juce::TextButton saveAsButton{"save as"};
        juce::TextButton deleteButton{"delete"};
        juce::TextButton backButton{"<"};
        juce::TextButton nextButton{">"};
        CustomComboBox presetMenu;
    } presetArea;
    
    // Botones superiores junto a presets (y=15)
    struct TopButtons {
        juce::TextButton abStateButton{"A/B"};
        juce::TextButton abCopyButton{"A-B"};
    } topButtons;
    
    // Botón central inferior (y=163)  
    struct CenterButtons {
        juce::TextButton diagramButton{"DIAGRAM"};  // Diagrama de bloques
    } centerButtons;
    
    // Botones de utilidad en la parte inferior izquierda (y=174)
    struct UtilityButtons {
        juce::TextButton undoButton{"undo"};
        juce::TextButton redoButton{"redo"};
        juce::TextButton resetGuiButton{"size"};
        juce::TextButton runGraphicsButton{"graphics"};  // Cambiado a TextButton
        juce::TextButton zoomButton{"zoom"};  // Zoom de gráficos
        juce::TextButton tooltipToggleButton{"tooltip"};  // Alternar visibilidad de tooltip
        juce::TextButton tooltipLangButton{"esp"};  // Alternar idioma ESP/ENG
        
        // Botones sin implementar para inferior derecha
        juce::TextButton hqButton{"HQ"};  // Sobremuestreo
        juce::TextButton dualMonoButton{"DUAL"};  // Modo Dual Mono
        juce::TextButton stereoLinkedButton{"LINK"};  // Modo Stereo Linked (por defecto)
        juce::TextButton msButton{"M/S"};  // Modo Mid/Side
        juce::TextButton midiLearnButton{"MIDI"};  // Aprendizaje MIDI
    } utilityButtons;
    
    // Botones de parámetros en la parte inferior derecha
    struct ParameterButtons {
        juce::TextButton deltaButton{"DELTA"};
        juce::TextButton bypassButton{"BYPASS"};
        
        std::unique_ptr<UndoableButtonAttachment> deltaAttachment;
        std::unique_ptr<UndoableButtonAttachment> bypassAttachment;
    } parameterButtons;
    
    //==========================================================================
    // BACKGROUND E IMAGES
    //==========================================================================
    
    // Título y versión en la parte inferior
    juce::TextButton titleLink{"JCBMaximizer v1.0.1"};
    
    // Imágenes de fondo
    juce::ImageComponent backgroundImage;
    juce::Image normalBackground;
    juce::Image bypassBackground;
    juce::Image deltaBackground;
    juce::Image diagramBackground;
    
    // Iconos de filtro
    
    //==========================================================================
    // COMPONENTES DE OVERLAY Y DIALOG
    //==========================================================================
    
    // Tooltip
    CustomTooltip tooltipComponent;
    
    // Diálogos personalizados
    std::unique_ptr<SavePresetDialog> savePresetDialog;
    std::unique_ptr<CustomConfirmDialog> deleteConfirmDialog;
    std::unique_ptr<CustomAlertDialog> alertDialog;
    std::unique_ptr<CustomThreeButtonDialog> overwritePresetDialog;
    
    // Superposición de créditos
    std::unique_ptr<CreditsOverlay> creditsOverlay;
    
    // Ventana de código para mostrar código de bloque desde DIAGRAM
    std::unique_ptr<CodeWindow> codeWindow;
    
    // Componentes de superposición del diagrama
    class DiagramOverlay : public juce::Component
    {
    public:
        DiagramOverlay(JCBMaximizerAudioProcessorEditor& editor) : owner(editor)
        {
            setInterceptsMouseClicks(true, true);
            setAlwaysOnTop(false);  // Cambiado a false para permitir que el código aparezca encima
            setWantsKeyboardFocus(true);  // No necesita foco, no maneja eventos de teclado
            
            // Cargar la imagen del diagrama PNG
            diagramImage = juce::ImageCache::getFromMemory(BinaryData::diagram_png,
                                                           BinaryData::diagram_pngSize);
            
            // Cargar la imagen de fondo del diagrama
            backgroundImage = juce::ImageCache::getFromMemory(BinaryData::diagramaFondo_png, 
                                                             BinaryData::diagramaFondo_pngSize);
        }
        
        bool keyPressed(const juce::KeyPress& key) override
        {
            if (key == juce::KeyPress::escapeKey)
            {
                owner.hideDiagram();
                return true;
            }
            return false;
        }
        
        void paint(juce::Graphics& g) override
        {
            // Dibujar la imagen de fondo directamente
            if (backgroundImage.isValid())
            {
                g.drawImage(backgroundImage, getLocalBounds().toFloat());
            }
            else
            {
                // Fallback: fondo sólido si no se carga la imagen
                g.fillAll(juce::Colours::black);
            }
            
            // Obtener el área del diagrama
            auto diagramBounds = getDiagramBounds();
            
            // Dibujar la imagen del diagrama PNG
            if (diagramImage.isValid())
            {
                // Dibujar el PNG centrado manteniendo proporción
                g.drawImage(diagramImage, diagramBounds.toFloat(),
                            juce::RectanglePlacement::centred | 
                            juce::RectanglePlacement::onlyReduceInSize);
            }

            // Dibujar glow effect sobre bloques hovered
            drawHoverGlow(g);

            // Dibujar botón de cierre en la esquina superior derecha
            auto closeBounds = getLocalBounds().removeFromTop(40).removeFromRight(100).reduced(5);
            g.setColour(juce::Colours::red.withAlpha(0.8f));
            g.fillRoundedRectangle(closeBounds.toFloat(), 4.0f);
            g.setColour(juce::Colours::white);
            g.setFont(14.0f);
            g.drawText("Close [ESC]", closeBounds, juce::Justification::centred);
        }
        
        void drawBlockLabels(juce::Graphics& g, const juce::Rectangle<int>& diagramBounds)
        {
            // Configurar fuente
            g.setFont(12.0f);
            g.setColour(juce::Colours::white);
            
            // Calcular posiciones relativas al área del diagrama
            float x = diagramBounds.getX();
            float y = diagramBounds.getY();
            float w = diagramBounds.getWidth();
            float h = diagramBounds.getHeight();
            
            // Estructura para almacenar etiquetas y sus posiciones normalizadas
            struct Label {
                juce::String text;
                float normX, normY;
                float width, height;
            };
            
            // Definir las etiquetas basándose en el diagrama
            std::vector<Label> labels = {
                // Entradas
                {"IN L", 0.025f, 0.238f, 0.041f, 0.071f},
                {"IN R", 0.025f, 0.333f, 0.041f, 0.071f},
                //{"SC L", 0.025f, 0.476f, 0.041f, 0.071f},
                //{"SC R", 0.025f, 0.571f, 0.041f, 0.071f},
                
                // Bloques principales
                {"TRIM IN", 0.106f, 0.262f, 0.049f, 0.142f},
                //{"TRIM SC", 0.106f, 0.476f, 0.049f, 0.142f},
                
                // Procesamiento
                //{"FILTERS", 0.335f, 0.762f, 0.065f, 0.238f},
                {"DETECTOR", 0.417f, 0.661f, 0.090f, 0.434f},
                //{"GAIN CALC", 0.531f, 0.750f, 0.100f, 0.262f},
                {"APPLY", 0.534f, 0.357f, 0.094f, 0.214f},
                
                // Salida
                //{"MAKEUP", 0.653f, 0.363f, 0.090f, 0.202f},
                //{"PARALLEL", 0.767f, 0.363f, 0.090f, 0.202f},
                {"OUTPUT", 0.873f, 0.369f, 0.090f, 0.190f},
                
                // Salidas finales
                {"OUT L", 0.996f, 0.303f, 0.068f, 0.071f},
                {"OUT R", 0.996f, 0.554f, 0.068f, 0.071f},
                
                // Medidores
                {"GR METER", 0.669f, 0.810f, 0.082f, 0.143f}
            };
            
            // Dibujar cada etiqueta
            for (const auto& label : labels)
            {
                float labelX = x + (w * label.normX);
                float labelY = y + (h * label.normY);
                float labelW = w * label.width;
                float labelH = h * label.height;
                
                juce::Rectangle<float> labelBounds(labelX, labelY, labelW, labelH);
                
                // Ajustar fuente según el tamaño del bloque
                if (label.width < 0.05f) {
                    g.setFont(10.0f);
                } else if (label.width < 0.08f) {
                    g.setFont(11.0f);
                } else {
                    g.setFont(12.0f);
                }
                
                g.drawText(label.text, labelBounds, juce::Justification::centred);
            }
        }
        
        void mouseMove(const juce::MouseEvent& event) override
        {
            // Verificar si el mouse está sobre un bloque clickeable
            auto blockName = getBlockAtPosition(event.position);
            
            // Actualizar estado de hover si cambió
            if (blockName != hoveredBlockName)
            {
                hoveredBlockName = blockName;
                isMouseOverClickableArea = blockName.isNotEmpty();
                repaint(); // Trigger repaint para actualizar efecto glow
            }
            
            if (blockName.isNotEmpty())
            {
                // Cambiar cursor a mano
                setMouseCursor(juce::MouseCursor::PointingHandCursor);
            }
            else
            {
                // Restaurar cursor normal
                setMouseCursor(juce::MouseCursor::NormalCursor);
            }
        }
        
        void mouseExit(const juce::MouseEvent& /*event*/) override
        {
            // Limpiar estado de hover cuando el mouse sale del diagrama
            if (isMouseOverClickableArea || hoveredBlockName.isNotEmpty())
            {
                hoveredBlockName = "";
                isMouseOverClickableArea = false;
                setMouseCursor(juce::MouseCursor::NormalCursor);
                repaint(); // Trigger repaint para quitar efecto glow
            }
        }
        
        void mouseDown(const juce::MouseEvent& event) override
        {
            // Si hay una ventana de código visible, verificar si el click es fuera de ella para cerrarla
            if (owner.codeWindow != nullptr && owner.codeWindow->isVisible())
            {
                if (!owner.codeWindow->getBounds().contains(event.position.toInt()))
                {
                    owner.hideCodeWindow();
                    return;
                }
            }
            
            // Verificar si se hizo click en el botón de cierre
            auto closeBounds = getLocalBounds().removeFromTop(40).removeFromRight(100).reduced(5);
            if (closeBounds.contains(event.position.toInt()))
            {
                owner.hideDiagram();
                return;
            }
            
            // Verificar si se hizo click en un bloque para mostrar código
            auto blockName = getBlockAtPosition(event.position);
            if (blockName.isNotEmpty())
            {
                // Thread-safe: mover operaciones pesadas a MessageManager::callAsync
                juce::Component::SafePointer<JCBMaximizerAudioProcessorEditor> safeOwner(&owner);
                juce::MessageManager::callAsync([safeOwner, blockName]() {
                    if (!safeOwner) return;  // Componente fue eliminado
                    
                    // Crear CodeWindow si no existe (solo en message thread)
                    if (safeOwner->codeWindow == nullptr)
                    {
                        safeOwner->codeWindow = std::make_unique<CodeWindow>();
                    }
                    
                    // Cargar código desde cache thread-safe
                    juce::String genCode = safeOwner->loadCodeFromFile(blockName);
                    
                    // Determinar título de ventana: usar "OUTPUT" para bloques específicos
                    juce::String windowTitle = blockName;
                    if (blockName == "LOOKAHEAD" || blockName == "MAKEUP" || 
                        blockName == "OUTPUT") {
                        windowTitle = "OUTPUT";
                    }
                    else if (blockName == "DELTA") {
                        windowTitle = "GAIN CORE";
                    }
                    
                    safeOwner->codeWindow->setCode(genCode, windowTitle);
                    
                    // Configurar colores: fondo negro sólido, texto blanco
                    safeOwner->codeWindow->setHaloColour(juce::Colours::white);
                    
                    // Configurar callback de cierre
                    safeOwner->codeWindow->onClose = [safeOwner]() {
                        if (safeOwner) safeOwner->hideCodeWindow();
                    };
                    
                    // Calcular tamaño responsivo
                    int pluginWidth = safeOwner->getWidth();
                    int pluginHeight = safeOwner->getHeight();
                    
                    int windowWidth = static_cast<int>(pluginWidth * 0.35f);
                    int windowHeight = static_cast<int>(pluginHeight * 0.50f);
                    
                    // Limitar tamaños
                    windowWidth = juce::jlimit(350, 600, windowWidth);
                    windowHeight = juce::jlimit(250, 450, windowHeight);
                    
                    int x = pluginWidth / 2 - windowWidth / 2;
                    int y = pluginHeight / 2 - windowHeight / 2;
                    
                    safeOwner->addChildComponent(safeOwner->codeWindow.get());
                    safeOwner->codeWindow->setBounds(x, y, windowWidth, windowHeight);
                    safeOwner->codeWindow->setVisible(true);
                    safeOwner->codeWindow->toFront(true);
                    safeOwner->codeWindow->grabKeyboardFocus();
                });
            }
        }
        
        // Método público para invalidar cache cuando el plugin se redimensiona
        void invalidateClickableAreasCache()
        {
            clickableAreasCached = false;
            // Limpiar estado de hover para evitar problemas
            hoveredBlockName = "";
            isMouseOverClickableArea = false;
        }
        
    private:
        
        JCBMaximizerAudioProcessorEditor& owner;
        juce::Image diagramImage;
        juce::Image backgroundImage;
        
        // Cache para optimizar performance - coordenadas relativas al sistema de referencia
        struct ClickableArea {
            juce::String blockName;
            float x, y, w, h;  // Coordenadas relativas al sistema de referencia (se escalan automáticamente)
        };
        std::vector<ClickableArea> cachedClickableAreas;
        bool clickableAreasCached = false;
        
        // Seguimiento de estado de hover para efecto glow
        juce::String hoveredBlockName;
        bool isMouseOverClickableArea = false;
        
        juce::String getBlockAtPosition(const juce::Point<float>& position)
        {
            // Inicializar cache una sola vez para optimizar performance
            if (!clickableAreasCached)
            {
                initializeClickableAreas();
            }
            
            int mouseX = (int)position.x;
            int mouseY = (int)position.y;
            
            // Verificar contra áreas escaladas usando getScaledBounds del owner
            for (const auto& area : cachedClickableAreas)
            {
                // Convertir coordenadas relativas a absolutas usando getScaledBounds
                auto scaledBounds = owner.getScaledBounds(area.x, area.y, area.w, area.h);
                
                if (mouseX >= scaledBounds.getX() && mouseX <= scaledBounds.getRight() &&
                    mouseY >= scaledBounds.getY() && mouseY <= scaledBounds.getBottom())
                {
                    return area.blockName;
                }
            }
            
            return {};
        }
        
        void initializeClickableAreas()
        {
            if (clickableAreasCached) return;
            
            // Áreas clickables en coordenadas del sistema de referencia
            cachedClickableAreas = {
                // Sección de entrada
                {"TRIM IN", 75.f, 87.f, 55.6f, 33.0f},
                //{"TRIM SC", 77.8f, 122.2f, 55.6f, 30.6f},
                
                // Procesamiento principal
                {"LOOKAHEAD", 170.8f, 16.7f, 430.7f, 13.9f},
                //{"FILTERS", 192.8f, 100.0f, 52.8f, 69.4f},
                {"DETECTOR", 215.0f, 112.0f, 120.0f, 57.0f},
                //{"GAIN CALC", 316.1f, 100.0f, 55.6f, 69.4f},
                {"GAIN CORE", 335.2f, 50.0f, 97.0f, 45.0f},
                
                // Sección de salida
                //{"MAKEUP", 468.0f, 54.4f, 50.4f, 33.3f},
                //{"PARALLEL", 461.1f, 54.4f, 76.1f, 33.3f},
                {"OUTPUT", 540.0f, 54.4f, 82.2f, 33.3f},
                {"DELTA", 480.4f, 110.6f, 47.8f, 40.4f}
            };
            
            clickableAreasCached = true;
        }
        
        // Obtener color específico por bloque para efectos hover dinámicos
        juce::Colour getBlockColor(const juce::String& blockName)
        {
            // Mapeo de bloques a colores temáticos basado en función:
            // Verde: Parámetros de detector (ATK, REL, HOLD, REACT, SMO) y modo DELTA
            // Púrpura: Parámetros de ganancia (THD, RATIO, KNEE, RANGE) y parallel
            // Azul: Procesamiento core, salida, makeup y temporal (lookahead)
            // Gris: Controles de nivel (trim)
            
            if (blockName == "DETECTOR")
                return juce::Colour(0xFF1DB954);  // Verde Spotify (parámetros ATK, REL, HOLD, REACT, SMO)
            else if (blockName == "GAIN CALC")
                return DarkTheme::accentSecondary;  // Púrpura (parámetros THD, RATIO, KNEE, RANGE)
            else if (blockName == "GAIN CORE" || blockName == "MAKEUP" || blockName == "OUTPUT" || blockName == "LOOKAHEAD")
                return DarkTheme::accent;  // Azul (procesamiento core, salida, temporal)
            else if (blockName == "FILTERS")
                return DarkTheme::textPrimary;
            else if (blockName == "PARALLEL")
                return DarkTheme::accentSecondary;  // Púrpura (parallel)
            else if (blockName == "TRIM IN" || blockName == "TRIM SC")
                return DarkTheme::textSecondary;  // Gris claro (controles de nivel)
            else if (blockName == "DELTA")
                return DarkTheme::accent;  // Azul (mismo que GAIN CORE, OUTPUT, LOOKAHEAD)
            else
                return juce::Colours::lightblue;  // Fallback al color original
        }
        
        void drawHoverGlow(juce::Graphics& g)
        {
            // Solo dibujar glow si hay un bloque siendo hovered
            if (!isMouseOverClickableArea || hoveredBlockName.isEmpty())
                return;
                
            // Buscar el área del bloque hovered
            for (const auto& area : cachedClickableAreas)
            {
                if (area.blockName == hoveredBlockName)
                {
                    // Convertir coordenadas relativas a escaladas usando getScaledBounds
                    auto scaledBounds = owner.getScaledBounds(area.x, area.y, area.w, area.h);
                    juce::Rectangle<float> baseRect = scaledBounds.toFloat();
                    
                    // Efecto glow de dos capas con color dinámico según el bloque
                    juce::Colour blockColor = getBlockColor(hoveredBlockName);
                    
                    // Capa exterior - glow más amplio y sutil
                    juce::Rectangle<float> outerGlow = baseRect.expanded(12.0f);
                    g.setColour(blockColor.withAlpha(0.15f));
                    g.fillRoundedRectangle(outerGlow, 8.0f);
                    
                    // Capa interior - glow más intenso y cercano (color más cálido)
                    juce::Rectangle<float> innerGlow = baseRect.expanded(6.0f);
                    g.setColour(blockColor.brighter(0.2f).withAlpha(0.25f));
                    g.fillRoundedRectangle(innerGlow, 6.0f);
                    
                    // Borde sutil para definir el área
                    g.setColour(blockColor.withAlpha(0.4f));
                    g.drawRoundedRectangle(baseRect.expanded(2.0f), 4.0f, 1.5f);
                    
                    break; // Solo uno a la vez
                }
            }
        }
        
        juce::Rectangle<int> getDiagramBounds() const
        {
            // Usar anchura reducida para dejar visibles los medidores
            auto bounds = getLocalBounds();
            
            // Calcular la franja central del plugin - hacerla más grande
            float totalHeight = bounds.getHeight();
            float yStart = totalHeight * 0.05f;   // Empezar muy arriba (5%)
            float height = totalHeight * 0.9f;    // Usar 90% de la altura total
            
            // Reducir márgenes para hacer el diagrama más grande
            int leftMargin = 30;    // Margen mínimo izquierdo
            int rightMargin = 30;   // Margen mínimo derecho
            int x = leftMargin;
            int width = bounds.getWidth() - leftMargin - rightMargin;
            
            return juce::Rectangle<int>(x, (int)yStart, width, (int)height);
        }
    };
    
    std::unique_ptr<DiagramOverlay> diagramOverlay;
    
    //==========================================================================
    // FUNCIONES UTILITY Y HELPER
    //==========================================================================
    
    // Función auxiliar para límites escalados - convierte coordenadas del sistema de referencia al tamaño actual
    juce::Rectangle<int> getScaledBounds(float x, float y, float w, float h) const noexcept
    {
        return juce::Rectangle<int>(
            getWidth() * x / REFERENCE_WIDTH,    // Escalar X del sistema de referencia
            getHeight() * y / REFERENCE_HEIGHT,   // Escalar Y del sistema de referencia
            getWidth() * w / REFERENCE_WIDTH,     // Escalar ancho del sistema de referencia
            getHeight() * h / REFERENCE_HEIGHT    // Escalar alto del sistema de referencia
        );
    }
    
    //==========================================================================
    // MÉTODOS DE SETUP
    //==========================================================================
    void setupKnobs();
    void setupMeters();
    void setupPresetArea();
    void setupUtilityButtons();
    void setupParameterButtons();
    void setupBackground();
    
    //==========================================================================
    // MÉTODOS DE UPDATE
    //==========================================================================
    void updateButtonStates();
    void updateBasicButtonStates();
    void updateBackgroundState();
    void updateFilterButtonText();
    void updateMeterStates();
    
    //==========================================================================
    // MÉTODOS DE MANEJO DE DELTA MODE
    //==========================================================================
    void applyDeltaModeToAllControls(bool deltaActive);
    void applyDeltaModeToPresetControls(bool deltaActive);
    void applyDeltaModeToAbControls(bool deltaActive);
    void applyDeltaModeToUndoRedoControls(bool deltaActive);
    void applyDeltaModeToUtilityControls(bool deltaActive);
    void applyDeltaModeToMetersAndDisplay(bool deltaActive);
    void updateTransferDisplay();
    void updateMeters();
    void updateSliderValues();
    
    //==========================================================================
    // MÉTODOS HELPER DE UI
    //==========================================================================
    void applyAlphaToMainControls(float alpha);
    
    //==========================================================================
    // MÉTODOS DE GESTIÓN DE PRESETS
    //==========================================================================
    juce::File getPresetsFolder();
    juce::Array<juce::File> populatePresetFolder();
    int getUpdatedNumPresets() { return populatePresetFolder().size(); }
    void savePresetFile();
    void saveAsPresetFile();
    void deletePresetFile();
    void refreshPresetMenu();
    void selectNextPreset();
    void selectPreviousPreset();
    void resetGuiSize();
    void showCustomConfirmDialog(const juce::String& message, const juce::String& subMessage, 
                                  std::function<void(bool)> callback, 
                                  const juce::String& confirmText = "YES", 
                                  const juce::String& cancelText = "NO");
    
    void showCustomAlertDialog(const juce::String& title, const juce::String& message);
    
    //==========================================================================
    // MANEJO DE PARÁMETROS Y EVENTOS
    //==========================================================================
    void handleParameterChange();
    void updateARButtonText();
    void updateRelSliderAlpha();
    
    //==========================================================================
    // GESTIÓN DE TOOLTIPS
    //==========================================================================
    void updateAllTooltips();
    juce::String getTooltipText(const juce::String& key);
    void updateTodoButtonTexts();
    
    //==========================================================================
    // MÉTODOS DE DIAGRAM Y CODE WINDOW
    //==========================================================================
    void showDiagram();
    void hideDiagram();
    void hideCodeWindow();
    juce::String loadCodeFromFile(const juce::String& blockName);
    void initializeCodeContentCache();
    juce::String getBasicBlockDescription(const juce::String& blockName);
    
    //==========================================================================
    // MÉTODOS DE CREDITS
    //==========================================================================
    void showCredits();
    void hideCredits();
    
    //==========================================================================
    // THREAD SAFETY Y AUTOMATIZACIÓN
    //==========================================================================
    void queueParameterUpdate(const juce::String& paramID, float normalizedValue);
    void processPendingParameterUpdates();
    
    //==========================================================================
    // VARIABLES DE STATE
    //==========================================================================
    
    // Instancias de Look and Feel
    CustomSlider::LookAndFeel sliderLAFBig;
    SmallButtonLAF smallButtonLAF;
    
    // Banderas de estado principales
    bool isLoadingPreset = false;
    bool isProcessingQueue = false;  // Flag para prevenir deshacer durante procesamiento de cola
    bool isBypassed = false;
    bool bypassTextVisible = false;
    // Estado de interfaz
    float maxGainReductionFromBuffer = 0.0f;
    int clipResetCounter = 0;
    GuiSizeState currentSizeState = GuiSizeState::Current;
    juce::Point<int> lastCustomSize;
    
    // Sistema de tooltips
    TooltipLanguage currentLanguage = TooltipLanguage::Spanish;
    bool tooltipsEnabled = true;
    
    // Timing y debouncing
    juce::uint32 lastDiagramButtonTime = 0;
    
    // Variables de thread safety
    std::atomic<int> automationUpdateCount{0};
    std::vector<DeferredParameterUpdate> pendingParameterUpdates;
    std::atomic<bool> hasPendingParameterUpdates{false};
    mutable std::mutex parameterUpdateMutex;
    
    // Sistema universal de decay para todos los DAWs
    void applyMeterDecayIfNeeded();
    
    // Cache de contenido de código
    std::unordered_map<juce::String, juce::String> codeContentCache;
    bool codeContentCacheInitialized = false;
    
    // Presets de fábrica
    juce::StringArray factoryPresetNames;
    std::unordered_map<int, juce::String> presetIdToNameMap;
    
    // Operaciones de archivo
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    // Listeners especializados
    std::unique_ptr<TransferFunctionParameterListener> transferFunctionListener;
    std::unique_ptr<AutorelParameterListener> autorelParameterListener;
    std::unique_ptr<DeltaParameterListener> deltaParameterListener;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JCBMaximizerAudioProcessorEditor)
};
