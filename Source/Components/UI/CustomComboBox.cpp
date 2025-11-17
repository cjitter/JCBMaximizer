//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBMaximizer and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "CustomComboBox.h"

//==============================================================================
// CONSTRUCTOR Y DESTRUCTOR
//==============================================================================

CustomComboBox::CustomComboBox()
{
    setSize(100, 20);
    setWantsKeyboardFocus(true);
    setMouseClickGrabsKeyboardFocus(true);
}

CustomComboBox::~CustomComboBox()
{
    hidePopup();
}

//==============================================================================
// OVERRIDES DE COMPONENT - RENDERING Y EVENTOS
//==============================================================================

void CustomComboBox::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Estilo similar a los botones del plugin
    g.setColour(juce::Colours::transparentBlack);
    g.fillRoundedRectangle(bounds, 2.0f);
    
    // Contorno con el mismo estilo que los otros botones
    g.setColour(DarkTheme::textSecondary.withAlpha(0.7f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 2.0f, 1.0f);
    
    // Calcular área para el triángulo (a la izquierda)
    auto arrowBounds = bounds.removeFromLeft(16.0f).reduced(4.0f, 3.0f);
    
    // Área para el texto (lo que queda después del triángulo)
    auto textBounds = bounds.reduced(3.0f, 2.0f);
    
    // Triángulo estilizado hacia abajo similar a los botones < y >
    juce::Path arrow;
    auto arrowCenterX = arrowBounds.getCentreX();
    auto arrowCenterY = arrowBounds.getCentreY();
    auto arrowSize = 4.0f;
    
    // Crear un triángulo más definido y estilizado
    arrow.addTriangle(arrowCenterX - arrowSize, arrowCenterY - arrowSize/2,
                     arrowCenterX + arrowSize, arrowCenterY - arrowSize/2,
                     arrowCenterX, arrowCenterY + arrowSize/2);
    
    // Color y estilo similar a los botones de navegación
    g.setColour(DarkTheme::textPrimary.withAlpha(0.8f));
    g.fillPath(arrow);
    
    // Añadir un contorno sutil para mayor definición
    g.setColour(DarkTheme::textSecondary.withAlpha(0.5f));
    g.strokePath(arrow, juce::PathStrokeType(0.5f));
    
    // Texto con el mismo estilo que los otros botones
    g.setColour(DarkTheme::textPrimary);
    auto font = juce::Font(juce::FontOptions(14.0f)); // Consistencia con otros componentes
    if (isTextItalic) {
        font.setItalic(true);
    }
    g.setFont(font);
    
    auto displayText = getText();
    if (displayText.isEmpty() && !textWhenNothingSelected.isEmpty())
        displayText = textWhenNothingSelected;
    if (items.isEmpty() && !textWhenNoChoicesAvailable.isEmpty())
        displayText = textWhenNoChoicesAvailable;
        
    g.drawFittedText(displayText, textBounds.toNearestInt(), juce::Justification::centred, 1);
}

void CustomComboBox::resized()
{
    if (popupWindow != nullptr)
        hidePopup();
}

void CustomComboBox::mouseDown(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    
    // Verificar si el componente está habilitado antes de mostrar popup
    if (!isEnabled())
        return;
    
    // Alternar popup siempre
    if (isPopupShown)
    {
        hidePopup();
    }
    else
    {
        showPopup();
    }
        
    repaint();
}

void CustomComboBox::mouseEnter(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    repaint();
}

void CustomComboBox::mouseExit(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    repaint();
}

bool CustomComboBox::hitTest(int x, int y)
{
    return getLocalBounds().contains(x, y);
}

//==============================================================================
// INTERFAZ PÚBLICA DEL COMBOBOX - GESTIÓN DE ITEMS
//==============================================================================

void CustomComboBox::addItem(const juce::String& text, int itemId)
{
    items.add({text, itemId, false, false, {}, {}});
}

void CustomComboBox::addItemList(const juce::StringArray& itemsToAdd, int firstId)
{
    for (int i = 0; i < itemsToAdd.size(); ++i)
    {
        addItem(itemsToAdd[i], firstId + i);
    }
}

