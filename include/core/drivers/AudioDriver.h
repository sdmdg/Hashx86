#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include <core/driver.h>
#include <types.h>

typedef void (*AudioCallback)(void* context);

class AudioDriver {
protected:
    uint32_t sampleRate;
    uint8_t channels;
    uint8_t bitsPerSample;
    bool isPlaying;
    uint8_t masterVolume;
    AudioCallback refillCallback;
    void* callbackContext;

public:
    AudioDriver() {
        this->sampleRate = 44100;
        this->channels = 2;
        this->bitsPerSample = 16;
        this->isPlaying = false;
        this->masterVolume = 100;
        this->refillCallback = nullptr;
        this->callbackContext = nullptr;
    }

    virtual ~AudioDriver() {}

    void SetRefillCallback(AudioCallback cb, void* ctx) {
        this->refillCallback = cb;
        this->callbackContext = ctx;
    }

    // --- Configuration ---
    virtual void SetFormat(uint32_t sampleRate, uint8_t channels, uint8_t bits) = 0;

    // NEW: Helper to just change rate
    virtual void SetSampleRate(uint32_t newRate) {
        if (newRate == this->sampleRate) return;
        SetFormat(newRate, this->channels, this->bitsPerSample);
    }

    virtual uint32_t GetBufferSize() = 0;
    virtual uint32_t WriteData(uint8_t* buffer, uint32_t size) = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;

    // Flow Control
    virtual bool IsReadyForData() {
        return true;
    }

    virtual void SetVolume(uint8_t vol) {
        if (vol > 100) vol = 100;
        this->masterVolume = vol;
        ApplyHardwareVolume();
    }

    uint32_t GetSampleRate() {
        return sampleRate;
    }
    uint8_t GetChannels() {
        return channels;
    }
    bool IsPlaying() {
        return isPlaying;
    }

protected:
    void NotifyRefillNeeded() {
        if (refillCallback) {
            refillCallback(callbackContext);
        }
    }
    virtual void ApplyHardwareVolume() = 0;
};

#endif
