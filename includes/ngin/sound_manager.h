#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <irrklang/irrKlang.h>
using namespace irrklang;

#include <ngin/fileutils.h>
#include <ngin/log.h>
#include <unordered_map>
#include <atomic>

class SoundManager
{
public:
    SoundManager() {
        if (!engine) {
            engine = createIrrKlangDevice();
        }
    }

    ~SoundManager() {
        for (auto& sound : sounds) {
            if (sound.second) {
                sound.second->stop();
                sound.second->drop();
            }
        }
        if (engine) {
            engine->drop();
            engine = nullptr;
        }
    }

    static void setSound(float soundVal) {
        engine->setSoundVolume(soundVal); // Adjust global volume
    }

    static unsigned int play(const std::string& fileName, float volume = 1.0,
                             bool playLooped = false, bool startPaused = false) {
        Log::console("Sound Manager: Playing " + fileName);
        if (engine) {
            ISound* sound = engine->play2D(FileUtils::getResourcePath("audio/" + fileName + ".mp3").c_str(), playLooped, startPaused, true);
            if (sound) {
                sound->setVolume(volume);
                unsigned int id = nextSoundId++;
                sounds[id] = sound;
                return id;  // Return the unique sound ID
            }
        } else {
            Log::console("Error: Sound engine not initialized!");
        }
        return 0;  // Return 0 if sound could not be played
    }
    static void playInstance(const std::string& fileName, float volume = 1.0,
        bool startPaused = false) {
        Log::console("sound manager play song! " + fileName);
        if (engine) {
            ISound* sound1 = engine->play2D(FileUtils::getResourcePath("audio/" + fileName + ".mp3").c_str(), false, startPaused, true);
            if (sound1) {
                sound1->setVolume(volume); // Set volume to 50%

                sound1->drop();
            }
        } else {
            Log::console("Error: Sound engine not initialized!");
        }
    }

    static void stopSound(unsigned int soundId) {
        if (sounds.find(soundId) != sounds.end() && sounds[soundId]) {
            sounds[soundId]->stop();
            sounds[soundId]->drop();  // Properly manage memory
            sounds.erase(soundId);  // Remove from map
        }
    }

    static bool isPlaying(unsigned int soundId) {
        auto it = sounds.find(soundId);
        if (it != sounds.end() && it->second) {
            return !it->second->isFinished();
        }
        return false; // Sound is not playing or does not exist
    }

private:
    static ISoundEngine* engine;
    static std::unordered_map<unsigned int, ISound*> sounds;  // Map to track sounds
    static std::atomic<unsigned int> nextSoundId;  // Atomic to ensure thread safety
};

#endif
