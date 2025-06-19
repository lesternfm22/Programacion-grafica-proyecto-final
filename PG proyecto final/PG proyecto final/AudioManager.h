#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <irrKlang/irrKlang.h>
#include <string>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    //void playBackgroundMusic(); 
    void playBackgroundMusic(const std::string& filepath, float volume);

    irrklang::ISound* getBackgroundMusic() const { return backgroundMusic; }


    void stopBackgroundMusic();   
    void playClickSound();        

    void setMusicVolume(float volume);   // 0.0 a 1.0
    void setEffectsVolume(float volume); // 0.0 a 1.0

    //void playFootstepSound();
    void playFootstepLoop();
    void stopFootstepLoop();

private:
    irrklang::ISoundEngine* engine;
    irrklang::ISound* backgroundMusic;
    irrklang::ISound* footstepLoop;
    float musicVolume;
    float effectsVolume;
};

#endif
