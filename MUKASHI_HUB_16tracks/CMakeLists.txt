juce_add_plugin(MUKASHI_HUB VERSION "1.0.0" COMPANY_NAME "MUKASHI" PLUGIN_MANUFACTURERS_CODE Muka PLUGIN_CODE Mhub FORMATS VST3 PRODUCT_NAME "MUKASHI Space HUB (16)")
target_sources(MUKASHI_HUB PRIVATE HubProcessor.cpp)
target_link_libraries(MUKASHI_HUB PRIVATE juce::juce_audio_utils juce::juce_dsp)
