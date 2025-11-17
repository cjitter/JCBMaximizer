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
#include "DarkThemeColors.h"

//==============================================================================
// CLASE BASE PARA CUSTOM DIALOGS
//==============================================================================

/**
 * CustomDialog - Clase base para diálogos modales con tema oscuro moderno
 * 
 * Sistema base de diálogos que provee funcionalidad común para todos los
 * diálogos modales del plugin. Características principales:
 * - Fondo translúcido con efectos de glow multicapa
 * - Manejo automático de teclado (ESC para cerrar)
 * - Sistema de callbacks para respuesta de usuario
 * - Posicionamiento automático centrado
 * - Integración completa con DarkTheme del plugin
 * - Soporte para títulos con gradientes y efectos visuales
 */
class CustomDialog : public juce::Component
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CustomDialog(const juce::String& title, int width = 300, int height = 200)
        : dialogTitle(title), dialogWidth(width), dialogHeight(height)
    {
        setInterceptsMouseClicks(true, true);
        setAlwaysOnTop(true);
        setOpaque(false);
        setWantsKeyboardFocus(true);
    }
    
    ~CustomDialog() override = default;
    
    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        // Fondo más translúcido
        g.fillAll(juce::Colours::black.withAlpha(0.3f));
        
        // Obtener el área del diálogo
        auto dialogBounds = getDialogBounds();
        
        // Enhanced multi-layer glow effect
        float scaleFactor = dialogBounds.getWidth() / 300.0f;
        
        // Outer glow layer - subtle blue
        g.setColour(DarkTheme::accent.withAlpha(0.06f));
        g.fillRoundedRectangle(dialogBounds.expanded(18 * scaleFactor).toFloat(), 14.0f * scaleFactor);
        
        // Mid glow layer - purple accent
        g.setColour(DarkTheme::accentSecondary.withAlpha(0.10f));
        g.fillRoundedRectangle(dialogBounds.expanded(12 * scaleFactor).toFloat(), 10.0f * scaleFactor);
        
        // Inner glow layer - stronger accent
        g.setColour(DarkTheme::accent.withAlpha(0.18f));
        g.fillRoundedRectangle(dialogBounds.expanded(6 * scaleFactor).toFloat(), 8.0f * scaleFactor);
        
        // Immediate border glow
        g.setColour(DarkTheme::accentSecondary.withAlpha(0.25f));
        g.fillRoundedRectangle(dialogBounds.expanded(2 * scaleFactor).toFloat(), 6.0f * scaleFactor);
        
        // Main panel with subtle gradient
        auto gradient = juce::ColourGradient(DarkTheme::backgroundDark.brighter(0.05f), 
                                           dialogBounds.getTopLeft().toFloat(),
                                           DarkTheme::backgroundDark.darker(0.03f), 
                                           dialogBounds.getBottomLeft().toFloat(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(dialogBounds.toFloat(), 6.0f * scaleFactor);
        
        // Enhanced border with gradient
        g.setColour(DarkTheme::accent.withAlpha(0.7f));
        g.drawRoundedRectangle(dialogBounds.toFloat().reduced(0.5f), 6.0f * scaleFactor, 1.8f * scaleFactor);
        
        // Inner highlight border
        g.setColour(DarkTheme::accentSecondary.withAlpha(0.3f));
        g.drawRoundedRectangle(dialogBounds.toFloat().reduced(1.5f), 5.0f * scaleFactor, 0.8f * scaleFactor);
        
        // Enhanced title with subtle glow
        g.setColour(DarkTheme::textPrimary);
        g.setFont(juce::Font(juce::FontOptions(18.0f * scaleFactor).withStyle("Bold")));
        auto titleBounds = dialogBounds.removeFromTop(40 * scaleFactor);
        
        // Title glow effect
        g.setColour(DarkTheme::accent.withAlpha(0.15f));
        g.drawText(dialogTitle, titleBounds.translated(0, 1), juce::Justification::centred);
        
        // Main title text
        g.setColour(DarkTheme::textPrimary);
        g.drawText(dialogTitle, titleBounds, juce::Justification::centred);
        
        // Enhanced separator line with gradient
        auto separatorY = titleBounds.getBottom();
        auto separatorStart = dialogBounds.getX() + 20;
        auto separatorEnd = dialogBounds.getRight() - 20;
        
        juce::ColourGradient separatorGradient(
            juce::Colours::transparentBlack, separatorStart, separatorY,
            DarkTheme::accent.withAlpha(0.4f), (separatorStart + separatorEnd) / 2, separatorY, false);
        separatorGradient.addColour(1.0, juce::Colours::transparentBlack);
        g.setGradientFill(separatorGradient);
        g.drawLine(separatorStart, separatorY, separatorEnd, separatorY, 1.5f);
    }
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        // Si hace clic fuera del diálogo, cerrarlo
        if (!getDialogBounds().contains(event.getPosition()))
        {
            closeDialog(false);
        }
    }
    
    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::escapeKey)
        {
            closeDialog(false);
            return true;
        }
        return false;
    }
    
    //==========================================================================
    // MÉTODOS PÚBLICOS DE CONFIGURACIÓN
    //==========================================================================
    
    virtual void showDialog()
    {
        if (auto* parent = getParentComponent())
        {
            setBounds(parent->getLocalBounds());
            parent->addAndMakeVisible(this);
            setVisible(true);
            toFront(true);
            grabKeyboardFocus();
            
            // Informar al editor del plugin que queremos el foco del teclado
            if (auto* editor = dynamic_cast<juce::AudioProcessorEditor*>(parent))
            {
                editor->setWantsKeyboardFocus(true);
                editor->grabKeyboardFocus();
            }
        }
    }
    
    virtual void closeDialog(bool result)
    {
        setVisible(false);
        if (onDialogClosed)
            onDialogClosed(result);
        if (auto* parent = getParentComponent())
            parent->removeChildComponent(this);
    }
    
    //==========================================================================
    // CALLBACKS
    //==========================================================================
    
    std::function<void(bool)> onDialogClosed;  // Callback cuando el diálogo se cierra
    
