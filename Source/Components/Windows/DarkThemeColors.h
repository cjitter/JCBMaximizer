//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBMaximizer and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================
#pragma once

//==============================================================================
// PALETA DE COLORES DARK THEME - JCBCompressor
//==============================================================================

#include <juce_graphics/juce_graphics.h>

using namespace juce;

/**
 * Paleta de colores del tema oscuro moderno para JCBCompressor
 * 
 * Sistema unificado de colores que define toda la estética visual del plugin.
 * Incluye colores para backgrounds, accents, texto, controles, medidores y 
 * funciones helper para gradientes. Optimizado para legibilidad y coherencia
 * visual en entornos de audio profesional.
 */
namespace DarkTheme
{
    //==========================================================================
    // COLORES DE BACKGROUND
    //==========================================================================
    const Colour backgroundDark = Colour(0xff1a1a1a);     // Fondo principal oscuro
    const Colour backgroundMedium = Colour(0xff2d2d2d);   // Fondo intermedio
    const Colour backgroundLight = Colour(0xff3a3a3a);    // Fondo claro para controles
    
    //==========================================================================
    // COLORES DE ACCENT
    //==========================================================================
    const Colour accent = Colour(0xff4a9eff);             // Blue accent principal
    const Colour accentSecondary = Colour(0xff7c4dff);    // Purple accent secundario
    const Colour accentWarm = Colour(0xffff6b35);         // Orange accent cálido
    
    //==========================================================================
    // COLORES DE TEXTO
    //==========================================================================
    const Colour textPrimary = Colour(0xffffffff);        // Texto principal blanco
    const Colour textSecondary = Colour(0xffb0b0b0);      // Texto secundario gris claro
    const Colour textMuted = Colour(0xff808080);          // Texto tenue gris oscuro
    
    //==========================================================================
    // COLORES DE CONTROLES
    //==========================================================================
    const Colour sliderTrack = Colour(0xff404040);        // Track de sliders
    const Colour sliderThumb = accent;                     // Thumb de sliders (blue)
    const Colour sliderFill = accent.withAlpha(0.8f);     // Fill de sliders
    
    //==========================================================================
    // COLORES DE BOTONES
    //==========================================================================
    const Colour buttonNormal = backgroundLight;          // Estado normal
    const Colour buttonHover = Colour(0xff4a4a4a);        // Estado hover
    const Colour buttonPressed = accent.withAlpha(0.3f);  // Estado pressed
    const Colour buttonText = textPrimary;                // Texto de botones
    
    //==========================================================================
    // COLORES DE MEDIDORES - Tema Violet/Blue/Purple
    //==========================================================================
    const Colour meterBackground = Colour(0xff252525);        // Fondo de medidores
    const Colour meterLow = Colour(0xff9c27b0);              // Purple para niveles bajos
    const Colour meterMid = Colour(0xff673ab7);              // Deep purple intermedio
    const Colour meterHigh = Colour(0xff3f51b5);             // Indigo para niveles altos
    const Colour meterClip = Colour(0xff2196f3);             // Blue para clip
    const Colour meterClipIndicator = Colour(0xffff0000);    // Red para detección 0dBFS
    
    //==========================================================================
    // COLORES DE MODO DELTA - Tema Teal/Cyan
    //==========================================================================
    const Colour deltaLow = Colour(0xff00bcd4);              // Cyan claro
    const Colour deltaMid = Colour(0xff00acc1);              // Dark cyan
    const Colour deltaHigh = Colour(0xff0097a7);             // Darker cyan
    const Colour deltaClip = Colour(0xff00838f);             // Darkest cyan
    
    //==========================================================================
    // COLORES DE MODO SOLO SC - Tema Red
    //==========================================================================
    const Colour soloScLow = Colour(0xffff6b6b);             // Light red
    const Colour soloScMid = Colour(0xffee5a24);             // Orange red
    const Colour soloScHigh = Colour(0xffd32f2f);            // Medium red
    const Colour soloScClip = Colour(0xffc62828);            // Dark red
    
