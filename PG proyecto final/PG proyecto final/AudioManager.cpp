#include "AudioManager.h"
#include <string>

AudioManager::AudioManager()
    : backgroundMusic(nullptr), footstepLoop(nullptr), musicVolume(0.4f), effectsVolume(0.6f)
{
    engine = irrklang::createIrrKlangDevice();
}

AudioManager::~AudioManager() {
    if (backgroundMusic) {
        backgroundMusic->stop();
        backgroundMusic->drop();
    }

    if (footstepLoop) {
        footstepLoop->stop();
        footstepLoop->drop();
    }

    if (engine)
        engine->drop();
}


void AudioManager::playFootstepLoop() {
    if (!engine) return;

    if (!footstepLoop) {
        footstepLoop = engine->play2D("Sound/WoodStep.mp3", true, false, true); // true = loop
        if (footstepLoop)
            footstepLoop->setVolume(effectsVolume);
    }
}

void AudioManager::stopFootstepLoop() {
    if (footstepLoop) {
        footstepLoop->stop();
        footstepLoop->drop();
        footstepLoop = nullptr;
    }
}


//void AudioManager::playBackgroundMusic() {
//    if (!engine) return;
//
//    if (!backgroundMusic) {
//        backgroundMusic = engine->play2D("Sound/MenuSound.mp3", true, false, true);
//        if (backgroundMusic)
//            backgroundMusic->setVolume(musicVolume);
//    }
//}

void AudioManager::playBackgroundMusic(const std::string& filepath, float volume) {
    if (!engine) return;

    // Si ya hay una canción reproduciéndose, la paramos primero
    if (backgroundMusic) {
        backgroundMusic->stop();
        backgroundMusic->drop(); // Libera memoria
        backgroundMusic = nullptr;
    }

    // Cargar la nueva canción
    backgroundMusic = engine->play2D(filepath.c_str(), true, false, true);
    if (backgroundMusic) {
        backgroundMusic->setVolume(volume);
    }
}


void AudioManager::stopBackgroundMusic() {
    if (backgroundMusic) {
        backgroundMusic->stop();
        backgroundMusic->drop();
        backgroundMusic = nullptr;
    }
}

void AudioManager::playClickSound() {
    if (!engine) return;

    irrklang::ISound* clickSound = engine->play2D("Sound/click.mp3", false, false, true);
    if (clickSound)
        clickSound->setVolume(effectsVolume);
}

void AudioManager::setMusicVolume(float volume) {
    musicVolume = volume;
    if (backgroundMusic)
        backgroundMusic->setVolume(musicVolume);
}

void AudioManager::setEffectsVolume(float volume) {
    effectsVolume = volume;
}