void CustomComboBox::addCategoryItem(const juce::String& categoryName, 
                                    const juce::StringArray& subItems, 
                                    int startId)
{
    Item item;
    item.text = categoryName;
    item.id = startId;
    item.isCategory = true;
    item.isSeparator = false;
    item.subItems = subItems;
    
    // Asignar IDs a los sub-items
    for (int i = 0; i < subItems.size(); ++i)
    {
        item.subItemIds.push_back(startId + i + 1);
    }
    
    items.add(item);
}

void CustomComboBox::clear()
{
    items.clear();
    selectedId = 0;
    hidePopup();
    repaint();
}

void CustomComboBox::setSelectedId(int newItemId)
{
    if (selectedId != newItemId)
    {
        selectedId = newItemId;
        repaint();
        if (onChange)
            onChange();
    }
}

void CustomComboBox::setSelectedIdWithoutNotification(int newItemId)
{
    if (selectedId != newItemId)
    {
        selectedId = newItemId;
        repaint();
    }
}

int CustomComboBox::getSelectedId() const
{
    return selectedId;
}

int CustomComboBox::getNumItems() const
{
    return items.size();
}

juce::String CustomComboBox::getItemText(int index) const
{
    if (index >= 0 && index < items.size())
        return items[index].text;
    return {};
}

int CustomComboBox::getItemId(int index) const
{
    if (index >= 0 && index < items.size())
        return items[index].id;
    return 0;
}

juce::String CustomComboBox::getText() const
{
    for (const auto& item : items)
    {
        if (item.id == selectedId && !item.isCategory)
            return item.text;
            
        // Si es una categoría, buscar en sus sub-items
        if (item.isCategory)
        {
            for (int i = 0; i < static_cast<int>(item.subItemIds.size()); ++i)
            {
                if (item.subItemIds[i] == selectedId)
                {
                    // Retornar el texto del subitem
                    if (i < item.subItems.size())
                        return item.subItems[i];
                }
            }
        }
    }
    return {};
}

std::vector<int> CustomComboBox::getAllSelectableIds() const
{
    std::vector<int> selectableIds;
    
    // Recorrer todos los items
    for (const auto& item : items)
    {
        // Si es un separador, saltarlo
        if (item.isSeparator || item.text.startsWith("---"))
            continue;
            
        // Si es una categoría con sub-items, añadir los IDs de los sub-items
        if (item.isCategory && !item.subItemIds.empty())
        {
            // Añadir todos los IDs de los sub-items
            for (int subId : item.subItemIds)
            {
                selectableIds.push_back(subId);
            }
        }
        // Si es un item normal (no categoría, no separador), añadir su ID
        else if (!item.isCategory)
        {
            selectableIds.push_back(item.id);
        }
    }
    
    return selectableIds;
}

//==============================================================================
// CONFIGURACIÓN DE TEXTO Y ESTILO
//==============================================================================

void CustomComboBox::setTextWhenNothingSelected(const juce::String& newMessage)
{
    textWhenNothingSelected = newMessage;
    repaint();
}

void CustomComboBox::setTextWhenNoChoicesAvailable(const juce::String& newMessage)
{
    textWhenNoChoicesAvailable = newMessage;
    repaint();
}

void CustomComboBox::setJustificationType(juce::Justification newJustification)
{
    justification = newJustification;
    repaint();
}

void CustomComboBox::setTextItalic(bool shouldBeItalic)
{
    isTextItalic = shouldBeItalic;
    repaint();
}

juce::String CustomComboBox::getTextWhenNothingSelected() const
{
    return textWhenNothingSelected;
}

//==============================================================================
// GESTIÓN DE POPUP - MOSTRAR Y OCULTAR
//==============================================================================

