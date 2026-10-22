cmake_minimum_required(VERSION 3.22)
project(NOCTIVORE_FX VERSION 12.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use local JUCE: cmake -S . -B build -DJUCE_DIR=/path/to/JUCE
if(DEFINED JUCE_DIR)
 add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/juce" EXCLUDE_FROM_ALL)
else()
 include(FetchContent)
 # Set long-path handling on the JUCE clone itself, regardless of the user's
 # global Git configuration. This works with Visual Studio 2022 and 2026.
 FetchContent_Declare(juce
   GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
   GIT_TAG 8.0.15
   GIT_SHALLOW TRUE
   GIT_CONFIG core.longpaths=true
   GIT_PROGRESS TRUE
 )
 FetchContent_MakeAvailable(juce)
endif()
juce_add_plugin(NOCTIVORE_FX
 COMPANY_NAME "Noctivore Audio"
 BUNDLE_ID "audio.voidcore.spectral"
 PLUGIN_MANUFACTURER_CODE NcFx
 PLUGIN_CODE Vc11
 PRODUCT_NAME "VOIDCORE"
 FORMATS VST3
 IS_SYNTH TRUE
 NEEDS_MIDI_INPUT TRUE
 NEEDS_MIDI_OUTPUT FALSE
 IS_MIDI_EFFECT FALSE
 EDITOR_WANTS_KEYBOARD_FOCUS FALSE
 VST3_CATEGORIES Instrument Synth
 COPY_PLUGIN_AFTER_BUILD FALSE)
# Safe fallback if a specific host rejects the custom editor.
# Set -DNOCTIVORE_GENERIC_EDITOR=ON or environment NOCTIVORE_SAFE_GUI=1.
option(NOCTIVORE_GENERIC_EDITOR "Use JUCE generic parameter editor" OFF)
juce_generate_juce_header(NOCTIVORE_FX)
target_sources(NOCTIVORE_FX PRIVATE
 Source/PluginProcessor.h Source/PluginProcessor.cpp
 Source/RomplerEngine.h Source/PsySpace.h Source/BankCatalog.h)
# Keep the fallback build genuinely independent of custom editor compilation.
if(NOT NOCTIVORE_GENERIC_EDITOR)
 target_sources(NOCTIVORE_FX PRIVATE
  Source/VoidcoreEditor.h Source/VoidcoreEditor.cpp)
endif()
target_compile_definitions(NOCTIVORE_FX PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
 PRIVATE NOCTIVORE_GENERIC_EDITOR=$<BOOL:${NOCTIVORE_GENERIC_EDITOR}>)
if(MSVC)
  target_compile_options(NOCTIVORE_FX PRIVATE /utf-8 /bigobj)
endif()
target_link_libraries(NOCTIVORE_FX PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC
 juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Every VST3 build contains its factory audio in the correct module-relative location.
# On Windows: <product>.vst3/Contents/Resources/Banks/*.wav
add_custom_command(TARGET NOCTIVORE_FX_VST3 POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:NOCTIVORE_FX_VST3>/../Resources/Banks"
 COMMAND ${CMAKE_COMMAND} -E copy_directory
         "${CMAKE_CURRENT_SOURCE_DIR}/Banks"
         "$<TARGET_FILE_DIR:NOCTIVORE_FX_VST3>/../Resources/Banks"
 COMMENT "Bundling 200 VOIDCORE V12 XENO 48k PCM16 WAV multisamples")