protected:
    //==========================================================================
    // MÉTODOS PROTEGIDOS
    //==========================================================================
    
    virtual juce::Rectangle<int> getDialogBounds() const
    {
        auto bounds = getLocalBounds();
        int x = (bounds.getWidth() - dialogWidth) / 2;
        int y = (bounds.getHeight() - dialogHeight) / 2;
        return juce::Rectangle<int>(x, y, dialogWidth, dialogHeight);
    }
    
    //==========================================================================
    // VARIABLES PROTEGIDAS
    //==========================================================================
    
    juce::String dialogTitle;  // Título del diálogo
    int dialogWidth;           // Ancho del diálogo
    int dialogHeight;          // Altura del diálogo
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomDialog)
};

//==============================================================================
// DIÁLOGO DE CONFIRMACIÓN CON DOS BOTONES
//==============================================================================

/**
 * CustomConfirmDialog - Diálogo de confirmación YES/NO con estilo moderno
 * 
 * Diálogo especializado para confirmaciones binarias. Características principales:
 * - Dos botones con jerarquía visual clara (YES principal, NO secundario)
 * - Mensaje configurable con text wrapping automático
 * - Styling moderno con gradientes y efectos de glow
 * - Callback con resultado booleano según botón presionado
 * - Integración perfecta con el sistema de temas del plugin
 */