void CustomComboBox::showPopup()
{
    if (isPopupShown || items.isEmpty())
        return;
        
    popupWindow = std::make_unique<PopupWindow>(*this);
    mouseInterceptor = std::make_unique<MouseInterceptor>(*this);
    
    // Calcular posición y tamaño del popup
    auto itemHeight = 22;
    auto popupHeight = juce::jmin(items.size() * itemHeight, 300); // Máximo 300px de altura
    auto popupWidth = getWidth();
    
    // Encontrar el componente de nivel superior (la ventana del plugin)
    auto* topLevelComp = getTopLevelComponent();
    if (topLevelComp != nullptr)
    {
        // Primero añadir el interceptor que cubre toda la ventana
        mouseInterceptor->setBounds(topLevelComp->getLocalBounds());
        topLevelComp->addAndMakeVisible(mouseInterceptor.get());
        
        // Posicionar debajo del combobox relativo al componente padre
        auto localBounds = getLocalBounds();
        auto boundsInParent = localAreaToGlobal(localBounds);
        auto parentPosition = topLevelComp->getScreenPosition();
        
        popupWindow->setBounds(boundsInParent.getX() - parentPosition.getX(), 
                              boundsInParent.getBottom() - parentPosition.getY(), 
                              popupWidth, popupHeight);
        
        // Añadir el popup después del interceptor para que esté encima
        topLevelComp->addAndMakeVisible(popupWindow.get());
    }
    
    popupWindow->setVisible(true);
    popupWindow->toFront(false);
    
    isPopupShown = true;
}

void CustomComboBox::hidePopup()
{
    if (popupWindow != nullptr)
    {
        if (auto* parent = popupWindow->getParentComponent())
            parent->removeChildComponent(popupWindow.get());
        popupWindow.reset();
    }
    
    if (mouseInterceptor != nullptr)
    {
        if (auto* parent = mouseInterceptor->getParentComponent())
            parent->removeChildComponent(mouseInterceptor.get());
        mouseInterceptor.reset();
    }
    
    isPopupShown = false;
}

void CustomComboBox::selectItem(int itemId)
{
    setSelectedId(itemId);
    hidePopup();
}

//==============================================================================
// IMPLEMENTACIÓN DE POPUP WINDOW
//==============================================================================

CustomComboBox::PopupWindow::PopupWindow(CustomComboBox& owner) : comboBox(owner)
{
    setWantsKeyboardFocus(true);
    setMouseClickGrabsKeyboardFocus(false);
    setAlwaysOnTop(true);
    
    // Crear el viewport para scroll
    viewport = std::make_unique<juce::Viewport>();
    viewport->setScrollBarsShown(true, false); // Solo scroll vertical
    
    // Crear el contenedor de la lista
    listContainer = std::make_unique<ListContainer>(*this);
    viewport->setViewedComponent(listContainer.get(), false);
    
    // Configurar el viewport con estilo del tema
    viewport->setScrollBarThickness(8);
    viewport->getVerticalScrollBar().setColour(juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
    viewport->getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId, DarkTheme::textSecondary.withAlpha(0.5f));
    
    addAndMakeVisible(viewport.get());
}

void CustomComboBox::PopupWindow::paint(juce::Graphics& g)
{
    // Dibujar un fondo con tinte púrpura translúcido para integrar con el tema
    g.setColour(DarkTheme::backgroundMedium.overlaidWith(DarkTheme::accentSecondary.withAlpha(0.1f)).withAlpha(0.85f));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
    
    // Dibujar un borde con el color de acento del tema
    g.setColour(DarkTheme::accent.withAlpha(0.4f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);
}

void CustomComboBox::PopupWindow::resized()
{
    // El viewport ocupa todo el espacio del popup
    viewport->setBounds(getLocalBounds());
    
    // El contenedor de la lista tiene la altura total necesaria
    auto itemHeight = 22;
    auto totalHeight = comboBox.items.size() * itemHeight;
    listContainer->setSize(getWidth(), totalHeight);
    
    // Forzar repintado del contenedor
    listContainer->repaint();
}

void CustomComboBox::PopupWindow::mouseDown(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    // Los eventos del mouse son manejados por el ListContainer dentro del viewport
}

void CustomComboBox::PopupWindow::mouseMove(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    // Los eventos del mouse son manejados por el ListContainer dentro del viewport
}

void CustomComboBox::PopupWindow::mouseExit(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    // Los eventos del mouse son manejados por el ListContainer dentro del viewport
}

bool CustomComboBox::PopupWindow::hitTest(int x, int y)
{
    return getLocalBounds().contains(x, y);
}

void CustomComboBox::PopupWindow::inputAttemptWhenModal()
{
    comboBox.hidePopup();
}

void CustomComboBox::PopupWindow::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    // Pasar el evento al viewport para que maneje el scroll
    if (viewport != nullptr)
        viewport->mouseWheelMove(event, wheel);
}

//==============================================================================
// IMPLEMENTACIÓN DE LIST CONTAINER
//==============================================================================

