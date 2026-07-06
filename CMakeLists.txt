cmake_minimum_required(VERSION 3.22)

project(BoratoLA2A VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BORATO_FETCH_JUCE "Download JUCE when no local/package installation is available" OFF)
option(BORATO_COPY_PLUGIN_AFTER_BUILD "Copy the plugin after building" OFF)
option(BORATO_USE_OPENGL_RENDERER "Attach a JUCE OpenGL renderer to the editor" OFF)
set(BORATO_JUCE_SOURCE_DIR "" CACHE PATH "Path to a JUCE source checkout")

if(NOT BORATO_JUCE_SOURCE_DIR AND EXISTS "C:/JUCE/CMakeLists.txt")
    set(BORATO_JUCE_SOURCE_DIR "C:/JUCE")
endif()

if(BORATO_JUCE_SOURCE_DIR AND EXISTS "${BORATO_JUCE_SOURCE_DIR}/CMakeLists.txt")
    set(JUCE_BUILD_HELPER_TOOLS OFF CACHE BOOL "Bootstrap JUCE helper tools at configure time" FORCE)
    add_subdirectory("${BORATO_JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/juce" EXCLUDE_FROM_ALL)
else()
    find_package(JUCE CONFIG QUIET)
    if(NOT JUCE_FOUND)
        if(NOT BORATO_FETCH_JUCE)
            message(FATAL_ERROR "JUCE not found. Set BORATO_JUCE_SOURCE_DIR/JUCE_DIR or enable BORATO_FETCH_JUCE.")
        endif()
        include(FetchContent)
        FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
                                  GIT_TAG 8.0.12 GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(juce)
    endif()
endif()

# Work around the JUCE 8 resource helper target being emitted literally by
# recent Visual Studio generators, matching the other Borato CMake projects.
if(WIN32)
    function(_juce_add_resources_rc source_target dest_target)
        if(NOT TARGET ${source_target}_rc_lib)
            get_target_property(juce_library_code ${source_target} JUCE_GENERATED_SOURCES_DIRECTORY)
            get_target_property(input_info_file ${source_target} JUCE_INFO_FILE)
            get_target_property(generated_icon ${source_target} JUCE_ICON_FILE)
            set(dependency)
            if(generated_icon)
                set(dependency DEPENDS "${generated_icon}")
            endif()
            set(resource_rc_file "${juce_library_code}/${source_target}_resources.rc")
            add_custom_command(OUTPUT "${resource_rc_file}"
                COMMAND "$<TARGET_FILE:juce::juceaide>" rcfile "${input_info_file}" "${resource_rc_file}"
                ${dependency} VERBATIM)
            add_library(${source_target}_rc_lib OBJECT "${resource_rc_file}")
        endif()
        target_link_libraries(${dest_target} PRIVATE ${source_target}_rc_lib)
    endfunction()
endif()

juce_add_plugin(BoratoLA2A
    COMPANY_NAME "Borato Company"
    BUNDLE_ID com.boratocompany.la2a
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${BORATO_COPY_PLUGIN_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE Bora
    PLUGIN_CODE BLA2
    FORMATS VST3 Standalone
    PRODUCT_NAME "Borato LA-2A")

juce_generate_juce_header(BoratoLA2A)

target_sources(BoratoLA2A PRIVATE
    Source/PluginProcessor.cpp Source/PluginProcessor.h
    Source/PluginEditor.cpp Source/PluginEditor.h
    Source/DSP/OptoCompressor.cpp Source/DSP/OptoCompressor.h
    Source/DSP/T4OpticalCellModel.cpp Source/DSP/T4OpticalCellModel.h
    Source/DSP/SidechainPreEmphasis.cpp Source/DSP/SidechainPreEmphasis.h
    Source/DSP/AnalogStages.cpp Source/DSP/AnalogStages.h
    Source/ui/GraphicsHelpers.cpp Source/ui/GraphicsHelpers.h
    Source/ui/RackScrew.cpp Source/ui/RackScrew.h
    Source/ui/VuMeterComponent.cpp Source/ui/VuMeterComponent.h
    Source/ui/VintageKnobComponent.cpp Source/ui/VintageKnobComponent.h
    Source/ui/ToggleSwitchComponent.cpp Source/ui/ToggleSwitchComponent.h
    Source/ui/JewelLightComponent.cpp Source/ui/JewelLightComponent.h
    Source/ui/R37TrimComponent.cpp Source/ui/R37TrimComponent.h
    Source/ui/La2aPanelComponent.cpp Source/ui/La2aPanelComponent.h)

target_include_directories(BoratoLA2A PRIVATE Source)
target_link_libraries(BoratoLA2A PRIVATE juce::juce_audio_utils juce::juce_dsp)
if(BORATO_USE_OPENGL_RENDERER)
    target_link_libraries(BoratoLA2A PRIVATE juce::juce_opengl)
endif()
target_link_libraries(BoratoLA2A PUBLIC
    juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

target_compile_definitions(BoratoLA2A PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    BORATO_USE_OPENGL_RENDERER=$<BOOL:${BORATO_USE_OPENGL_RENDERER}>)

if(MSVC)
    target_compile_options(BoratoLA2A PRIVATE /W4 /permissive- /wd4244 /wd4267)
else()
    target_compile_options(BoratoLA2A PRIVATE -Wall -Wextra -Wpedantic)
endif()
