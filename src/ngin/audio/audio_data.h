#ifndef AUDIO_DATA_H
#define AUDIO_DATA_H

#include <AL/al.h>
#include <AL/alc.h>
#include <glm/vec3.hpp> // Include glm for 3D vector support
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>


#include <cstdint>


class AudioData {
public:
    AudioData(const std::string& name) : name_(name), buffer_(0), source_(0), isPlaying_(false) {}

    ~AudioData() {
        stop(); // Ensure playback is stopped before cleanup
        if (playThread_.joinable()) {
            playThread_.join(); // Wait for the playback thread to finish
        }
        if (source_) {
            alDeleteSources(1, &source_);
            source_ = 0; // Reset to avoid dangling references
        }
        if (buffer_) {
            alDeleteBuffers(1, &buffer_);
            buffer_ = 0; // Reset to avoid dangling references
        }
    }

    bool loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open WAV file: " << filePath << std::endl;
            return false;
        }

        bool isMono = isWavMono(filePath);
        if (!isMono) {
            std::cerr << "Only mono WAV files are supported." << std::endl;
            return false;
        }

        // Read WAV headers and data
        char riffHeader[4];
        file.read(riffHeader, 4);
        if (std::string(riffHeader, 4) != "RIFF") return logError("Missing RIFF header");

        file.ignore(4); // Skip file size

        char waveHeader[4];
        file.read(waveHeader, 4);
        if (std::string(waveHeader, 4) != "WAVE") return logError("Missing WAVE header");

        char fmtHeader[4];
        file.read(fmtHeader, 4);
        if (std::string(fmtHeader, 4) != "fmt ") return logError("Missing fmt header");

        uint32_t fmtChunkSize;
        file.read(reinterpret_cast<char*>(&fmtChunkSize), sizeof(fmtChunkSize));

        uint16_t audioFormat, numChannels;
        uint32_t sampleRate, byteRate;
        uint16_t blockAlign, bitsPerSample;

        file.read(reinterpret_cast<char*>(&audioFormat), sizeof(audioFormat));
        file.read(reinterpret_cast<char*>(&numChannels), sizeof(numChannels));
        file.read(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));
        file.read(reinterpret_cast<char*>(&byteRate), sizeof(byteRate));
        file.read(reinterpret_cast<char*>(&blockAlign), sizeof(blockAlign));
        file.read(reinterpret_cast<char*>(&bitsPerSample), sizeof(bitsPerSample));

        if (audioFormat != 1) return logError("Only PCM format is supported");

        file.ignore(fmtChunkSize - 16); // Skip extra fmt chunk data

        uint32_t dataSize;
        file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

        std::vector<char> audioData(dataSize);
        file.read(audioData.data(), dataSize);

        // Generate OpenAL buffer and source
        alGenBuffers(1, &buffer_);
        if (!checkOpenALError("alGenBuffers")) return false;

        ALenum format = (numChannels == 1) ? 
                        ((bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) : 
                        ((bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16);
        alBufferData(buffer_, format, audioData.data(), dataSize, sampleRate);
        if (!checkOpenALError("alBufferData")) return false;

        alGenSources(1, &source_);
        if (!checkOpenALError("alGenSources")) return false;

        alSourcei(source_, AL_BUFFER, buffer_);
        if (!checkOpenALError("alSourcei (AL_BUFFER)")) return false;

        alSource3f(source_, AL_POSITION, 0.0f, 0.0f, 0.0f);
        if (!checkOpenALError("alSource3f (AL_POSITION)")) return false;

        alSource3f(source_, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        if (!checkOpenALError("alSource3f (AL_VELOCITY)")) return false;

        alSourcei(source_, AL_SOURCE_RELATIVE, AL_FALSE);
        if (!checkOpenALError("alSourcei (AL_SOURCE_RELATIVE)")) return false;

        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
        if (!checkOpenALError("alDistanceModel")) return false;

        return true;
    }

    bool isWavMono(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open WAV file: " << filePath << std::endl;
            return false; // Error opening the file
        }

        // Check for RIFF header
        char riffHeader[4];
        file.read(riffHeader, 4);
        if (std::string(riffHeader, 4) != "RIFF") {
            std::cerr << "Not a valid RIFF file." << std::endl;
            return false;
        }

        // Skip file size and WAVE header.
        file.ignore(8);

        //check for fmt header
        char fmtHeader[4];
        file.read(fmtHeader, 4);
        if (std::string(fmtHeader, 4) != "fmt ") {
            std::cerr << "Not a valid fmt file." << std::endl;
            return false;
        }

        //skip fmt chunk size.
        file.ignore(4);

        // Skip audio format.
        file.ignore(2);

        // Read number of channels.
        uint16_t numChannels;
        file.read(reinterpret_cast<char*>(&numChannels), sizeof(numChannels));

        return numChannels == 1; // Return true if mono, false if not.
    }

    void play(const glm::vec3& sourcePosition, const glm::vec3& listenerPosition) {
        if (isPlaying_) return; // Prevent multiple play calls

        // Set listener position and orientation (default to facing forward)
        alListener3f(AL_POSITION, listenerPosition.x, listenerPosition.y, listenerPosition.z);
        if (!checkOpenALError("alListener3f (AL_POSITION)")) return;

        ALfloat listenerOrientation[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f }; // Forward and up vectors
        alListenerfv(AL_ORIENTATION, listenerOrientation);
        if (!checkOpenALError("alListenerfv (AL_ORIENTATION)")) return;

        // Set source position
        alSource3f(source_, AL_POSITION, sourcePosition.x, sourcePosition.y, sourcePosition.z);
        if (!checkOpenALError("alSource3f (AL_POSITION)")) return;

        // Set source gain (volume is now determined by distance and OpenAL attenuation model)
        alSourcef(source_, AL_GAIN, 1.0f);
        if (!checkOpenALError("alSourcef (AL_GAIN)")) return;

        // Ensure the source is not relative to the listener for spatial effects
        alSourcei(source_, AL_SOURCE_RELATIVE, AL_FALSE);
        if (!checkOpenALError("alSourcei (AL_SOURCE_RELATIVE)")) return;

        isPlaying_ = true;
        std::cout << "AudioData: Source is starting to play." << std::endl; // Log when playback starts
        playThread_ = std::thread([this]() {
            alSourcePlay(source_);
            if (!checkOpenALError("alSourcePlay")) return;
        });
    }

    void stop() {
        if (isPlaying_) {
            std::cout << "AudioData: Stopping the source." << std::endl; // Log when stopping is initiated
            isPlaying_ = false; // Signal the thread to stop
            playThread_.join(); // Wait for the playback thread to finish
            alSourceStop(source_); // Stop the OpenAL source
            std::cout << "AudioData: Source has been stopped." << std::endl; // Log after stopping
        }
    }

    const std::string& getName() const {
        return name_;
    }

private:
    std::string name_;
    ALuint buffer_;
    ALuint source_;
    std::atomic<bool> isPlaying_;
    std::thread playThread_;

    bool logError(const std::string& message) {
        std::cerr << "AudioData Error: " << message << std::endl;
        return false;
    }

    bool checkOpenALError(const std::string& context) {
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cerr << "OpenAL Error (" << context << "): " << alGetString(error) << std::endl;
            return false;
        }
        return true;
    }
};

#endif // AUDIO_DATA_H
