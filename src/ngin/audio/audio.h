#ifndef AUDIO_H
#define AUDIO_H

#include <iostream>
#include <fstream>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include <cstdint>
#include <glm/vec3.hpp> // Include glm for 3D vector support
#include <thread>
#include <atomic>

#include <ngin/resources.h>

struct AudioSound {
    std::string name;
    glm::vec3 source;
    glm::vec3 look;
    glm::vec3 listener;
    bool isPlaying;
};

class Audio {
public:
    static inline ALCdevice* device; // Now a static member
    static inline ALCcontext* context; // Now a static member

    static bool create() {
        device = alcOpenDevice(nullptr); // Open the default device
        if (!device) {
            std::cerr << "Failed to open OpenAL device" << std::endl;
            return false;
        }

        context = alcCreateContext(device, nullptr); // Create a context
        if (!context) {
            std::cerr << "Failed to create OpenAL context" << std::endl;
            alcCloseDevice(device);
            return false;
        }

        if (!alcMakeContextCurrent(context)) { // Make the context current
            std::cerr << "Failed to make OpenAL context current" << std::endl;
            alcDestroyContext(context);
            alcCloseDevice(device);
            return false;
        }
        

        std::cout << "OpenAL initialized successfully!" << std::endl;
        return true;
    }

    static void destroy() {
        alcMakeContextCurrent(nullptr);
        if (context) alcDestroyContext(context);
        if (device) alcCloseDevice(device);
    }

    static bool play(const std::string& name, const glm::vec3& sourcePosition, const glm::vec3& listenerPosition) {
        AudioData& data = Resources::getAudioData(name);
        data.play(sourcePosition, listenerPosition);
        return true;
    }

    static void update() {
        if (!sounds_.empty()) {
            AudioSound& sound = sounds_.front();
            if (!sound.isPlaying) {
                AudioData* data = Resources::getAudioDataPtr(sound.name);
                data->play(sound.source, sound.listener);
                sound.isPlaying = true;
                source = data;
                std::cout << "Playing sound: " << sound.name << std::endl;
            } else {
                if (!source->isPlaying()) {
                    std::cout << "Sound finished: " << sound.name << std::endl;
                    sounds_.erase(sounds_.begin());
                    source->stop();
                    source = nullptr; // Reset the source pointer
                }
            }
        }
    }

    static void startAudioThread() {
        stopAudioThread(); // Ensure any existing thread is stopped
        running_ = true;
        audioThread_ = std::thread([]() {
            Resources::loadAudioFiles();
            while (running_) {
                update(); // Call the update method
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
            }
        });
    }

    static void stopAudioThread() {
        if (running_) {
            running_ = false;
            if (audioThread_.joinable()) {
                audioThread_.join();
            }
        }
    }

    static void setAudio(const std::string& name,
                        glm::vec3 sourcePosition = glm::vec3(0.0f),
                        glm::vec3 listenerPosition = glm::vec3(0.0f),
                        glm::vec3 lookAt = glm::vec3(0.0f)) {
        AudioSound sound;
        sound.name = name;
        sound.source = sourcePosition;
        sound.listener = listenerPosition;
        sound.look = lookAt;
        sound.isPlaying = false;

        sounds_.push_back(sound);
    }

private:
    static inline std::thread audioThread_;
    static inline std::atomic<bool> running_ = false;

    static inline std::vector<AudioSound> sounds_;
    static inline AudioData* source = nullptr; // Pointer to the audio source
};

#endif