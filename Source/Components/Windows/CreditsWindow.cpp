//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBMaximizer and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "CreditsWindow.h"
#include "../../Helpers/UTF8Helper.h"
#include "BinaryData.h"

//==============================================================================
// CONSTRUCTOR Y DESTRUCTOR - CreditsWindow
//==============================================================================
CreditsWindow::CreditsWindow(const juce::String& format)
    : pluginFormat(format)
{
    setWantsKeyboardFocus(true);
    
    // Cargar la imagen de fondo del diagrama
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::diagramaFondo_png, 
                                                     BinaryData::diagramaFondo_pngSize);
    
    // Configurar botón de idioma
    langButton.setButtonText("ENG");
    langButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    langButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    langButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.8f));
    langButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    langButton.onClick = [this]() { toggleLanguage(); };
    addAndMakeVisible(langButton);
    
    buildFullCreditsText();
    
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

CreditsWindow::~CreditsWindow()
{
}

//==============================================================================
// COMPONENT OVERRIDES - CreditsWindow
//==============================================================================
void CreditsWindow::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Dibujar la imagen de fondo con esquinas redondeadas
    if (backgroundImage.isValid())
    {
        // Crear un path con esquinas redondeadas
        juce::Path roundedRect;
        roundedRect.addRoundedRectangle(bounds, 6.0f);
        
        // Aplicar clipping para las esquinas redondeadas
        g.saveState();
        g.reduceClipRegion(roundedRect);
        


        // Dibujar la imagen escalada para llenar el área con transparencia
        g.setOpacity(0.6f);  // Hacer la imagen más translúcida
        g.drawImage(backgroundImage, bounds, 
                    juce::RectanglePlacement::stretchToFit);
        g.setOpacity(1.0f);  // Restaurar opacidad completa
        
        g.restoreState();
    }
    else
    {
        // Fallback: fondo negro si no se carga la imagen
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds, 6.0f);
    }
    
    // Sin borde - comentado para quitar el contorno blanco
    // g.setColour(juce::Colours::white.withAlpha(0.8f));
    // g.drawRoundedRectangle(bounds.reduced(1.0f), 6.0f, 2.0f);
    
    // Color del texto
    auto textColour = getTerminalColour();
    
    // Escalar el tamaño de fuente con la ventana para mejor legibilidad
    float scaleFactor = juce::jmin(getWidth() / 700.0f, getHeight() / 200.0f);
    float fontSize = 8.0f * scaleFactor;  // Base de 8pt escalada
    // Sin límites para permitir que el texto crezca con la ventana
    fontSize = juce::jmax(6.0f, fontSize); // Solo límite inferior de 6pt
    
    // Configurar fuente escalada
    juce::Font monoFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), fontSize, juce::Font::plain));
    g.setFont(monoFont);
    g.setColour(textColour);
    
    // Padding escalado con la ventana
    int padding = static_cast<int>(10 * scaleFactor);
    padding = juce::jmax(5, juce::jmin(padding, 20));
    
    // Calcular el área de texto
    auto textArea = getLocalBounds().reduced(padding);
    
    // Usar drawMultiLineText para mejor calidad
    // Dividir el texto en líneas
    auto lines = juce::StringArray::fromLines(fullText);
    
    // Calcular altura de línea
    int lineHeight = static_cast<int>(fontSize * 1.15f);  // Reducir espaciado entre líneas
    
    // Posicionar el texto más arriba, con margen mínimo
    int yPos = textArea.getY() - lineHeight;  // Subir un renglón completo
    
    // Dibujar cada línea centrada
    for (int lineIndex = 0; lineIndex < static_cast<int>(lines.size()); ++lineIndex)
    {
        const auto& line = lines[lineIndex];
        
        // Configurar fuente más grande para el título (segunda línea después de la línea vacía)
        bool isTitle = (lineIndex == 1);  // "JCBMaximizer v1.0.0" es la segunda línea
        if (isTitle)
        {
            float titleSize = fontSize * 1.5f;  // 50% más grande
            // Sin límite superior para permitir que crezca con la ventana
            juce::Font titleFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), titleSize, juce::Font::bold));
            g.setFont(titleFont);
        }
        else
        {
            g.setFont(monoFont);
        }
        
        // Verificar que hay espacio para esta línea
        if (yPos < textArea.getY())
        {
            yPos += lineHeight;
            continue; // Saltar líneas que están arriba del área visible
        }
        
        if (yPos + lineHeight > textArea.getBottom())
            break; // Detener si no hay espacio para más líneas
        
        // Buscar y actualizar bounds de links en esta línea
        for (auto& link : links)
        {
            if (line.contains(link.text))
            {
                // Calcular bounds centrados
                int textWidth = static_cast<int>(juce::GlyphArrangement::getStringWidth(monoFont, link.text));
                int lineWidth = static_cast<int>(juce::GlyphArrangement::getStringWidth(monoFont, line));
                int lineX = textArea.getCentreX() - lineWidth / 2;
                
                int linkStart = line.indexOf(link.text);
                juce::String beforeLink = line.substring(0, linkStart);
                int beforeWidth = static_cast<int>(juce::GlyphArrangement::getStringWidth(monoFont, beforeLink));
                
                link.bounds = juce::Rectangle<int>(lineX + beforeWidth, yPos, textWidth, lineHeight);
            }
        }
        
        // Variable removed - was unused
        
        // Verificar si esta línea tiene algún link
        bool hasLinkInLine = false;
        int linkIndexInLine = -1;
        for (size_t i = 0; i < links.size(); ++i)
        {
            if (line.contains(links[i].text))
            {
                hasLinkInLine = true;
                linkIndexInLine = static_cast<int>(i);
                if (hoveredLinkIndex == static_cast<int>(i))
                    break;
            }
        }
        
        // Siempre dibujar la línea completa primero
        g.drawText(line, textArea.getX(), yPos, textArea.getWidth(), lineHeight,
                  juce::Justification::centred, false);
        
        // Si hay un link con hover en esta línea, añadir efectos visuales
        if (hasLinkInLine && hoveredLinkIndex == linkIndexInLine)
        {
            int lineWidth = static_cast<int>(juce::GlyphArrangement::getStringWidth(monoFont, line));
            int startX = textArea.getCentreX() - lineWidth / 2;
            
            // Encontrar la posición del link en la línea
            int linkStart = line.indexOf(links[linkIndexInLine].text);
            juce::String beforeLink = line.substring(0, linkStart);
            juce::String linkText = links[linkIndexInLine].text;
            
            // Calcular posición exacta del link
            int linkX = startX + static_cast<int>(juce::GlyphArrangement::getStringWidth(monoFont, beforeLink));
            int linkWidth = static_cast<int>(juce::GlyphArrangement::getStringWidth(monoFont, linkText));
            
            // Dibujar highlight semi-transparente sobre el link (sin redibujar texto)
            g.setColour(juce::Colours::cyan.withAlpha(0.15f));
            g.fillRect(linkX, yPos, linkWidth, lineHeight);
            
            // Dibujar subrayado manual con color destacado
            g.setColour(textColour.brighter(0.3f));
            float underlineY = yPos + lineHeight - 2.0f;
            g.fillRect(linkX, static_cast<int>(underlineY), linkWidth, 1);
            
            // Restaurar color
            g.setColour(textColour);
        }
        
        // Espaciado adicional después del título
        if (isTitle)
        {
            yPos += lineHeight * 2.0f;  // Doble espaciado para crear línea en blanco
        }
        else
        {
            yPos += lineHeight;
        }
    }
}

