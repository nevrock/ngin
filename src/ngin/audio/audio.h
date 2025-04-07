#ifndef AUDIO_H
#define AUDIO_H

#include <iostream>
#include <fstream>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include <cstdint>

#include <ngin/resources.h>

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

    static bool play(const std::string& name) {
        AudioData& data = Resources::getAudioData(name);
        data.play();
        return true;
    }

private:
};

#endif