void CustomComboBox::PopupWindow::ListContainer::paint(juce::Graphics& g)
{
    // Fondo con el mismo tinte del popup pero un poco más transparente
    g.fillAll(DarkTheme::backgroundMedium.overlaidWith(DarkTheme::accentSecondary.withAlpha(0.08f)).withAlpha(0.8f));
    
    // Dibujar items
    auto itemHeight = 22;
    for (int i = 0; i < popupWindow.comboBox.items.size(); ++i)
    {
        const auto& item = popupWindow.comboBox.items[i];
        auto itemBounds = juce::Rectangle<float>(0.0f, 
                                                  static_cast<float>(i * itemHeight), 
                                                  static_cast<float>(getWidth()), 
                                                  static_cast<float>(itemHeight));
        
        // Highlight si está hovering
        if (i == popupWindow.hoveredItem)
        {
            g.setColour(DarkTheme::accent.withAlpha(0.3f));
            g.fillRect(itemBounds);
        }
        
        // Highlight si está seleccionado (solo items normales)
        if (!item.isCategory && !item.isSeparator && item.id == popupWindow.comboBox.selectedId)
        {
            g.setColour(DarkTheme::accent.withAlpha(0.2f));
            g.fillRect(itemBounds);
        }
        
        // Dibujar según tipo de item
        if (item.isSeparator || item.text.startsWith("---"))
        {
            // Dibujar línea separadora
            g.setColour(DarkTheme::textMuted.withAlpha(0.3f));
            auto lineY = itemBounds.getCentreY();
            g.drawLine(itemBounds.getX() + 10, lineY, itemBounds.getRight() - 10, lineY, 1.0f);
        }
        else if (item.isCategory)
        {
            // Dibujar categoría con flecha
            g.setColour(DarkTheme::textPrimary);
            g.setFont(juce::Font(juce::FontOptions(14.0f)).withStyle(juce::Font::bold));
            g.drawFittedText(item.text, 
                            itemBounds.reduced(8.0f, 2.0f).toNearestInt(),
                            juce::Justification::centredLeft, 1);
            
            // Dibujar flecha >
            g.setColour(DarkTheme::textMuted);
            g.setFont(juce::Font(juce::FontOptions(14.0f)));
            g.drawFittedText(">", 
                            itemBounds.reduced(8.0f, 2.0f).toNearestInt(),
                            juce::Justification::centredRight, 1);
        }
        else
        {
            // Item normal
            g.setColour(DarkTheme::textPrimary);
            g.setFont(juce::Font(juce::FontOptions(14.0f)));
            g.drawFittedText(item.text, 
                            itemBounds.reduced(8.0f, 2.0f).toNearestInt(),
                            juce::Justification::centredLeft, 1);
        }
    }
}

void CustomComboBox::PopupWindow::ListContainer::mouseDown(const juce::MouseEvent& event)
{
    auto itemHeight = 22;
    int clickedItem = event.y / itemHeight;
    
    if (clickedItem >= 0 && clickedItem < popupWindow.comboBox.items.size())
    {
        const auto& item = popupWindow.comboBox.items[clickedItem];
        
        if (item.isCategory)
        {
            // El submenú se mostrará en mouseMove cuando detecte hover sobre categoría
            // Aquí solo actualizamos el item hovereado para forzar la creación del submenú
            if (popupWindow.hoveredItem != clickedItem)
            {
                popupWindow.hoveredItem = clickedItem;
                repaint();
            }
        }
        else if (!item.isSeparator && !item.text.startsWith("---"))
        {
            // Solo seleccionar items normales (no categorías ni separadores)
            popupWindow.comboBox.selectItem(item.id);
        }
    }
}