class CustomConfirmDialog : public CustomDialog
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CustomConfirmDialog(const juce::String& title, 
                       const juce::String& message,
                       const juce::String& confirmText = "YES",
                       const juce::String& cancelText = "NO")
        : CustomDialog(title, 320, 150),  // Tamaño estandarizado
          messageText(message),
          confirmButtonText(confirmText),
          cancelButtonText(cancelText)
    {
        // Crear botones
        confirmButton = std::make_unique<juce::TextButton>(confirmButtonText);
        cancelButton = std::make_unique<juce::TextButton>(cancelButtonText);
        
        // Enhanced modern button styling
        auto setupButton = [](juce::TextButton* btn, bool isPrimary) {
            if (isPrimary) {
                // Primary button with gradient and modern styling
                btn->setColour(juce::TextButton::buttonColourId, DarkTheme::accentSecondary);
                btn->setColour(juce::TextButton::buttonOnColourId, DarkTheme::accentSecondary.brighter(0.25f));
                btn->setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
                btn->setColour(juce::ComboBox::outlineColourId, DarkTheme::accent.withAlpha(0.5f));
            } else {
                // Secondary button with subtle styling
                btn->setColour(juce::TextButton::buttonColourId, DarkTheme::backgroundLight.brighter(0.05f));
                btn->setColour(juce::TextButton::buttonOnColourId, DarkTheme::backgroundLight.brighter(0.15f));
                btn->setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.brighter(0.2f));
                btn->setColour(juce::ComboBox::outlineColourId, DarkTheme::border.withAlpha(0.6f));
            }
            btn->setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
        };
        
        setupButton(confirmButton.get(), true);
        setupButton(cancelButton.get(), false);
        
        confirmButton->onClick = [this]() { closeDialog(true); };
        cancelButton->onClick = [this]() { closeDialog(false); };
        
        addAndMakeVisible(confirmButton.get());
        addAndMakeVisible(cancelButton.get());
    }
    
    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        CustomDialog::paint(g);
        
        auto dialogBounds = getDialogBounds();
        dialogBounds.removeFromTop(40); // Espacio del título
        
        // Dibujar mensaje con tamaño consistente
        g.setColour(DarkTheme::textSecondary);
        g.setFont(juce::Font(juce::FontOptions(15.0f)));
        auto messageBounds = dialogBounds.removeFromTop(50); // Ajustado para coincidir con resized
        messageBounds = messageBounds.reduced(20, 10);
        g.drawFittedText(messageText, messageBounds, juce::Justification::centred, 2);
    }
    
    void resized() override
    {
        auto dialogBounds = getDialogBounds();
        dialogBounds.removeFromTop(40); // Título
        dialogBounds.removeFromTop(50); // Mensaje (reducido)
        
        // Posicionar botones desde abajo con más margen
        auto buttonArea = dialogBounds.removeFromBottom(45);
        buttonArea = buttonArea.reduced(20, 8);
        
        const int buttonWidth = 90;
        const int spacing = 16;
        int totalWidth = buttonWidth * 2 + spacing;
        int startX = (buttonArea.getWidth() - totalWidth) / 2;
        
        confirmButton->setBounds(buttonArea.getX() + startX, buttonArea.getY(), 
                                buttonWidth, buttonArea.getHeight());
        cancelButton->setBounds(buttonArea.getX() + startX + buttonWidth + spacing, 
                               buttonArea.getY(), buttonWidth, buttonArea.getHeight());
    }
    
private:
    //==========================================================================
    // VARIABLES DE CONTENIDO
    //==========================================================================
    
    juce::String messageText;          // Texto del mensaje a mostrar
    juce::String confirmButtonText;    // Texto del botón de confirmación
    juce::String cancelButtonText;     // Texto del botón de cancelación
    
    //==========================================================================
    // COMPONENTES DE UI
    //==========================================================================
    
    std::unique_ptr<juce::TextButton> confirmButton;  // Botón de confirmación
    std::unique_ptr<juce::TextButton> cancelButton;   // Botón de cancelación
};

//==============================================================================
// COMPONENTE AUXILIAR PARA PRESET NAME EDITOR
//==============================================================================

/**
 * PresetNameEditor - TextEditor mejorado para nombres de presets
 * 
 * TextEditor especializado que maneja correctamente todas las teclas incluída
 * la barra espaciadora. Soluciona problemas de captura de teclado en diálogos
 * modales asegurando que todas las teclas de escritura funcionen correctamente.
 */
