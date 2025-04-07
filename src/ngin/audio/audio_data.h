#ifndef AUDIO_DATA_H
#define AUDIO_DATA_H

#include <AL/al.h>
#include <AL/alc.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>

class AudioData {
public:
    AudioData(const std::string& name) : name_(name), buffer_(0), source_(0), isPlaying_(false) {}

    ~AudioData() {
        stop();
        if (source_) alDeleteSources(1, &source_);
        if (buffer_) alDeleteBuffers(1, &buffer_);
    }

    bool loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open WAV file: " << filePath << std::endl;
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

        char dataHeader[4];
        file.read(dataHeader, 4);
        if (std::string(dataHeader, 4) != "data") return logError("Missing data header");

        uint32_t dataSize;
        file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

        std::vector<char> audioData(dataSize);
        file.read(audioData.data(), dataSize);

        // Generate OpenAL buffer and source
        alGenBuffers(1, &buffer_);
        ALenum format = (numChannels == 1) ? 
                        ((bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) : 
                        ((bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16);
        alBufferData(buffer_, format, audioData.data(), dataSize, sampleRate);

        alGenSources(1, &source_);
        alSourcei(source_, AL_BUFFER, buffer_);

        return true;
    }

    void play() {
        if (isPlaying_) return; // Prevent multiple play calls
        isPlaying_ = true;
        playThread_ = std::thread([this]() {
            alSourcePlay(source_);
            ALint state;
            do {
                alGetSourcei(source_, AL_SOURCE_STATE, &state);
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Polling interval
            } while (state == AL_PLAYING);
            isPlaying_ = false;
        });
        playThread_.detach(); // Detach the thread to allow asynchronous playback
    }

    void stop() {
        if (isPlaying_) {
            alSourceStop(source_);
            isPlaying_ = false;
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
};

#endif // AUDIO_DATA_H
