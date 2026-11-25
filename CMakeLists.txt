cmake_minimum_required(VERSION 3.22)

project(OnePitch VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

juce_add_plugin(OnePitch
    COMPANY_NAME "1PITCH"
    BUNDLE_ID "com.onepitch.audio"
    PLUGIN_MANUFACTURER_CODE OnPt
    PLUGIN_CODE OnP1
    PRODUCT_NAME "1PITCH"
    DESCRIPTION "1PITCH Vocal Pitch Processor"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS VST3
)

target_sources(OnePitch
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
)

target_compile_features(OnePitch PRIVATE cxx_std_17)

target_compile_definitions(OnePitch
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(OnePitch
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
