# ==============================================================================
#
#  Copyright 2025 Juan Carlos Blancas
#
#  JCBTransient is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  JCBTransient is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with JCBTransient. If not, see <http://www.gnu.org/licenses/>.
#
# ==============================================================================

# Pluginval.cmake - Módulo para testing automático de plugins con pluginval

include(FetchContent)

# Opción para deshabilitar la descarga automática
option(PLUGINVAL_AUTO_DOWNLOAD "Automatically download pluginval if not found" ON)

# Directorio donde se guardará pluginval
set(PLUGINVAL_TOOLS_DIR "${CMAKE_SOURCE_DIR}/tools")

# Primero intentar encontrar pluginval en el sistema
find_program(PLUGINVAL_EXECUTABLE
    NAMES pluginval
    PATHS
        /Applications/pluginval.app/Contents/MacOS
        /usr/local/bin
        /opt/homebrew/bin
        ${PLUGINVAL_TOOLS_DIR}
    DOC "Path to pluginval executable"
)

# Si no se encuentra y la descarga automática está habilitada
if(NOT PLUGINVAL_EXECUTABLE AND PLUGINVAL_AUTO_DOWNLOAD)
    message(STATUS "pluginval not found in system. Attempting to download...")
    
    # Crear directorio tools si no existe
    file(MAKE_DIRECTORY ${PLUGINVAL_TOOLS_DIR})
    
    # Determinar la plataforma y arquitectura
    if(APPLE)
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
            set(PLUGINVAL_URL "https://github.com/Tracktion/pluginval/releases/download/v1.0.3/pluginval_macOS_arm64.zip")
            set(PLUGINVAL_FILENAME "pluginval_macOS_arm64.zip")
        else()
            set(PLUGINVAL_URL "https://github.com/Tracktion/pluginval/releases/download/v1.0.3/pluginval_macOS.zip")
            set(PLUGINVAL_FILENAME "pluginval_macOS.zip")
        endif()
    elseif(WIN32)
        set(PLUGINVAL_URL "https://github.com/Tracktion/pluginval/releases/download/v1.0.3/pluginval_Windows.zip")
        set(PLUGINVAL_FILENAME "pluginval_Windows.zip")
    elseif(UNIX AND NOT APPLE)
        set(PLUGINVAL_URL "https://github.com/Tracktion/pluginval/releases/download/v1.0.3/pluginval_Linux.zip")
        set(PLUGINVAL_FILENAME "pluginval_Linux.zip")
    endif()
    
    if(PLUGINVAL_URL)
        set(PLUGINVAL_DOWNLOAD_PATH "${CMAKE_BINARY_DIR}/${PLUGINVAL_FILENAME}")
        
        # Descargar pluginval
        message(STATUS "Downloading pluginval from ${PLUGINVAL_URL}")
        file(DOWNLOAD 
            ${PLUGINVAL_URL}
            ${PLUGINVAL_DOWNLOAD_PATH}
            STATUS DOWNLOAD_STATUS
            SHOW_PROGRESS
        )
        
        list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
        if(DOWNLOAD_RESULT EQUAL 0)
            # Extraer el archivo
            message(STATUS "Extracting pluginval...")
            
            if(APPLE)
                # En macOS, el zip contiene pluginval.app
                execute_process(
                    COMMAND unzip -q -o ${PLUGINVAL_DOWNLOAD_PATH} -d ${CMAKE_BINARY_DIR}/pluginval_temp
                    RESULT_VARIABLE UNZIP_RESULT
                )
                
                if(UNZIP_RESULT EQUAL 0)
                    # Buscar el ejecutable dentro del .app
                    file(GLOB PLUGINVAL_APP "${CMAKE_BINARY_DIR}/pluginval_temp/pluginval.app")
                    if(EXISTS "${PLUGINVAL_APP}/Contents/MacOS/pluginval")
                        file(COPY "${PLUGINVAL_APP}/Contents/MacOS/pluginval" 
                             DESTINATION ${PLUGINVAL_TOOLS_DIR}
                             FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
                        set(PLUGINVAL_EXECUTABLE "${PLUGINVAL_TOOLS_DIR}/pluginval")
                        message(STATUS "pluginval successfully downloaded and installed to ${PLUGINVAL_EXECUTABLE}")
                    endif()
                endif()
            else()
                # En otras plataformas, extraer directamente
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E tar xzf ${PLUGINVAL_DOWNLOAD_PATH}
                    WORKING_DIRECTORY ${PLUGINVAL_TOOLS_DIR}
                    RESULT_VARIABLE UNZIP_RESULT
                )
                
                if(UNZIP_RESULT EQUAL 0)
                    find_program(PLUGINVAL_EXECUTABLE
                        NAMES pluginval pluginval.exe
                        PATHS ${PLUGINVAL_TOOLS_DIR}
                        NO_DEFAULT_PATH
                    )
                endif()
            endif()
            
            # Limpiar archivos temporales
            file(REMOVE ${PLUGINVAL_DOWNLOAD_PATH})
            if(EXISTS "${CMAKE_BINARY_DIR}/pluginval_temp")
                file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/pluginval_temp")
            endif()
        else()
            message(WARNING "Failed to download pluginval from ${PLUGINVAL_URL}")
        endif()
    endif()