void CustomComboBox::PopupWindow::ListContainer::mouseMove(const juce::MouseEvent& event)
{
    auto itemHeight = 22;
    int newHoveredItem = event.y / itemHeight;
    
    if (newHoveredItem >= 0 && newHoveredItem < popupWindow.comboBox.items.size())
    {
        if (popupWindow.hoveredItem != newHoveredItem)
        {
            popupWindow.hoveredItem = newHoveredItem;
            repaint();
            
            // Verificar si es una categoría para mostrar submenú
            const auto& item = popupWindow.comboBox.items[newHoveredItem];
            if (item.isCategory && !item.subItems.isEmpty())
            {
                // Ocultar submenú anterior si existe
                if (popupWindow.activeSubMenu != nullptr)
                {
                    if (auto* parent = popupWindow.activeSubMenu->getParentComponent())
                        parent->removeChildComponent(popupWindow.activeSubMenu.get());
                    popupWindow.activeSubMenu.reset();
                }
                
                // Crear nuevo submenú
                popupWindow.activeSubMenu = std::make_unique<SubMenu>(popupWindow, 
                                                                      item.subItems, 
                                                                      item.subItemIds);
                
                // Calcular posición del submenú (a la derecha del item de categoría)
                auto itemBounds = juce::Rectangle<int>(0, newHoveredItem * itemHeight, 
                                                       getWidth(), itemHeight);
                
                // Posicionar el submenú a la derecha del popup principal
                auto subMenuWidth = 150; // Ancho del submenú
                auto subMenuHeight = static_cast<int>(item.subItems.size() * itemHeight);
                
                // Ajustar posición para que no se salga de la pantalla
                auto* topLevel = getTopLevelComponent();
                if (topLevel != nullptr)
                {
                    auto popupBounds = topLevel->getLocalArea(&popupWindow, popupWindow.getLocalBounds());
                    auto subMenuX = popupBounds.getRight();
                    auto subMenuY = popupBounds.getY() + itemBounds.getY();
                    
                    popupWindow.activeSubMenu->setBounds(subMenuX, subMenuY, subMenuWidth, subMenuHeight);
                    topLevel->addAndMakeVisible(popupWindow.activeSubMenu.get());
                }
                
                popupWindow.hoveredCategoryIndex = newHoveredItem;
            }
            else if (!item.isCategory)
            {
                // Si no es categoría, ocultar cualquier submenú activo
                if (popupWindow.activeSubMenu != nullptr)
                {
                    if (auto* parent = popupWindow.activeSubMenu->getParentComponent())
                        parent->removeChildComponent(popupWindow.activeSubMenu.get());
                    popupWindow.activeSubMenu.reset();
                    popupWindow.hoveredCategoryIndex = -1;
                }
            }
        }
    }
}

void CustomComboBox::PopupWindow::ListContainer::mouseExit(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    
    // Solo ocultar el submenú si el mouse no está yendo hacia él
    if (popupWindow.activeSubMenu != nullptr)
    {
        // Verificar si el mouse está sobre el submenú
        auto mousePos = event.getEventRelativeTo(popupWindow.activeSubMenu.get()).position;
        if (!popupWindow.activeSubMenu->getLocalBounds().contains(mousePos.toInt()))
        {
            // Dar un pequeño delay antes de cerrar para permitir transición suave
            juce::Timer::callAfterDelay(100, [this]() {
                if (popupWindow.activeSubMenu != nullptr)
                {
                    // Verificar nuevamente si el mouse no está sobre el submenú
                    auto currentMousePos = juce::Desktop::getInstance().getMousePosition();
                    auto subMenuScreenBounds = popupWindow.activeSubMenu->getScreenBounds();
                    
                    if (!subMenuScreenBounds.contains(currentMousePos))
                    {
                        if (auto* parent = popupWindow.activeSubMenu->getParentComponent())
                            parent->removeChildComponent(popupWindow.activeSubMenu.get());
                        popupWindow.activeSubMenu.reset();
                        popupWindow.hoveredCategoryIndex = -1;
                    }
                }
            });
        }
    }
    
    if (popupWindow.hoveredItem != -1)
    {
        popupWindow.hoveredItem = -1;
        repaint();
    }
}

//==============================================================================
// IMPLEMENTACIÓN DE MOUSE INTERCEPTOR
//==============================================================================