    //==========================================================================
    // COLORES DE GAIN REDUCTION
    //==========================================================================
    const Colour gainReductionStart = Colour(0xffff6b6b);    // Rojo coral brillante
    const Colour gainReductionEnd = Colour(0xffee5a24);      // Naranja rojizo
    
    //==========================================================================
    // COLORES DE BORDES Y CONTORNOS
    //==========================================================================
    const Colour border = Colour(0xff555555);                // Border standard
    const Colour borderHighlight = accent.withAlpha(0.6f);   // Border con highlight
    
    //==========================================================================
    // FUNCIONES HELPER PARA GRADIENTES
    //==========================================================================
    
    // Gradiente vertical básico
    inline ColourGradient createVerticalGradient(const Rectangle<float>& bounds, 
                                                const Colour& top, 
                                                const Colour& bottom)
    {
        return ColourGradient(top, bounds.getTopLeft(), 
                            bottom, bounds.getBottomLeft(), false);
    }
    
    // Gradiente radial básico
    inline ColourGradient createRadialGradient(const Rectangle<float>& bounds, 
                                             const Colour& center, 
                                             const Colour& edge)
    {
        return ColourGradient(center, bounds.getCentre(), 
                            edge, bounds.getBottomRight(), true);
    }
    
    //==========================================================================
    // GRADIENTES ESPECIALIZADOS PARA MEDIDORES
    //==========================================================================
    
    // Gradiente principal para input meters
    inline ColourGradient createMeterGradient(const Rectangle<float>& bounds)
    {
        // Gradiente triple con colores personalizados para input meters principales
        const Colour bottomColor = Colour(0xFF3C0E40);  // Violeta oscuro abajo
        const Colour middleColor = Colour(0xFF4B1E53);  // Violeta medio
        const Colour topColor = Colour(0xFF6495ED);     // Azul OUTPUT para niveles altos
        
        auto gradient = ColourGradient(topColor, bounds.getTopLeft(),
                                     bottomColor, bounds.getBottomLeft(), false);
        gradient.addColour(0.5, middleColor);
        return gradient;
    }
    
    // Gradiente especializado para sidechain meters
    inline ColourGradient createSidechainMeterGradient(const Rectangle<float>& bounds)
    {
        // Gradiente especial para sidechain meters - vinculado al color del botón EXT KEY (teal)
        const Colour bottomColor = Colour(0xFF3C0E40);  // Mismo violeta oscuro abajo
        const Colour middleColor = Colour(0xFF4B1E53).interpolatedWith(Colour(0xFF0097A7), 0.25f);  // Violeta medio con toque teal
        const Colour topColor = Colour(0xFF6495ED).interpolatedWith(Colour(0xFF0097A7), 0.15f);     // Azul OUTPUT con toque teal
        
        auto gradient = ColourGradient(topColor, bounds.getTopLeft(),
                                     bottomColor, bounds.getBottomLeft(), false);
        gradient.addColour(0.5, middleColor);
        return gradient;
    }
    
    // Gradiente para output meters
    inline ColourGradient createOutputMeterGradient(const Rectangle<float>& bounds)
    {
        // Gradiente usando azul OUTPUT a azul profundo a púrpura personalizado (invertido)
        const Colour outputBlue = Colour(0xFF6495ED);     // Azul de OUTPUT
        const Colour darkPurple = Colour(0xFF202245);     // Morado oscuro personalizado
        const Colour deepBlue = Colour(0xFF0D3B52);       // Azul profundo intermedio
        
        auto gradient = ColourGradient(outputBlue, bounds.getTopLeft(),
                                     darkPurple, bounds.getBottomLeft(), false);
        gradient.addColour(0.25, outputBlue.interpolatedWith(deepBlue, 0.5f));
        gradient.addColour(0.5, deepBlue);
        gradient.addColour(0.75, deepBlue.interpolatedWith(darkPurple, 0.5f));
        return gradient;
    }
    