class PresetNameEditor : public juce::TextEditor
{
public:
    bool keyPressed(const juce::KeyPress& key) override
    {
        // Permitir específicamente la barra espaciadora
        if (key == juce::KeyPress::spaceKey)
        {
            insertTextAtCaret(" ");
            return true;
        }
        
        // Dejar que TextEditor maneje el resto de las teclas normalmente
        return juce::TextEditor::keyPressed(key);
    }
};

//==============================================================================
// DIÁLOGO PARA GUARDAR PRESETS
//==============================================================================

/**
 * SavePresetDialog - Diálogo para introducir nombre de preset
 * 
 * Diálogo especializado para capturar nombres de presets del usuario.
 * Características principales:
 * - TextEditor personalizado con manejo mejorado de teclado
 * - Botones SAVE/CANCEL con styling moderno
 * - Validation automática de texto no vacío
 * - Support para Enter (confirmar) y Escape (cancelar)
 * - Focus automático en el campo de texto al abrir
 */
class SavePresetDialog : public CustomDialog
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    SavePresetDialog(const juce::String& initialName = "")
        : CustomDialog("Save Preset", 320, 150)  // Standardized size
    {
        // Crear campo de texto personalizado con manejo especial de teclas
        nameEditor = std::make_unique<PresetNameEditor>();
        nameEditor->setText(initialName);
        nameEditor->setColour(juce::TextEditor::backgroundColourId, DarkTheme::backgroundMedium.brighter(0.1f));
        nameEditor->setColour(juce::TextEditor::textColourId, DarkTheme::textPrimary);
        nameEditor->setColour(juce::TextEditor::outlineColourId, DarkTheme::border.withAlpha(0.6f));
        nameEditor->setColour(juce::TextEditor::focusedOutlineColourId, DarkTheme::accent.brighter(0.1f));
        nameEditor->setColour(juce::TextEditor::highlightColourId, DarkTheme::accent.withAlpha(0.3f));
        nameEditor->setColour(juce::TextEditor::highlightedTextColourId, DarkTheme::textPrimary);
        nameEditor->setJustification(juce::Justification::centred);
        nameEditor->setFont(juce::Font(juce::FontOptions(16.0f)));
        nameEditor->selectAll();
        
        // Configurar para consumir todas las teclas
        nameEditor->setWantsKeyboardFocus(true);
        
        // Enter key para confirmar
        nameEditor->onReturnKey = [this]() {
            if (nameEditor->getText().isNotEmpty())
                closeDialog(true);
        };
        
        // Escape key para cancelar
        nameEditor->onEscapeKey = [this]() {
            closeDialog(false);
        };
        
        // Crear botones
        saveButton = std::make_unique<juce::TextButton>("SAVE");
        cancelButton = std::make_unique<juce::TextButton>("CANCEL");
        
        // Enhanced modern button styling consistent with other dialogs
        saveButton->setColour(juce::TextButton::buttonColourId, DarkTheme::accentSecondary);
        saveButton->setColour(juce::TextButton::buttonOnColourId, DarkTheme::accentSecondary.brighter(0.25f));
        saveButton->setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
        saveButton->setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
        saveButton->setColour(juce::ComboBox::outlineColourId, DarkTheme::accent.withAlpha(0.5f));
        
        cancelButton->setColour(juce::TextButton::buttonColourId, DarkTheme::backgroundLight.brighter(0.05f));
        cancelButton->setColour(juce::TextButton::buttonOnColourId, DarkTheme::backgroundLight.brighter(0.15f));
        cancelButton->setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.brighter(0.2f));
        cancelButton->setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
        cancelButton->setColour(juce::ComboBox::outlineColourId, DarkTheme::border.withAlpha(0.6f));
        
        saveButton->onClick = [this]() {
            if (nameEditor->getText().isNotEmpty())
                closeDialog(true);
        };
        cancelButton->onClick = [this]() { closeDialog(false); };
        
        addAndMakeVisible(nameEditor.get());
        addAndMakeVisible(saveButton.get());
        addAndMakeVisible(cancelButton.get());
    }
    
    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        CustomDialog::paint(g);
        // No need to draw anything extra - just the base dialog
    }
    
    void resized() override
    {
        auto dialogBounds = getDialogBounds();
        dialogBounds.removeFromTop(45); // Título
        
        // Campo de texto - más prominente sin etiqueta
        auto editorBounds = dialogBounds.removeFromTop(35);
        editorBounds = editorBounds.reduced(20, 0);
        nameEditor->setBounds(editorBounds);
        
        // Botones subidos más arriba dentro de la ventana
        auto buttonArea = dialogBounds.removeFromBottom(60); // Más espacio para subirlos
        buttonArea = buttonArea.reduced(20, 15); // Más margen vertical
        
        const int buttonWidth = 90;
        const int spacing = 16;
        int totalWidth = buttonWidth * 2 + spacing;
        int startX = (buttonArea.getWidth() - totalWidth) / 2;
        
        saveButton->setBounds(buttonArea.getX() + startX, buttonArea.getY(), 
                             buttonWidth, buttonArea.getHeight());
        cancelButton->setBounds(buttonArea.getX() + startX + buttonWidth + spacing, 
                               buttonArea.getY(), buttonWidth, buttonArea.getHeight());
    }
    
    void showDialog() override
    {
        CustomDialog::showDialog();
        
        // Asegurar que el editor de texto tenga el foco del teclado
        if (nameEditor != nullptr)
        {
            nameEditor->setWantsKeyboardFocus(true);
            nameEditor->grabKeyboardFocus();
            nameEditor->selectAll();
            
            // Forzar que el componente raíz tenga el foco
            if (auto* topLevel = getTopLevelComponent())
            {
                if (auto* editor = dynamic_cast<juce::AudioProcessorEditor*>(topLevel))
                {
                    editor->setWantsKeyboardFocus(true);
                }
            }
        }
    }
    
    //==========================================================================
    // MÉTODOS PÚBLICOS DE ACCESO
    //==========================================================================
    
    juce::String getPresetName() const
    {
        return nameEditor->getText();
    }
    