void CustomComboBox::MouseInterceptor::paint(juce::Graphics& g)
{
    // Dibujar un fondo con tinte púrpura muy sutil para integrar con el tema
    g.fillAll(DarkTheme::backgroundDark.overlaidWith(DarkTheme::accentSecondary.withAlpha(0.05f)).withAlpha(0.3f));
    
    // Si hay un popup visible, podemos destacar su área con un halo sutil
    if (comboBox.popupWindow != nullptr)
    {
        // Obtener la posición del popup relativa a este componente
        auto popupBounds = getLocalArea(comboBox.popupWindow.get(), 
                                      comboBox.popupWindow->getLocalBounds());
        
        // Dibujar un halo sutil con el color de acento
        g.setColour(DarkTheme::accent.withAlpha(0.1f));
        g.fillRoundedRectangle(popupBounds.expanded(15).toFloat(), 12.0f);
        
        // Halo más intenso cerca del popup
        g.setColour(DarkTheme::accentSecondary.withAlpha(0.08f));
        g.fillRoundedRectangle(popupBounds.expanded(8).toFloat(), 8.0f);
    }
}

void CustomComboBox::MouseInterceptor::mouseDown(const juce::MouseEvent& event)
{
    // Convertir las coordenadas del evento a coordenadas del combobox
    auto clickPoint = comboBox.getLocalPoint(this, event.getPosition());
    
    // Si el click está dentro del combobox, cerrar el popup
    if (comboBox.getLocalBounds().contains(clickPoint))
    {
        comboBox.hidePopup();
        return;
    }
    
    // Si hay un popup, verificar si el click está dentro del popup
    if (comboBox.popupWindow != nullptr)
    {
        auto popupClickPoint = comboBox.popupWindow->getLocalPoint(this, event.getPosition());
        if (comboBox.popupWindow->getLocalBounds().contains(popupClickPoint))
            return;
    }
    
    // Si llegamos aquí, el click fue fuera del combobox y del popup
    comboBox.hidePopup();
}

//==============================================================================
// IMPLEMENTACIÓN DE SUBMENU
//==============================================================================

CustomComboBox::PopupWindow::SubMenu::SubMenu(PopupWindow& parent, 
                                              const juce::StringArray& items, 
                                              const std::vector<int>& ids)
    : parentWindow(parent), subItems(items), subItemIds(ids)
{
    setAlwaysOnTop(true);
    setOpaque(false);
}

void CustomComboBox::PopupWindow::SubMenu::paint(juce::Graphics& g)
{
    // Fondo del submenú con mayor opacidad para distinguirlo mejor
    g.fillAll(DarkTheme::backgroundMedium.overlaidWith(DarkTheme::accentSecondary.withAlpha(0.15f)).withAlpha(0.95f));
    
    // Borde más visible con color de acento
    g.setColour(DarkTheme::accent.withAlpha(0.5f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 3.0f, 1.0f);
    
    // Dibujar items
    const int itemHeight = 22;
    for (int i = 0; i < subItems.size(); ++i)
    {
        auto itemBounds = juce::Rectangle<float>(0.0f, 
                                                  static_cast<float>(i * itemHeight), 
                                                  static_cast<float>(getWidth()), 
                                                  static_cast<float>(itemHeight));
        
        // Highlight si está hovering
        if (i == hoveredIndex)
        {
            g.setColour(DarkTheme::accent.withAlpha(0.3f));
            g.fillRect(itemBounds);
        }
        
        // Texto del item
        g.setColour(DarkTheme::textPrimary);
        g.setFont(juce::Font(juce::FontOptions(14.0f)));
        g.drawFittedText(subItems[i], 
                        itemBounds.reduced(8.0f, 2.0f).toNearestInt(),
                        juce::Justification::centredLeft, 1);
    }
}

void CustomComboBox::PopupWindow::SubMenu::mouseDown(const juce::MouseEvent& event)
{
    const int itemHeight = 22;
    int clickedIndex = event.y / itemHeight;
    
    if (clickedIndex >= 0 && clickedIndex < subItems.size())
    {
        if (clickedIndex < static_cast<int>(subItemIds.size()))
        {
            // Obtener el ID del item seleccionado
            int selectedItemId = subItemIds[static_cast<size_t>(clickedIndex)];
            
            // Seleccionar el item en el combo principal
            parentWindow.comboBox.selectItem(selectedItemId);
        }
    }
}

void CustomComboBox::PopupWindow::SubMenu::mouseMove(const juce::MouseEvent& event)
{
    const int itemHeight = 22;
    int newHoveredIndex = event.y / itemHeight;
    
    if (newHoveredIndex >= 0 && newHoveredIndex < subItems.size())
    {
        if (hoveredIndex != newHoveredIndex)
        {
            hoveredIndex = newHoveredIndex;
            repaint();
        }
    }
}