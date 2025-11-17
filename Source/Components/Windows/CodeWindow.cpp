//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBMaximizer and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================

#include "CodeWindow.h"

//==============================================================================
// CONSTRUCTOR Y DESTRUCTOR - CodeWindow
//==============================================================================
CodeWindow::CodeWindow()
{
    setWantsKeyboardFocus(true);
    
    // Configurar el editor de texto
    codeEditor.setMultiLine(true);
    codeEditor.setReadOnly(true);
    codeEditor.setScrollbarsShown(true);
    codeEditor.setCaretVisible(true);  // Permitir ver el cursor para selección
    codeEditor.setPopupMenuEnabled(true);  // Habilitar menú contextual (copy/paste)
    
    // Establecer el LookAndFeel personalizado para scrollbars
    codeEditor.setLookAndFeel(&scrollBarLookAndFeel);
    
    // Configurar colores del editor - fondo negro sólido
    codeEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    codeEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    codeEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    codeEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    
    // NO establecer colores de texto ni scrollbar aquí - esperar a setHaloColour()
    
    // Configurar fuente monoespaciada más pequeña
    juce::Font monoFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
    codeEditor.setFont(monoFont);
    
    addAndMakeVisible(codeEditor);
    
    // Configurar el botón de copiar con mejor visibilidad
    copyButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    copyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    copyButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::grey);
    copyButton.onClick = [this]() { copyToClipboard(); };
    addAndMakeVisible(copyButton);
}

CodeWindow::~CodeWindow()
{
    // Liberar el LookAndFeel antes de destruir
    codeEditor.setLookAndFeel(nullptr);
    
    // Asegurar que el timer se detenga antes de destruir el objeto
    stopTimer();
}

//==============================================================================
// OVERRIDES DE COMPONENT - CodeWindow
//==============================================================================
void CodeWindow::paint(juce::Graphics& g)
{
    // Fondo negro sólido con esquinas redondeadas
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(bounds, 6.0f);
    
    // Borde blanco sólido
    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(bounds.reduced(1.0f), 6.0f, 2.0f);
    
    // Dibujar título en blanco
    juce::Font monoFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain));
    g.setFont(monoFont.withHeight(14.0f).boldened());
    g.setColour(juce::Colours::white);
    g.drawText("gen~ CODEBOX " + windowTitle, 20, 15, getWidth() - 140, 20, 
               juce::Justification::centred, false);
}

void CodeWindow::resized()
{
    // Posicionar el botón de copiar en la esquina superior derecha
    copyButton.setBounds(getWidth() - 100, 10, 80, 25);
    
    // Posicionar el editor de código
    auto codeArea = getLocalBounds().reduced(15).withTrimmedTop(45);
    codeEditor.setBounds(codeArea);
}

bool CodeWindow::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        if (onClose)
            onClose();
        return true;
    }
    
    // Permitir copiar con Ctrl+C / Cmd+C sin usar el botón
    if (key == juce::KeyPress('c', juce::ModifierKeys::commandModifier, 0))
    {
        // Si hay texto seleccionado en el editor, dejar que maneje el comando
        if (codeEditor.getHighlightedText().isNotEmpty())
        {
            return false;  // Dejar que el TextEditor maneje la copia
        }
        else
        {
            // Si no hay selección, copiar todo el código
            copyToClipboard();
            return true;
        }
    }
    
    return false;
}

void CodeWindow::mouseDown(const juce::MouseEvent& event)
{
    // No cerrar con click - solo con ESC o el botón de copiar
    // Esto permite interactuar con el texto sin cerrar accidentalmente
}

//==============================================================================
// MÉTODOS PÚBLICOS DE CONFIGURACIÓN - CodeWindow
//==============================================================================
void CodeWindow::setCode(const juce::String& code, const juce::String& title)
{
    codeText = code;
    windowTitle = title;
    codeEditor.setText(code);
    repaint();
}

void CodeWindow::setHaloColour(const juce::Colour& colour)
{
    // Forzar colores negro y blanco independientemente del color de entrada
    haloColour = juce::Colours::white;
    
    // Botón con contraste máximo - gris oscuro sobre negro
    copyButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    copyButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::grey);
    copyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    copyButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    
    // Forzar texto blanco y fondo negro sólido
    codeEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    codeEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    codeEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::white);
    codeEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::white);
    
    // Scrollbar gris sobre negro
    scrollBarLookAndFeel.setScrollBarColour(juce::Colours::grey);
    
    // Forzar actualización del editor de texto y sus scrollbars
    codeEditor.applyFontToAllText(codeEditor.getFont());
    codeEditor.repaint();
    
    // Repintar todos los componentes hijos (incluyendo scrollbars)
    for (auto* child : codeEditor.getChildren())
    {
        child->repaint();
    }
    
    repaint();
}

