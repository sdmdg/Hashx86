#ifndef WAV_H
#define WAV_H

#include <console.h>
#include <core/drivers/AudioMixer.h>
#include <core/filesystem/FAT32.h>
#include <core/filesystem/File.h>
#include <core/filesystem/msdospart.h>
#include <core/memory.h>
#include <stdint.h>
#include <utils/string.h>

// WAV File Structures
struct WavHeader {
    char riff[4];          // "RIFF"
    uint32_t overallSize;  // File size - 8
    char wave[4];          // "WAVE"
};

struct ChunkHeader {
    char id[4];
    uint32_t size;
};

struct WavFmt {
    uint16_t audioFormat;  // 1 = PCM
    uint16_t numChannels;  // 1 = Mono, 2 = Stereo
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

class Wav {
public:
    // Audio Data
    uint8_t* buffer;
    uint32_t length;

    // Properties
    uint32_t sampleRate;
    uint8_t channels;
    uint8_t bitsPerSample;
    bool valid;

public:
    // Constructors matching Bitmap pattern
    Wav(File* file);
    Wav(char* path);
    ~Wav();

    // Sends the audio to the global mixer
    void Play(bool loop = false);

private:
    void Load(File* file);
};

#endif
