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
    AudioData(const std::string& name) : name_(name), buffer_(0), source_(0), isPlaying_(false), durationInSeconds_(0) {}

    ~AudioData() {
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

        uint32_t dataSize = 0;

        // Locate the "data" chunk to correctly read its size
        char dataHeader[4];
        while (file.read(dataHeader, 4)) {
            if (std::string(dataHeader, 4) == "data") {
                file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
                break;
            } else {
                uint32_t chunkSize;
                file.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));
                file.ignore(chunkSize); // Skip unknown chunk
            }
        }

        if (dataSize == 0) return logError("Missing or invalid data chunk");

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

        // Correctly calculate and store the duration of the audio file
        int numSamples = dataSize / (numChannels * (bitsPerSample / 8));
        durationInSeconds_ = static_cast<int>(numSamples / static_cast<float>(sampleRate));
        std::cout << "Audio file duration: " << durationInSeconds_ << " seconds." << std::endl;

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

        alSourcePlay(source_);
        if (!checkOpenALError("alSourcePlay")) return;
        isPlaying_ = true; // Set to true when playback starts
    }

    void stop() {
        if (source_) {
            alSourceStop(source_); // Stop the OpenAL source
            if (!checkOpenALError("alSourceStop")) return;

            alSourcei(source_, AL_BUFFER, 0); // Detach the buffer from the source
            if (!checkOpenALError("alSourcei (AL_BUFFER)")) return;

            isPlaying_ = false; // Set to false when playback stops
        }
    }

    const std::string& getName() const {
        return name_;
    }

    bool isPlaying() {
        ALint state; // Update isPlaying_ based on the actual source state
        alGetSourcei(source_, AL_SOURCE_STATE, &state);
        isPlaying_ = (state == AL_PLAYING);
        //std::cout << "Checking state for source " << source_ << ": state = " << state
        //<< " (AL_PLAYING=" << AL_PLAYING << ", AL_STOPPED=" << AL_STOPPED << ")" << std::endl;
        return isPlaying_;
    }

    int getDurationInSeconds() const {
        return durationInSeconds_;
    }

private:
    std::string name_;
    ALuint buffer_;
    ALuint source_;
    bool isPlaying_ = false; // Track playback state
    int durationInSeconds_ = 0; // Store the duration of the audio file

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
