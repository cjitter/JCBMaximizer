# ==============================================================================
#
#  Copyright 2025 Juan Carlos Blancas
#
#  JCBMaximizer is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  JCBMaximizer is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with JCBMaximizer. If not, see <http://www.gnu.org/licenses/>.
#
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

project(JCBMaximizer VERSION 1.0.0)

# Configuración C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

# Versión mínima de macOS
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Versión mínima de macOS")

# Binary Universal (Apple Silicon + Intel)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Arquitecturas de compilación para macOS")

# Descargar JUCE directamente con FetchContent
include(FetchContent)

FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
)

# Opciones de JUCE para evitar compilar extras innecesarios
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(JUCE)

# Opciones de configuración
option(JUCE_BUILD_AAX "Build AAX format (requires AAX SDK and Pro Tools Developer)" ON)
option(JUCE_AUTO_INSTALL_PLUGINS "Auto-install plugins after build" ON)
option(JCB_DISABLE_SANITIZER "Disable runtime output sanitizer (only when debugging issues)" OFF)

# SDK externo opcional (AAX) para macOS
set(JUCE_AAX_SDK_PATH "$ENV{HOME}/SDK/aax-sdk-2-8-1" CACHE PATH "Ruta al SDK de AAX")

# Definir los formatos soportados según el estado del flag
if(JUCE_BUILD_AAX)
    set(PLUGIN_FORMATS VST3 AU AAX)
else()
    set(PLUGIN_FORMATS VST3 AU)
endif()

# Crear el plugin usando la lista explícita
juce_add_plugin(JCBMaximizer
        # Información básica del plugin
        PLUGIN_NAME "JCBMaximizer"
        PRODUCT_NAME "JCBMaximizer"
        VERSION 1.0.0
        DESCRIPTION "Stereo-linked maximizer with lookahead"

        # Información de la empresa
        COMPANY_NAME "Coeval"
        COMPANY_WEBSITE "https://github.com/cjitter"
        BUNDLE_ID "com.coeval.JCBMaximizer"
        PLUGIN_MANUFACTURER_CODE "CoeV"
        PLUGIN_CODE "CoE4"

        # Formatos soportados
        FORMATS ${PLUGIN_FORMATS}

        # Configuración del plugin
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD FALSE

        # Categorías por formato
        VST3_CATEGORIES Dynamics
        AU_MAIN_TYPE kAudioUnitType_Effect
        AU_SUBTYPE kAudioUnitSubType_DynamicsProcessor
        AAX_CATEGORY AAX_ePlugInCategory_Dynamics
)

# Archivos fuente del plugin
target_sources(JCBMaximizer PRIVATE
        # Core plugin files
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp

        # UI Components
        Source/Components/UI/TransferFunctionDisplay.cpp
        Source/Components/UI/CustomComboBox.cpp
        Source/Components/Windows/CreditsWindow.cpp
        Source/Components/Windows/CodeWindow.cpp
        
        # Gen~ DSP engine
        exported-code/JCBMaximizer.cpp
        exported-code/gen_dsp/genlib.cpp
        exported-code/gen_dsp/json.c
        exported-code/gen_dsp/json_builder.c
)

# Añadir recursos binarios
juce_add_binary_data(JCBMaximizer_BinaryData
        SOURCES
        Assets/fondo.png
        Assets/bypass.png
        Assets/delta.png
        Assets/diagramaFondo.png
        Assets/diagram.png
        # Code files
        Assets/code/InputTrim.txt
        Assets/code/GainCore.txt
        Assets/code/Detector.txt
        Assets/code/Output.txt
        # Presets de fábrica
        Assets/FactoryPresets/General_Day_Menu.preset
        # Presets José Peña
        Assets/FactoryPresets/Drums_Sn_Attack.preset
        Assets/FactoryPresets/Master_Limiter_Soft.preset
        Assets/FactoryPresets/Master_Limiter.preset
        Assets/FactoryPresets/Master_Clipper.preset
        Assets/FactoryPresets/Master_Limiter_2.preset
        Assets/FactoryPresets/Guitar_Max.preset
        Assets/FactoryPresets/Bass_Saturate_2.preset
        Assets/FactoryPresets/Bass_Saturate.preset
)

# Linkear binary data al plugin
target_link_libraries(JCBMaximizer PRIVATE
        JCBMaximizer_BinaryData
)

