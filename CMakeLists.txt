cmake_minimum_required(VERSION 3.22)
project(WarlordSynth VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(COPY_AFTER_BUILD "Copier le VST3 dans le dossier systeme apres compilation" ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(WarlordSynth
    COMPANY_NAME "MonStudio"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Mstd
    PLUGIN_CODE Wsyn
    FORMATS VST3 Standalone
    VST3_CATEGORIES Instrument Synth
    PRODUCT_NAME "Warlord Synth"
    COPY_PLUGIN_AFTER_BUILD ${COPY_AFTER_BUILD})

target_sources(WarlordSynth PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp)

target_compile_definitions(WarlordSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(WarlordSynth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
