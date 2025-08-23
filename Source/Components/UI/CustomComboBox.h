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
#include "../Windows/DarkThemeColors.h"

//==============================================================================
// CLASE PRINCIPAL DEL CUSTOM COMBOBOX
//==============================================================================

/**
 * CustomComboBox personalizado para el plugin JCBCompressor
 * 
 * Diseñado específicamente para la selección de algoritmos de detección
 * con estilo visual integrado al tema del plugin. Incluye:
 * - Popup window personalizada con scroll automático
 * - Sistema de interceptor de mouse para clicks externos
 * - Estilo visual integrado con DarkTheme
 * - Viewport con scroll para listas largas
 */
class CustomComboBox : public juce::Component
{
public:
    //==========================================================================
    // CONSTRUCTOR Y DESTRUCTOR
    //==========================================================================
    
    CustomComboBox();
    ~CustomComboBox() override;

    //==========================================================================
    // OVERRIDES DE COMPONENT
    //==========================================================================
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    bool hitTest(int x, int y) override;

    //==========================================================================
    // INTERFAZ PÚBLICA DEL COMBOBOX
    //==========================================================================
    
    void addItem(const juce::String& text, int itemId);
    void addItemList(const juce::StringArray& itemsToAdd, int firstId);
    void addCategoryItem(const juce::String& categoryName, const juce::StringArray& subItems, int startId);
    void clear();
    void setSelectedId(int newItemId);
    void setSelectedIdWithoutNotification(int newItemId);
    int getSelectedId() const;
    int getNumItems() const;
    juce::String getText() const;
    juce::String getItemText(int index) const;
    int getItemId(int index) const;
    void setTextWhenNothingSelected(const juce::String& newMessage);
    void setTextWhenNoChoicesAvailable(const juce::String& newMessage);
    void setJustificationType(juce::Justification justification);
    void setTextItalic(bool shouldBeItalic);
    juce::String getTextWhenNothingSelected() const;
    std::vector<int> getAllSelectableIds() const;
    
    //==========================================================================
    // CALLBACK DE CAMBIO
    //==========================================================================
    
    std::function<void()> onChange;

private:
    //==========================================================================
    // POPUP WINDOW PARA LISTA DE ITEMS
    //==========================================================================
    
    /**
     * Ventana popup que contiene la lista de items seleccionables
     * Incluye viewport con scroll automático para listas largas
     */
    struct PopupWindow : public juce::Component
    {
        //======================================================================
        // CONTENEDOR DE LISTA CON SCROLL
        //======================================================================
        
        /**
         * Contenedor interno para los items de la lista
         * Maneja el rendering de items y eventos de mouse
         */
        struct ListContainer : public juce::Component
        {
            ListContainer(PopupWindow& owner) : popupWindow(owner) {}
            
            void paint(juce::Graphics& g) override;
            void mouseDown(const juce::MouseEvent& event) override;
            void mouseMove(const juce::MouseEvent& event) override;
            void mouseExit(const juce::MouseEvent& event) override;
            
            PopupWindow& popupWindow;
        };
        
        //======================================================================
        // CONSTRUCTOR Y OVERRIDES
        //======================================================================
        
        explicit PopupWindow(CustomComboBox& owner);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseMove(const juce::MouseEvent& event) override;
        void mouseExit(const juce::MouseEvent& event) override;
        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
        bool hitTest(int x, int y) override;
        void inputAttemptWhenModal() override;
        void resized() override;
        
        //======================================================================
        // SUBMENU PARA CATEGORÍAS
        //======================================================================
        
        class SubMenu : public juce::Component
        {
        public:
            SubMenu(PopupWindow& parent, const juce::StringArray& items, const std::vector<int>& ids);
            void paint(juce::Graphics& g) override;
            void mouseDown(const juce::MouseEvent& event) override;
            void mouseMove(const juce::MouseEvent& event) override;
            
            PopupWindow& parentWindow;
            juce::StringArray subItems;
            std::vector<int> subItemIds;
            int hoveredIndex = -1;
        };
        
        //======================================================================
        // VARIABLES DE ESTADO
        //======================================================================
        
        CustomComboBox& comboBox;
        int hoveredItem = -1;
        std::unique_ptr<SubMenu> activeSubMenu;
        int hoveredCategoryIndex = -1;
        
        // Componentes para el sistema de scroll
        std::unique_ptr<ListContainer> listContainer;
        std::unique_ptr<juce::Viewport> viewport;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PopupWindow)
    };
    
    //==========================================================================
    // INTERCEPTOR DE MOUSE PARA CLICKS EXTERNOS
    //==========================================================================
    
    /**
     * Componente interceptor para capturar clicks fuera del popup
     * Se extiende sobre toda la ventana padre para detectar clicks externos
     * y cerrar automáticamente el popup
     */
    struct MouseInterceptor : public juce::Component
    {
        explicit MouseInterceptor(CustomComboBox& owner) : comboBox(owner) 
        {
            setInterceptsMouseClicks(true, true);
            setAlwaysOnTop(true);
        }
        
        void mouseDown(const juce::MouseEvent& event) override;
        void paint(juce::Graphics& g) override;
        
        CustomComboBox& comboBox;
    };
    
    //==========================================================================
    // MÉTODOS PRIVADOS DE GESTIÓN DE POPUP
    //==========================================================================
    
    void showPopup();
    void hidePopup();
    void selectItem(int itemId);
    
    //==========================================================================
    // ESTRUCTURA DE DATOS PARA ITEMS
    //==========================================================================
    
    struct Item
    {
        juce::String text;
        int id;
        bool isCategory = false;
        bool isSeparator = false;
        juce::StringArray subItems;
        std::vector<int> subItemIds;
    };
    
    //==========================================================================
    // VARIABLES DE ESTADO PRINCIPALES
    //==========================================================================
    
    juce::Array<Item> items;
    int selectedId = 0;
    juce::String textWhenNothingSelected;
    juce::String textWhenNoChoicesAvailable;
    juce::Justification justification = juce::Justification::centredLeft;
    bool isTextItalic = false;
    
    //==========================================================================
    // GESTIÓN DE POPUP Y UI STATE
    //==========================================================================
    
    std::unique_ptr<PopupWindow> popupWindow;
    std::unique_ptr<MouseInterceptor> mouseInterceptor;
    bool isPopupShown = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomComboBox)
};