endif()

# Verificar si finalmente tenemos pluginval
if(PLUGINVAL_EXECUTABLE)
    message(STATUS "Found pluginval: ${PLUGINVAL_EXECUTABLE}")
else()
    if(PLUGINVAL_AUTO_DOWNLOAD)
        message(WARNING "pluginval could not be downloaded automatically.")
    endif()
    message(WARNING "pluginval not found. Plugin validation tests will be skipped.")
    message(STATUS "You can manually download pluginval from: https://github.com/Tracktion/pluginval/releases")
    message(STATUS "Or disable auto-download with: -DPLUGINVAL_AUTO_DOWNLOAD=OFF")
endif()

# Función para agregar tests de pluginval a un target
function(add_pluginval_tests target)
    if(NOT PLUGINVAL_EXECUTABLE)
        return()
    endif()

    # Obtener el formato del plugin desde el nombre del target
    string(REGEX MATCH "_([A-Z0-9]+)$" format_match ${target})
    if(format_match)
        set(plugin_format ${CMAKE_MATCH_1})
    else()
        message(WARNING "Cannot determine plugin format from target name: ${target}")
        return()
    endif()

    # Verificar que el formato sea compatible con pluginval
    if(plugin_format STREQUAL "AAX")
        message(STATUS "Skipping pluginval tests for ${target}: AAX format not supported by pluginval")
        return()
    endif()

    # Definir la ruta del plugin según el formato
    if(plugin_format STREQUAL "VST3")
        set(plugin_path "${CMAKE_CURRENT_BINARY_DIR}/JCBTransient_artefacts/$<CONFIG>/VST3/JCBTransient.vst3")
    elseif(plugin_format STREQUAL "AU")
        set(plugin_path "${CMAKE_CURRENT_BINARY_DIR}/JCBTransient_artefacts/$<CONFIG>/AU/JCBTransient.component")
    else()
        message(WARNING "Unsupported plugin format for pluginval: ${plugin_format}")
        return()
    endif()

    # Crear directorio temporal para logs
    set(PLUGINVAL_TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/pluginval_temp")
    
    # Agregar test básico de validación
    add_test(
        NAME pluginval_${target}_basic
        COMMAND ${PLUGINVAL_EXECUTABLE}
            --validate-in-process
            --output-dir ${PLUGINVAL_TEMP_DIR}
            --timeout-ms 30000
            ${plugin_path}
    )

    # Configurar propiedades del test
    set_tests_properties(pluginval_${target}_basic PROPERTIES
        TIMEOUT 60
        LABELS "pluginval;${plugin_format}"
    )

    # El test depende de que el plugin esté construido
    set_tests_properties(pluginval_${target}_basic PROPERTIES
        FIXTURES_REQUIRED ${target}_built
    )

    # Agregar test extendido (más riguroso pero más lento)
    add_test(
        NAME pluginval_${target}_extended
        COMMAND ${PLUGINVAL_EXECUTABLE}
            --validate-in-process
            --randomise
            --timeout-ms 60000
            --repeat 3
            --output-dir ${PLUGINVAL_TEMP_DIR}
            ${plugin_path}
    )

    set_tests_properties(pluginval_${target}_extended PROPERTIES
        TIMEOUT 300
        LABELS "pluginval;${plugin_format};extended"
    )

    set_tests_properties(pluginval_${target}_extended PROPERTIES
        FIXTURES_REQUIRED ${target}_built
    )

    # Agregar test de limpieza que se ejecuta después
    add_test(
        NAME cleanup_pluginval_${target}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${PLUGINVAL_TEMP_DIR}
    )
    
    # Configurar el test de limpieza para que se ejecute después de los tests
    set_tests_properties(cleanup_pluginval_${target} PROPERTIES
        FIXTURES_CLEANUP ${target}_pluginval_cleanup
        LABELS "cleanup"
    )
    
    # Hacer que los tests de pluginval usen este fixture
    set_tests_properties(pluginval_${target}_basic pluginval_${target}_extended PROPERTIES
        FIXTURES_REQUIRED "${target}_built;${target}_pluginval_cleanup"
    )
    
    message(STATUS "Added pluginval tests for ${target} (${plugin_format})")
endfunction()

# Función para crear fixture que asegura que el plugin esté construido antes del test
function(add_pluginval_build_fixture target)
    add_test(
        NAME setup_${target}
        COMMAND ${CMAKE_COMMAND} --build . --target ${target} --config $<CONFIG>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    set_tests_properties(setup_${target} PROPERTIES
        FIXTURES_SETUP ${target}_built
        LABELS "build"
    )
endfunction()
