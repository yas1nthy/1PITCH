#include "PluginProcessor.h"

namespace ParameterIDs
{
    static constexpr auto key             = "key";
    static constexpr auto scale           = "scale";
    static constexpr auto retuneSpeed     = "retuneSpeed";
    static constexpr auto humanize        = "humanize";
    static constexpr auto flexTune        = "flexTune";
    static constexpr auto correction      = "correction";
    static constexpr auto formant         = "formant";
    static constexpr auto transpose       = "transpose";
    static constexpr auto fineTune        = "fineTune";
    static constexpr auto mix             = "mix";
    static constexpr auto inputGain       = "inputGain";
    static constexpr auto outputGain      = "outputGain";
    static constexpr auto bypass          = "bypass";
    static constexpr auto mode            = "mode";
}

OnePitchAudioProcessor::OnePitchAudioProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "1PITCH_STATE", createParameterLayout())
{
    inputGainParameter  = parameters.getRawParameterValue (ParameterIDs::inputGain);
    outputGainParameter = parameters.getRawParameterValue (ParameterIDs::outputGain);
    bypassParameter     = parameters.getRawParameterValue (ParameterIDs::bypass);
}

juce::AudioProcessorValueTreeState::ParameterLayout OnePitchAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const juce::StringArray keys {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    const juce::StringArray scales {
        "Major", "Minor", "Chromatic", "Harmonic Minor", "Melodic Minor"
    };

    const juce::StringArray modes {
        "Natural", "Hard Tune"
    };

    layout.add (std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { ParameterIDs::key, 1 }, "Key", keys, 0));

    layout.add (std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { ParameterIDs::scale, 1 }, "Scale", scales, 1));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParameterIDs::retuneSpeed, 1 }, "Retune Speed",
        juce::NormalisableRange<float> (1.0f, 1000.0f, 0.1f, 0.35f), 20.0f, "ms"));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParameterIDs::humanize, 1 }, "Humanize",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 35.0f, "%"));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParameterIDs::flexTune, 1 }, "Flex Tune",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 25.0f, "%"));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParameterIDs::correction, 1 }, "Correction",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, "%"));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParameterIDs::formant, 1 }, "Formant",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f, "st"));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParameterIDs::transpose, 1 }, "Transpose",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 1.0f), 0.0f, "st"));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParameterIDs::fineTune, 1 }, "Fine Tune",
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f, "ct"));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParameterIDs::mix, 1 }, "Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, "%"));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParameterIDs::inputGain, 1 }, "Input Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f), 0.0f, "dB"));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParameterIDs::outputGain, 1 }, "Output Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f), 0.0f, "dB"));

    layout.add (std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { ParameterIDs::bypass, 1 }, "Bypass", false));

    layout.add (std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { ParameterIDs::mode, 1 }, "Mode", modes, 0));

    return layout;
}

void OnePitchAudioProcessor::prepareToPlay (double sampleRate, int)
{
    const auto smoothingTimeSeconds = 0.02;
    inputGainSmoother.reset (sampleRate, smoothingTimeSeconds);
    outputGainSmoother.reset (sampleRate, smoothingTimeSeconds);

    inputGainSmoother.setCurrentAndTargetValue (
        juce::Decibels::decibelsToGain (inputGainParameter->load()));

    outputGainSmoother.setCurrentAndTargetValue (
        juce::Decibels::decibelsToGain (outputGainParameter->load()));
}

void OnePitchAudioProcessor::releaseResources()
{
}

bool OnePitchAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainInput  = layouts.getMainInputChannelSet();
    const auto mainOutput = layouts.getMainOutputChannelSet();

    if (mainInput != mainOutput)
        return false;

    return mainOutput == juce::AudioChannelSet::mono()
        || mainOutput == juce::AudioChannelSet::stereo();
}

void OnePitchAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (bypassParameter->load() >= 0.5f)
        return;

    inputGainSmoother.setTargetValue (
        juce::Decibels::decibelsToGain (inputGainParameter->load()));

    outputGainSmoother.setTargetValue (
        juce::Decibels::decibelsToGain (outputGainParameter->load()));

    const auto channels = getTotalNumInputChannels();
    const auto samples = buffer.getNumSamples();

    for (int sample = 0; sample < samples; ++sample)
    {
        // Advance the smoothers once per frame so every channel receives
        // exactly the same gain trajectory.
        const auto inputGain = inputGainSmoother.getNextValue();
        const auto outputGain = outputGainSmoother.getNextValue();
        const auto totalGain = inputGain * outputGain;

        for (int channel = 0; channel < channels; ++channel)
            buffer.getWritePointer (channel)[sample] *= totalGain;
    }

    for (int channel = channels; channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear (channel, 0, samples);
}

juce::AudioProcessorEditor* OnePitchAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

bool OnePitchAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String OnePitchAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OnePitchAudioProcessor::acceptsMidi() const { return false; }
bool OnePitchAudioProcessor::producesMidi() const { return false; }
bool OnePitchAudioProcessor::isMidiEffect() const { return false; }
double OnePitchAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int OnePitchAudioProcessor::getNumPrograms() { return 1; }
int OnePitchAudioProcessor::getCurrentProgram() { return 0; }
void OnePitchAudioProcessor::setCurrentProgram (int) {}
const juce::String OnePitchAudioProcessor::getProgramName (int) { return {}; }
void OnePitchAudioProcessor::changeProgramName (int, const juce::String&) {}

void OnePitchAudioProcessor::getStateInformation (juce::MemoryBlock& destinationData)
{
    const auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());

    if (xml != nullptr)
        copyXmlToBinary (*xml, destinationData);
}

void OnePitchAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OnePitchAudioProcessor();
}