void CreditsWindow::resized()
{
    // Posicionar el botón de idioma en la esquina superior derecha
    langButton.setBounds(getWidth() - 40, 5, 35, 20);
}

bool CreditsWindow::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        if (onClose)
            onClose();
        return true;
    }
    
    return false;
}

void CreditsWindow::mouseDown(const juce::MouseEvent& event)
{
    // Verificar si el clic fue en algún link
    bool clickedLink = false;
    for (size_t i = 0; i < links.size(); ++i)
    {
        if (links[i].bounds.contains(event.getPosition()))
        {
            juce::URL(links[i].url).launchInDefaultBrowser();
            clickedLink = true;
            break;
        }
    }
    
    // Cerrar la ventana si el clic fue en cualquier otro lugar
    if (!clickedLink && onClose)
    {
        onClose();
    }
}

void CreditsWindow::mouseMove(const juce::MouseEvent& event)
{
    // Buscar si el mouse está sobre algún link
    int newHoveredIndex = -1;
    for (size_t i = 0; i < links.size(); ++i)
    {
        if (links[i].bounds.contains(event.getPosition()))
        {
            newHoveredIndex = static_cast<int>(i);
            break;
        }
    }
    
    if (newHoveredIndex != hoveredLinkIndex)
    {
        hoveredLinkIndex = newHoveredIndex;
        setMouseCursor(hoveredLinkIndex >= 0 ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

//==============================================================================
// MÉTODOS PÚBLICOS - CreditsWindow
//==============================================================================
void CreditsWindow::buildFullCreditsText()
{
    if (isEnglish)
        buildFullCreditsTextEnglish();
    else
        buildFullCreditsTextSpanish();
}

void CreditsWindow::toggleLanguage()
{
    isEnglish = !isEnglish;
    langButton.setButtonText(isEnglish ? "ESP" : "ENG");
    buildFullCreditsText();
    repaint();
    
    // Notificar al padre para que ajuste el tamaño
    if (auto* parent = getParentComponent())
    {
        parent->resized();
    }
}

//==============================================================================
// MÉTODOS PRIVADOS - CreditsWindow
//==============================================================================
void CreditsWindow::buildFullCreditsTextSpanish()
{
    fullText.clear();
    links.clear();
    
    // Generar texto formateado con saltos de línea para llenar el diálogo
    fullText = "\nJCBMaximizer v1.0.0";
    if (pluginFormat.isNotEmpty())
        fullText += " (" + pluginFormat + ")";
    fullText += "\n";
    fullText += JUCE_UTF8(" Plugin de audio open source hecho con Max → gen~ → Plugin Export → JUCE \n");
    fullText += JUCE_UTF8(" © 2025 Juan Carlos Blancas \n\n");

    // Primer bloque: Tecnologías principales
    fullText += JUCE_UTF8(" gen~ (Max/MSP, Cycling'74) - Motor DSP \n");
    fullText += JUCE_UTF8(" JUCE 8 & CMake - Framework y sistema de construcción \n");
    fullText += JUCE_UTF8(" Claude Code – Mejora visual, funcional y optimización del código sobre diseño inicial básico \n\n");

    // Segundo bloque: Bibliografía técnica
    fullText += JUCE_UTF8(" Recursos \n");
    fullText += JUCE_UTF8(" Graham Wakefield & Gregory Taylor - Generating Sound and Organizing Time \n");
    fullText += JUCE_UTF8(" Will C. Pirkle - Designing Audio Effect Plugins in C++ \n");
    fullText += JUCE_UTF8(" Dynamic Range Compressor Design - Giannoulis, Massberg, Reiss \n");
    fullText += JUCE_UTF8(" The Complete Beginner's Guide to Audio Plug-in Development - Matthijs Hollemans \n\n");

    // Tercer bloque: Contribuciones de código base
    fullText += JUCE_UTF8(" Código base para vincular gen~ Plugin Export con JUCE APVTS (Kengo Suzuki) \n");
    fullText += JUCE_UTF8(" Código base para medidores (A. Murthy) | Tooltips (F. Becker) | Skin base/Presets (J. Peña)\n\n");

    // Cuarto bloque: Licencia y formatos compatibles
    fullText += JUCE_UTF8(" Este plugin ha sido desarrollado con el framework JUCE bajo licencia GPLv3. \n");
    fullText += JUCE_UTF8(" Formatos compatibles: AU, VST3 y AAX \n");

    // Definir los links clickeables disponibles
    links.push_back({" Juan Carlos Blancas ", "https://github.com/cjitter/JCBCompressor", {}});
    links.push_back({" gen~ (Max/MSP, Cycling'74) ", "https://cycling74.com", {}});
    links.push_back({" JUCE ", "https://github.com/juce-framework/JUCE", {}});
    links.push_back({" CMake ", "https://cmake.org", {}});
    links.push_back({" Claude Code ", "https://claude.ai/", {}});
    links.push_back({" Graham Wakefield & Gregory Taylor - Generating Sound and Organizing Time ", "https://cycling74.com/books/go", {}});
    links.push_back({" Will C. Pirkle - Designing Audio Effect Plugins in C++ ", "https://www.willpirkle.com", {}});
    links.push_back({" Dynamic Range Compressor Design - Giannoulis, Massberg, Reiss ", "https://eecs.qmul.ac.uk/~josh/documents/2012/GiannoulisMassbergReiss-dynamicrangecompression-JAES2012.pdf", {}});
    links.push_back({" (A. Murthy) ", "https://www.youtube.com/watch?v=ILMdPjFQ9ps", {}});
    links.push_back({" (F. Becker) ", "https://github.com/francoisbecker/fb-utils/blob/master/include/fbu/tooltip_component.hpp", {}});
    //links.push_back({" JUCE ", "https://juce.com", {}});
    links.push_back({" (Kengo Suzuki) ", "https://github.com/szkkng/jr-granular", {}});
    links.push_back({" The Complete Beginner's Guide to Audio Plug-in Development - Matthijs Hollemans ", "https://www.theaudioprogrammer.com/books/beginners-plugin-book", {}});
}

void CreditsWindow::buildFullCreditsTextEnglish() 
{
    fullText.clear();
    links.clear();

    // Generar texto formateado con saltos de línea para llenar el diálogo
    fullText = "\nJCBMaximizer v1.0.0";
    if (pluginFormat.isNotEmpty())
        fullText += " (" + pluginFormat + ")";
    fullText += "\n";
    fullText += JUCE_UTF8(" Open source audio plugin made with Max → gen~ → Plugin Export → JUCE \n");
    fullText += JUCE_UTF8(" © 2025 Juan Carlos Blancas \n\n");

    // Primer bloque: Tecnologías principales
    fullText += JUCE_UTF8(" gen~ (Max/MSP, Cycling'74) - DSP Engine \n");
    fullText += JUCE_UTF8(" JUCE 8 & CMake - Framework and build system \n");
    fullText += JUCE_UTF8(" Claude Code – Visual and functional improvement and code optimization over basic initial design \n\n");

    // Segundo bloque: Bibliografía técnica
    fullText += JUCE_UTF8(" Resources \n");
    fullText += JUCE_UTF8(" Graham Wakefield & Gregory Taylor - Generating Sound and Organizing Time \n");
    fullText += JUCE_UTF8(" Will C. Pirkle - Designing Audio Effect Plugins in C++ \n");
    fullText += JUCE_UTF8(" Dynamic Range Compressor Design - Giannoulis, Massberg, Reiss \n");
    fullText += JUCE_UTF8(" The Complete Beginner's Guide to Audio Plug-in Development - Matthijs Hollemans \n\n");

    // Tercer bloque: Contribuciones de código base
    fullText += JUCE_UTF8(" Base code for linking gen~ Plugin Export with JUCE APVTS (Kengo Suzuki) \n");
    fullText += JUCE_UTF8(" Base code for meters (A. Murthy) | Tooltips (F. Becker) | Base skin/Presets (J. Peña)\n\n");

    // Cuarto bloque: Licencia y formatos compatibles
    fullText += JUCE_UTF8(" This plugin has been developed with the JUCE framework under GPLv3 license. \n");
    fullText += JUCE_UTF8(" Compatible formats: AU, VST3 and AAX \n");

    // Definir los links clickeables disponibles (idénticos a la versión en español)
    links.push_back({" Juan Carlos Blancas ", "https://github.com/cjitter/JCBCompressor", {}});
    links.push_back({" gen~ (Max/MSP, Cycling'74) ", "https://cycling74.com", {}});
    links.push_back({" JUCE ", "https://github.com/juce-framework/JUCE", {}});
    links.push_back({" CMake ", "https://cmake.org", {}});
    links.push_back({" Claude Code ", "https://claude.ai/", {}});
    links.push_back({" Graham Wakefield & Gregory Taylor - Generating Sound and Organizing Time ", "https://cycling74.com/books/go", {}});
    links.push_back({" Will C. Pirkle - Designing Audio Effect Plugins in C++ ", "https://www.willpirkle.com", {}});
    links.push_back({" Dynamic Range Compressor Design - Giannoulis, Massberg, Reiss ", "https://eecs.qmul.ac.uk/~josh/documents/2012/GiannoulisMassbergReiss-dynamicrangecompression-JAES2012.pdf", {}});
    links.push_back({" (A. Murthy) ", "https://www.youtube.com/watch?v=ILMdPjFQ9ps", {}});
    links.push_back({" (F. Becker) ", "https://github.com/francoisbecker/fb-utils/blob/master/include/fbu/tooltip_component.hpp", {}});
    //links.push_back({" JUCE ", "https://juce.com", {}});
    links.push_back({" (Kengo Suzuki) ", "https://github.com/szkkng/jr-granular", {}});
    links.push_back({" The Complete Beginner's Guide to Audio Plug-in Development - Matthijs Hollemans ", "https://www.theaudioprogrammer.com/books/beginners-plugin-book", {}});
}

juce::Colour CreditsWindow::getTerminalColour() const
{
    // Usar blanco para el texto de créditos (estilo terminal monocromático)
    return juce::Colours::white;
}

juce::Rectangle<int> CreditsWindow::getLinkBounds() const
{
    // Calcular la posición aproximada del link en el texto (método legacy)
    // Basándonos en el layout actual de renderizado
    int padding = static_cast<int>(10 * (getHeight() / 120.0f));
    auto textArea = getLocalBounds().reduced(padding);
    
    // El link está cerca del final del texto
    // Estimamos su posición basándonos en el número de líneas
    float scaleFactor = getHeight() / 120.0f;
    float fontSize = 9.0f * scaleFactor;
    int lineHeight = static_cast<int>(fontSize * 1.2f);
    
    // El link está en la línea 11 aproximadamente
    int linkY = textArea.getY() + (textArea.getHeight() - lineHeight * 12) / 2 + lineHeight * 10;
    
    // Ancho estimado del link
    int linkWidth = static_cast<int>(fontSize * 30); // ~30 caracteres
    int linkX = textArea.getCentreX() - linkWidth / 2;
    
    return juce::Rectangle<int>(linkX, linkY, linkWidth, lineHeight);
}

//==============================================================================
// CONSTRUCTOR Y DESTRUCTOR - CreditsOverlay
//==============================================================================
CreditsOverlay::CreditsOverlay(const juce::String& pluginFormat)
    : creditsWindow(pluginFormat)
{
    addAndMakeVisible(creditsWindow);
    
    // Conectar callbacks para delegación de cierre
    creditsWindow.onClose = [this]() {
        if (onClose)
            onClose();
    };
    
    setInterceptsMouseClicks(true, true);
}

//==============================================================================
// OVERRIDES DE COMPONENT - CreditsOverlay
//==============================================================================
void CreditsOverlay::paint(juce::Graphics& g)
{
    // Fondo semi-transparente como los otros diálogos del plugin
    g.fillAll(juce::Colours::black.withAlpha(0.5f));
    
    // Sin aureola - solo el fondo oscuro translúcido
}

void CreditsOverlay::resized()
{
    auto bounds = getLocalBounds();
    
    // La ventana de créditos siempre ocupa toda la ventana del plugin
    // tanto en español como en inglés para máxima legibilidad
    creditsWindow.setBounds(bounds);
}

void CreditsOverlay::mouseDown(const juce::MouseEvent& event)
{
    // Solo cerrar si el clic fue fuera de la ventana de créditos
    // (aunque actualmente creditsWindow ocupa todo el área)
    if (!creditsWindow.getBounds().contains(event.getPosition()))
    {
        if (onClose)
            onClose();
    }
}