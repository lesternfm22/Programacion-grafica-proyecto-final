#include "AudioManager.h"
#include <string>

AudioManager::AudioManager()
    : backgroundMusic(nullptr), footstepLoop(nullptr), musicVolume(0.4f), effectsVolume(0.6f)
{
    // Initialize the audio engine
    engine = irrklang::createIrrKlangDevice();
}

AudioManager::~AudioManager() {
    // Clean up background music
    if (backgroundMusic) {
        backgroundMusic->stop();
        backgroundMusic->drop();
    }

    // Clean up footstep sound
    if (footstepLoop) {
        footstepLoop->stop();
        footstepLoop->drop();
    }

    // Clean up audio engine
    if (engine)
        engine->drop();
}

// Plays the footstep sound effect in a loop
void AudioManager::playFootstepLoop() {
    if (!engine) return;

    // Only create new instance if not already playing
    if (!footstepLoop) {
        footstepLoop = engine->play2D("Sound/WoodStep.mp3", true, false, true); // true = loop
        if (footstepLoop)
            footstepLoop->setVolume(effectsVolume);
    }
}

// Stops the looping footstep sound effect
void AudioManager::stopFootstepLoop() {
    if (footstepLoop) {
        footstepLoop->stop();
        footstepLoop->drop();
        footstepLoop = nullptr;
    }
}

// Plays background music from specified file path
void AudioManager::playBackgroundMusic(const std::string& filepath, float volume) {
    if (!engine) return;

    // Stop current music if already playing
    if (backgroundMusic) {
        backgroundMusic->stop();
        backgroundMusic->drop(); // Free memory
        backgroundMusic = nullptr;
    }

    // Load and play new music track
    backgroundMusic = engine->play2D(filepath.c_str(), true, false, true);
    if (backgroundMusic) {
        backgroundMusic->setVolume(volume);
    }
}

// Stops currently playing background music
void AudioManager::stopBackgroundMusic() {
    if (backgroundMusic) {
        backgroundMusic->stop();
        backgroundMusic->drop();
        backgroundMusic = nullptr;
    }
}

// Sets volume for background music
void AudioManager::setMusicVolume(float volume) {
    musicVolume = volume;
    if (backgroundMusic)
        backgroundMusic->setVolume(musicVolume);
}

// Sets volume for sound effects
void AudioManager::setEffectsVolume(float volume) {
    effectsVolume = volume;
}