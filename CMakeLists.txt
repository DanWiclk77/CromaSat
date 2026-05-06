cmake_minimum_required(VERSION 3.22)

project(CromaSat VERSION 1.2.0)

# 1. Download/Include JUCE
set(JUCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/JUCE CACHE PATH "Path to JUCE")
add_subdirectory(${JUCE_DIRECTORY} JUCE)

# 2. Define the Plugin
juce_add_plugin(CromaSat
    COMPANY_NAME "AIStudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_STEP TRUE
    PLUGIN_MANUFACTURER_CODE "AISp"
    PLUGIN_CODE "CSat"
    FORMATS VST3 AU
    PRODUCT_NAME "Croma Sat")

# 3. Source Files
target_sources(CromaSat PRIVATE
    vst/Source/PluginProcessor.cpp
    vst/Source/PluginProcessor.h
    vst/Source/PluginEditor.cpp
    vst/Source/PluginEditor.h)

# 4. Include Folders & Libraries
target_compile_definitions(CromaSat PRIVATE
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0)

target_link_libraries(CromaSat PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_graphics)

# 5. Asset injection (The Compiled React App)
# We pack only the html for the base loading
juce_add_binary_data(CromaData SOURCES
    dist/index.html)

target_link_libraries(CromaSat PRIVATE CromaData)
