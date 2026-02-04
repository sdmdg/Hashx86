#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include <core/drivers/AudioDriver.h>
#include <core/memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils/string.h>

class AudioDriver;

struct AudioStream {
    uint8_t* data;
    uint32_t length;
    uint32_t position;
    bool active;
    bool looping;
};

class AudioMixer {
private:
    AudioDriver* driver;
    AudioStream streams[8];

    uint8_t* mixBuffer;
    uint32_t bufferSize;

public:
    explicit AudioMixer(AudioDriver* drv);

    void PlayBuffer(uint8_t* data, uint32_t length, bool loop);

    void SetOutputSampleRate(uint32_t rate);

    void Update();

private:
    void ProcessAudio();
};

#endif  // AUDIO_MIXER_H