# Includes
target_include_directories(JCBMaximizer PRIVATE
        Source/
        exported-code/
        exported-code/gen_dsp/
)

if(JCB_DISABLE_SANITIZER)
    target_compile_definitions(JCBMaximizer PRIVATE JCB_DISABLE_SANITIZER=1)
endif()

# Módulos JUCE - Solo los necesarios (DSP manejado por Gen~)
target_link_libraries(JCBMaximizer PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
)

# Definiciones del compilador - Esenciales + optimizaciones
target_compile_definitions(JCBMaximizer PUBLIC
        # Configuraciones básicas estándar
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0

        # Configuraciones adicionales
        JUCE_REPORT_APP_USAGE=0

        # Optimización de rendimiento
        JUCE_STRICT_REFCOUNTEDPTR=1
)

# Flags del compilador para macOS (Clang)
target_compile_options(JCBMaximizer PRIVATE
    -Wall -Wextra -Wno-unused-parameter
)

# Instalación automática para macOS (se puede desactivar más arriba con JUCE_AUTO_INSTALL_PLUGINS OFF)
if(JUCE_AUTO_INSTALL_PLUGINS)
    # Rutas de instalación del sistema macOS
    set(VST3_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
    set(AU_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
    set(AAX_DIR "/Applications/Pro Tools Developer/Plug-Ins")

    # Instalación VST3 (carpeta de usuario)
    if(TARGET JCBMaximizer_VST3)
        add_custom_command(TARGET JCBMaximizer_VST3 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Installing VST3 to user directory (${VST3_DIR})..."
            COMMAND ${CMAKE_COMMAND} -E make_directory "${VST3_DIR}"
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${VST3_DIR}/JCBMaximizer.vst3"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_BINARY_DIR}/JCBMaximizer_artefacts/$<CONFIG>/VST3/JCBMaximizer.vst3"
                "${VST3_DIR}/JCBMaximizer.vst3"
            COMMENT "Installing VST3 plugin to ${VST3_DIR} (user folder)"
            VERBATIM
        )
    endif()

    # Instalación AU (carpeta de usuario)
    if(TARGET JCBMaximizer_AU)
        add_custom_command(TARGET JCBMaximizer_AU POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Installing AU to user directory (${AU_DIR})..."
            COMMAND ${CMAKE_COMMAND} -E make_directory "${AU_DIR}"
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${AU_DIR}/JCBMaximizer.component"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_BINARY_DIR}/JCBMaximizer_artefacts/$<CONFIG>/AU/JCBMaximizer.component"
                "${AU_DIR}/JCBMaximizer.component"
            COMMENT "Installing AU plugin to ${AU_DIR} (user folder)"
            VERBATIM
        )
    endif()

    # Instalación AAX
    if(TARGET JCBMaximizer_AAX)
        add_custom_command(TARGET JCBMaximizer_AAX POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Installing AAX to AAX_DIR..."
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${AAX_DIR}/JCBMaximizer.aaxplugin"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_BINARY_DIR}/JCBMaximizer_artefacts/$<CONFIG>/AAX/JCBMaximizer.aaxplugin"
                "${AAX_DIR}/JCBMaximizer.aaxplugin"
            COMMENT "Installing AAX plugin to ${AAX_DIR}..."
            VERBATIM
        )
    endif()
endif()

# Información de configuración
message(STATUS "JCBMaximizer ${PROJECT_VERSION} - Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build directory: ${CMAKE_CURRENT_BINARY_DIR}/JCBMaximizer_artefacts/")

# Estado del SDK AAX
if(JUCE_BUILD_AAX)
    if(EXISTS "${JUCE_AAX_SDK_PATH}")
        message(STATUS "AAX SDK: Found at ${JUCE_AAX_SDK_PATH}")
        message(STATUS "AAX: Will be built")
    else()
        message(WARNING "AAX SDK not found at ${JUCE_AAX_SDK_PATH} - AAX format disabled")
        message(STATUS "To build AAX, set JUCE_AAX_SDK_PATH to your AAX SDK location")
    endif()
else()
    message(STATUS "AAX: Disabled (use -DJUCE_BUILD_AAX=ON to enable)")
endif()

# Estado de auto-instalación
if(JUCE_AUTO_INSTALL_PLUGINS)
    message(STATUS "Auto-install: Enabled")
else()
    message(STATUS "Auto-install: Disabled")
endif()