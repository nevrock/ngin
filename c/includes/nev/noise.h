#ifndef NOISE_H
#define NOISE_H

#include <noise/FastNoiseLite.h>
#include <nev/dict.h>
#include <iostream>  // Include the iostream header for console output

class Noise {
public:
    Noise(
        unsigned int seed = 1237,
        float frequency = 0.01,
        FastNoiseLite::NoiseType type = FastNoiseLite::NoiseType_Perlin,
        FastNoiseLite::FractalType fractalType = FastNoiseLite::FractalType_FBm,
        unsigned int octaves = 5,
        float lacunarity = 2.0,
        float gain = 0.5,
        float weightedStrength = 0.45
    ) : seed_(seed), frequency_(frequency), type_(type),
        fractalType_(fractalType), octaves_(octaves), lacunarity_(lacunarity),
        gain_(gain), weightedStrength_(weightedStrength) {
    }

    Noise(
        const Dict* d
    ) : dict_(d), 
        seed_(static_cast<unsigned int>(d->getC<int>("seed", 1237))),
        frequency_(d->getC<float>("frequency", 0.01)), 
        type_(static_cast<FastNoiseLite::NoiseType>(d->getC<int>("type", FastNoiseLite::NoiseType_Perlin))),
        fractalType_(static_cast<FastNoiseLite::FractalType>(d->getC<int>("fractal_type", FastNoiseLite::FractalType_FBm))), 
        octaves_(static_cast<unsigned int>(d->getC<int>("octaves", 5))), 
        lacunarity_(d->getC<float>("lacunarity", 2.0)),
        gain_(d->getC<float>("gain", 0.5)), 
        weightedStrength_(d->getC<float>("weighted_strength", 0.45)) {
    }

    float getNoise(float x, float y) {
        FastNoiseLite noiseGenerator;

        noiseGenerator.SetSeed(seed_); 
        noiseGenerator.SetFrequency(frequency_); 
        noiseGenerator.SetNoiseType(type_); 
        noiseGenerator.SetFractalType(fractalType_); 
        noiseGenerator.SetFractalOctaves(octaves_); 
        noiseGenerator.SetFractalLacunarity(lacunarity_); 
        noiseGenerator.SetFractalGain(gain_); 
        noiseGenerator.SetFractalWeightedStrength(weightedStrength_); 

        float noiseValue = noiseGenerator.GetNoise(x, y);

        // Print the parameters and the generated noise value to the console
        /*
        std::cout << "Noise Parameters: Seed=" << seed_
                  << ", Frequency=" << frequency_
                  << ", Type=" << type_
                  << ", FractalType=" << fractalType_
                  << ", Octaves=" << octaves_
                  << ", Lacunarity=" << lacunarity_
                  << ", Gain=" << gain_
                  << ", WeightedStrength=" << weightedStrength_
                  << std::endl;

        std::cout << "Generated Noise Value at (" << x << ", " << y << "): " << noiseValue << std::endl;
        */

        return noiseValue;
    }

private:
    const Dict* dict_;
    unsigned int seed_;
    float frequency_;
    FastNoiseLite::NoiseType type_;
    FastNoiseLite::FractalType fractalType_;
    unsigned int octaves_;
    float lacunarity_;
    float gain_;
    float weightedStrength_;
};

#endif // NOISE_H