//==============================================================================
// MÉTODOS PRIVADOS DE UTILIDAD - CodeWindow
//==============================================================================
juce::Colour CodeWindow::getCodeColour() const
{
    // Retornar el color del halo aclarado para el texto
    return haloColour.brighter(0.3f);
}

void CodeWindow::copyToClipboard()
{
    // Deshabilitar el botón para evitar múltiples clicks
    copyButton.setEnabled(false);
    
    try {
        // Copiar al portapapeles
        juce::SystemClipboard::copyTextToClipboard(codeText);
        
        copyButton.setButtonText("Copied!");
        
        // Iniciar timer para restaurar el texto después de 1 segundo
        startTimer(1000);
    }
    catch (...) {
        // Si falla, mostrar error
        copyButton.setButtonText("Error!");
        
        // Iniciar timer para restaurar el texto
        startTimer(1000);
    }
}

void CodeWindow::timerCallback()
{
    // Restaurar el texto original del botón
    copyButton.setButtonText("Copy Code");
    copyButton.setEnabled(true);
    
    // Detener el timer
    stopTimer();
}

//==============================================================================
// CONSTRUCTOR Y DESTRUCTOR - CodeOverlay
//==============================================================================
CodeOverlay::CodeOverlay()
{
    addAndMakeVisible(codeWindow);
    
    // Conectar callbacks
    codeWindow.onClose = [this]() {
        if (onClose)
            onClose();
    };
    
    setInterceptsMouseClicks(true, true);
}

//==============================================================================
// OVERRIDES DE COMPONENT - CodeOverlay
//==============================================================================
void CodeOverlay::paint(juce::Graphics& g)
{
    // Fondo semi-transparente
    g.fillAll(juce::Colours::black.withAlpha(0.5f));
    
    // Obtener los límites de la ventana de código
    auto codeBounds = codeWindow.getBounds().toFloat();
    
    // Dibujar aureola/halo alrededor del diálogo
    // Capa más externa
    g.setColour(codeWindow.getHaloColour().withAlpha(0.1f));
    g.fillRoundedRectangle(codeBounds.expanded(12), 8.0f);
    
    // Capa intermedia
    g.setColour(codeWindow.getHaloColour().withAlpha(0.15f));
    g.fillRoundedRectangle(codeBounds.expanded(8), 6.0f);
    
    // Capa más cercana
    g.setColour(codeWindow.getHaloColour().withAlpha(0.2f));
    g.fillRoundedRectangle(codeBounds.expanded(4), 4.0f);
}

void CodeOverlay::resized()
{
    auto bounds = getLocalBounds();
    
    // Ventana más estrecha y alta
    int dialogWidth = static_cast<int>(bounds.getWidth() * 0.35f);  // 35% del ancho
    int dialogHeight = static_cast<int>(bounds.getHeight() * 0.7f); // 70% de la altura
    
    // Limitar tamaños
    dialogWidth = juce::jmin(dialogWidth, 450);  // Máximo 450px
    dialogWidth = juce::jmax(dialogWidth, 350);  // Mínimo 350px
    dialogHeight = juce::jmin(dialogHeight, 500); // Máximo 500px
    dialogHeight = juce::jmax(dialogHeight, 300); // Mínimo 300px
    
    // Centrar la ventana
    codeWindow.setBounds(bounds.getCentreX() - dialogWidth/2,
                         bounds.getCentreY() - dialogHeight/2,
                         dialogWidth, dialogHeight);
}

void CodeOverlay::mouseDown(const juce::MouseEvent& event)
{
    // Solo cerrar si se hace click fuera de la ventana de código
    if (!codeWindow.getBounds().contains(event.position.toInt()))
    {
        if (onClose)
            onClose();
    }
}

//==============================================================================
// MÉTODOS PÚBLICOS DE CONFIGURACIÓN - CodeOverlay
//==============================================================================
void CodeOverlay::setCode(const juce::String& code, const juce::String& title)
{
    codeWindow.setCode(code, title);
}

void CodeOverlay::setHaloColour(const juce::Colour& colour)
{
    codeWindow.setHaloColour(colour);
    repaint();
}