private:
    //==========================================================================
    // COMPONENTES DE UI
    //==========================================================================
    
    std::unique_ptr<PresetNameEditor> nameEditor;     // Editor de texto para el nombre
    std::unique_ptr<juce::TextButton> saveButton;     // Botón de guardar
    std::unique_ptr<juce::TextButton> cancelButton;   // Botón de cancelar
};

//==============================================================================
// DIÁLOGO CON TRES OPCIONES (OVERWRITE/SAVE AS/CANCEL)
//==============================================================================

/**
 * CustomThreeButtonDialog - Diálogo especializado con tres botones
 * 
 * Diálogo usado principalmente para el workflow de presets cuando ya
 * existe un preset con el mismo nombre. Características principales:
 * - Tres botones con jerarquía visual clara (Overwrite/Save As/Cancel)
 * - Styling diferenciado por importancia (primario/secundario/terciario)
 * - Return valor numérico indicando qué botón fue presionado
 * - Layout optimizado para acomodar tres botones sin crowding
 * - Integración perfecta con el sistema de gestión de presets
 */
class CustomThreeButtonDialog : public CustomDialog
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CustomThreeButtonDialog(const juce::String& title, 
                           const juce::String& message,
                           const juce::String& button1Text = "Opcion 1",
                           const juce::String& button2Text = "Opcion 2",
                           const juce::String& button3Text = "Cancel")
        : CustomDialog(title, 320, 150),  // Aumentado de 300 a 320 para acomodar 3 botones
          messageText(message)
    {
        // Crear los 3 botones
        button1 = std::make_unique<juce::TextButton>(button1Text);
        button2 = std::make_unique<juce::TextButton>(button2Text);
        button3 = std::make_unique<juce::TextButton>(button3Text);
        
        // Enhanced button hierarchy with modern visual styling
        // Button 1 (Overwrite) - Primary with purple accent
        button1->setColour(juce::TextButton::buttonColourId, DarkTheme::accentSecondary);
        button1->setColour(juce::TextButton::buttonOnColourId, DarkTheme::accentSecondary.brighter(0.25f));
        button1->setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
        button1->setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
        button1->setColour(juce::ComboBox::outlineColourId, DarkTheme::accentSecondary.withAlpha(0.6f));
        
        // Button 2 (Save As) - Secondary with blue accent
        button2->setColour(juce::TextButton::buttonColourId, DarkTheme::accent.withAlpha(0.9f));
        button2->setColour(juce::TextButton::buttonOnColourId, DarkTheme::accent.brighter(0.2f));
        button2->setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
        button2->setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
        button2->setColour(juce::ComboBox::outlineColourId, DarkTheme::accent.withAlpha(0.5f));
        
        // Button 3 (Cancel) - Tertiary with subtle styling
        button3->setColour(juce::TextButton::buttonColourId, DarkTheme::backgroundLight.brighter(0.05f));
        button3->setColour(juce::TextButton::buttonOnColourId, DarkTheme::backgroundLight.brighter(0.15f));
        button3->setColour(juce::TextButton::textColourOffId, DarkTheme::textSecondary.brighter(0.2f));
        button3->setColour(juce::TextButton::textColourOnId, DarkTheme::textPrimary);
        button3->setColour(juce::ComboBox::outlineColourId, DarkTheme::border.withAlpha(0.6f));
        
        button1->onClick = [this]() { 
            selectedButton = 1;
            closeDialog(true); 
        };
        button2->onClick = [this]() { 
            selectedButton = 2;
            closeDialog(true); 
        };
        button3->onClick = [this]() { 
            selectedButton = 3;
            closeDialog(false); 
        };
        
        addAndMakeVisible(button1.get());
        addAndMakeVisible(button2.get());
        addAndMakeVisible(button3.get());
    }
    
    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        CustomDialog::paint(g);
        
        if (!messageText.isEmpty())
        {
            auto dialogBounds = getDialogBounds();
            dialogBounds.removeFromTop(40); // Título
            
            // Dibujar mensaje con tamaño consistente
            g.setColour(DarkTheme::textSecondary);
            g.setFont(juce::Font(juce::FontOptions(15.0f)));
            auto messageBounds = dialogBounds.removeFromTop(60); // Vuelto al tamaño anterior
            messageBounds = messageBounds.reduced(20, 10);
            g.drawFittedText(messageText, messageBounds, juce::Justification::centred, 3);
        }
    }
    
    void resized() override
    {
        auto dialogBounds = getDialogBounds();
        dialogBounds.removeFromTop(40); // Título
        
        if (!messageText.isEmpty())
            dialogBounds.removeFromTop(60); // Mensaje
        
        // Calcular posición para centrar botones en la mitad de la altura restante
        const int buttonHeight = 35;
        int remainingHeight = dialogBounds.getHeight();
        int buttonY = dialogBounds.getY() + (remainingHeight - buttonHeight) / 2;
        
        auto buttonArea = juce::Rectangle<int>(dialogBounds.getX(), buttonY, 
                                              dialogBounds.getWidth(), buttonHeight);
        buttonArea = buttonArea.reduced(20, 0);
        
        const int buttonWidth = 75;  // Optimized for three buttons
        const int spacing = 12;
        int totalWidth = buttonWidth * 3 + spacing * 2;
        int startX = (buttonArea.getWidth() - totalWidth) / 2;
        
        button1->setBounds(buttonArea.getX() + startX, buttonArea.getY(), 
                          buttonWidth, buttonArea.getHeight());
        button2->setBounds(buttonArea.getX() + startX + buttonWidth + spacing, 
                          buttonArea.getY(), buttonWidth, buttonArea.getHeight());
        button3->setBounds(buttonArea.getX() + startX + (buttonWidth + spacing) * 2, 
                          buttonArea.getY(), buttonWidth, buttonArea.getHeight());
    }
    
    //==========================================================================
    // MÉTODOS PÚBLICOS DE ACCESO
    //==========================================================================
    
    int getSelectedButton() const { return selectedButton; }  // Obtener índice del botón seleccionado
    
