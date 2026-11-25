# 1PITCH — VST3 project

This repository builds the Windows x64 VST3 plugin using GitHub Actions.
You do not need to install JUCE or Visual Studio on your own computer.

## Build on GitHub

1. Create a new GitHub repository named `1PITCH`.
2. Upload all files and folders from this project, including the hidden `.github` folder.
3. Open the repository's **Actions** tab.
4. Select **Build 1PITCH VST3**.
5. Press **Run workflow** (or push a commit to `main`).
6. Wait for the workflow to finish successfully.
7. Open the completed workflow run and download the artifact named
   `1PITCH-Windows-x64-VST3`.

## Install

Extract the downloaded artifact. Copy `1PITCH.vst3` into:

`C:\Program Files\Common Files\VST3\`

If the artifact contains a folder named `1PITCH.vst3`, copy that entire folder.
Restart your DAW and rescan VST3 plugins.

## Current implementation stage

This is the buildable project foundation. It currently provides:
- JUCE VST3 plugin target and GitHub Actions build.
- Mono/stereo bus support.
- APVTS parameters with host automation and state save/restore.
- Input and output gain with smoothing.
- Generic JUCE parameter editor.

The pitch-correction DSP engine (pitch detection, note tracking, pitch shifting
and formant processing) is not implemented in this foundation stage yet. Those
modules must be added and tested before this can be considered a functioning
Auto-Tune product.