    // Gradiente para gain reduction meter
    inline ColourGradient createGainReductionGradient(const Rectangle<float>& bounds)
    {
        // Color blanco sólido para gain reduction meter (era #764CED violeta)
        auto solidColor = Colour(0xFFFFFFFF);  // Color blanco sólido sin gradiente
        auto gradient = ColourGradient(solidColor, bounds.getTopLeft(),
                                     solidColor, bounds.getBottomLeft(), false);
        return gradient;
    }
    
    // Gradiente azul para gain reduction meter (igual que output envelope)
    inline ColourGradient createBlueGainReductionGradient(const Rectangle<float>& bounds)
    {
        // Gradiente usando los mismos colores que output meters - azul de claro a oscuro
        const Colour outputBlue = Colour(0xFF6495ED);     // Azul de OUTPUT (arriba)
        const Colour darkPurple = Colour(0xFF202245);     // Morado oscuro personalizado (abajo)
        const Colour deepBlue = Colour(0xFF0D3B52);       // Azul profundo intermedio
        
        auto gradient = ColourGradient(outputBlue, bounds.getTopLeft(),
                                     darkPurple, bounds.getBottomLeft(), false);
        gradient.addColour(0.25, outputBlue.interpolatedWith(deepBlue, 0.5f));
        gradient.addColour(0.5, deepBlue);
        gradient.addColour(0.75, deepBlue.interpolatedWith(darkPurple, 0.5f));
        return gradient;
    }
    
    // Gradiente para gain reduction en modo DELTA
    inline ColourGradient createDeltaGainReductionGradient(const Rectangle<float>& bounds)
    {
        // Usar el mismo color verde que el histograma en modo DELTA
        auto deltaGreen = Colour(0xFF1DB954);  // Verde del histograma
        auto gradient = ColourGradient(deltaGreen, bounds.getTopLeft(),
                                     deltaGreen, bounds.getBottomLeft(), false);
        return gradient;
    }
    
    // Gradiente básico para controles
    inline ColourGradient createControlGradient(const Rectangle<float>& bounds)
    {
        return ColourGradient(backgroundLight.brighter(0.1f), bounds.getTopLeft(),
                            backgroundLight.darker(0.1f), bounds.getBottomLeft(), false);
    }
    
    //==========================================================================
    // GRADIENTES PARA MODOS ESPECIALES
    //==========================================================================
    
    // Gradientes para modo DELTA
    inline ColourGradient createDeltaMeterGradient(const Rectangle<float>& bounds)
    {
        auto gradient = ColourGradient(deltaClip, bounds.getTopLeft(),
                                     deltaLow, bounds.getBottomLeft(), false);
        gradient.addColour(0.3, deltaHigh);
        gradient.addColour(0.7, deltaMid);
        return gradient;
    }
    
    // Gradiente para output meters en modo DELTA
    inline ColourGradient createDeltaOutputMeterGradient(const Rectangle<float>& bounds)
    {
        // Gradiente invertido: cyan arriba, cyan más oscuro abajo
        auto gradient = ColourGradient(deltaLow, bounds.getTopLeft(),
                                     deltaClip, bounds.getBottomLeft(), false);
        gradient.addColour(0.3, deltaMid);
        gradient.addColour(0.7, deltaHigh);
        return gradient;
    }
    
    // Gradientes para modo SOLO SC
    inline ColourGradient createSoloScMeterGradient(const Rectangle<float>& bounds)
    {
        auto gradient = ColourGradient(soloScClip, bounds.getTopLeft(),
                                     soloScLow, bounds.getBottomLeft(), false);
        gradient.addColour(0.3, soloScHigh);
        gradient.addColour(0.7, soloScMid);
        return gradient;
    }
    
    // Gradiente para output meters en modo SOLO SC
    inline ColourGradient createSoloScOutputMeterGradient(const Rectangle<float>& bounds)
    {
        // Mismo gradiente que input meters para consistencia visual en modo Solo SC
        auto gradient = ColourGradient(soloScClip, bounds.getTopLeft(),
                                     soloScLow, bounds.getBottomLeft(), false);
        gradient.addColour(0.3, soloScHigh);
        gradient.addColour(0.7, soloScMid);
        return gradient;
    }
}