private:
    //==========================================================================
    // VARIABLES DE CONTENIDO
    //==========================================================================
    
    juce::String messageText;    // Texto del mensaje a mostrar
    int selectedButton = 0;      // Índice del botón seleccionado (1, 2, o 3)
    
    //==========================================================================
    // COMPONENTES DE UI
    //==========================================================================
    
    std::unique_ptr<juce::TextButton> button1;  // Botón 1 (Overwrite - primario)
    std::unique_ptr<juce::TextButton> button2;  // Botón 2 (Save As - secundario)
    std::unique_ptr<juce::TextButton> button3;  // Botón 3 (Cancel - terciario)
};

//==============================================================================
// DIÁLOGO DE ALERTA CON BOTÓN OK
//==============================================================================

/**
 * CustomAlertDialog - Diálogo de alerta simple con un solo botón
 * 
 * Diálogo informativo para mostrar mensajes al usuario que requieren
 * solo confirmación de lectura. Características principales:
 * - Un solo botón OK con styling prominente
 * - Mensaje con text wrapping automático
 * - Usado para errores, avisos y confirmaciones informativas
 * - Layout optimizado para legibilidad del mensaje
 * - Integración perfecta con el sistema de diálogos del plugin
 */
class CustomAlertDialog : public CustomDialog
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CustomAlertDialog(const juce::String& title, const juce::String& message)
        : CustomDialog(title, 300, 150),  // Same size as Save As
          messageText(message)
    {
        // Crear botón OK
        okButton = std::make_unique<juce::TextButton>("OK");
        okButton->setColour(juce::TextButton::buttonColourId, DarkTheme::accent);
        okButton->setColour(juce::TextButton::textColourOffId, DarkTheme::textPrimary);
        okButton->onClick = [this]() { closeDialog(true); };
        
        addAndMakeVisible(okButton.get());
    }
    
    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override
    {
        CustomDialog::paint(g);
        
        auto dialogBounds = getDialogBounds();
        dialogBounds.removeFromTop(50); // Título
        
        // Mensaje (ajustado para coincidir con resized)
        auto messageBounds = dialogBounds.removeFromTop(35);
        messageBounds = messageBounds.reduced(20, 0);
        
        g.setColour(DarkTheme::textSecondary);
        g.setFont(juce::Font(juce::FontOptions(15.0f)));
        g.drawFittedText(messageText, messageBounds, juce::Justification::centred, 3);
    }
    
    //==========================================================================
    // MÉTODOS PROTEGIDOS OVERRIDE
    //==========================================================================
    
    juce::Rectangle<int> getDialogBounds() const override
    {
        auto bounds = getLocalBounds();
        int x = (bounds.getWidth() - dialogWidth) / 2;
        int y = (bounds.getHeight() - dialogHeight) / 2; // Centrado como las demás ventanas
        return juce::Rectangle<int>(x, y, dialogWidth, dialogHeight);
    }
    
    void resized() override
    {
        auto dialogBounds = getDialogBounds();
        dialogBounds.removeFromTop(50); // Title
        dialogBounds.removeFromTop(35); // Message (reducido para dar espacio al botón)
        
        // Botón OK subido dentro de la ventana
        auto buttonArea = dialogBounds.removeFromBottom(50);
        buttonArea = buttonArea.reduced(20, 12);
        
        const int buttonWidth = 80;
        int centerX = (buttonArea.getWidth() - buttonWidth) / 2;
        
        okButton->setBounds(buttonArea.getX() + centerX, buttonArea.getY(), 
                           buttonWidth, buttonArea.getHeight());
    }
    
private:
    //==========================================================================
    // VARIABLES DE CONTENIDO
    //==========================================================================
    
    juce::String messageText;  // Texto del mensaje a mostrar
    
    //==========================================================================
    // COMPONENTES DE UI
    //==========================================================================
    
    std::unique_ptr<juce::TextButton> okButton;  // Botón OK
};