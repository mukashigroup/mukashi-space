juce_add_plugin(MUKASHI_SOURCE VERSION "1.0.0" COMPANY_NAME "MUKASHI" PLUGIN_MANUFACTURERS_CODE Muka PLUGIN_CODE Msrc FORMATS VST3 PRODUCT_NAME "MUKASHI Space SOURCE")
target_sources(MUKASHI_SOURCE PRIVATE SpaceProcessor.cpp)
target_link_libraries(MUKASHI_SOURCE PRIVATE juce::juce_audio_utils juce::juce_dsp)
