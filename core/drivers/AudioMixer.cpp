/**
 * @file        AudioMixer.cpp
 * @brief       Audio Mixer for #x86
 *
 * @date        29/01/2026
 * @version     1.0.0-beta
 */

#include <core/drivers/AudioMixer.h>

AudioMixer::AudioMixer(AudioDriver* drv) : driver(drv), mixBuffer(nullptr), bufferSize(0) {
    memset(streams, 0, sizeof(streams));

    bufferSize = driver->GetBufferSize();
    mixBuffer = (uint8_t*)kmalloc(bufferSize);

    if (mixBuffer) {
        memset(mixBuffer, 0, bufferSize);
    }
}

void AudioMixer::SetOutputSampleRate(uint32_t rate) {
    if (driver) driver->SetSampleRate(rate);
}

void AudioMixer::PlayBuffer(uint8_t* data, uint32_t length, bool loop) {
    if (!data || length == 0) return;

    for (int i = 0; i < 8; i++) {
        if (!streams[i].active) {
            streams[i].data = data;
            streams[i].length = length;
            streams[i].position = 0;
            streams[i].looping = loop;
            streams[i].active = true;
            break;
        }
    }

    if (!driver->IsPlaying()) {
        // Fill ALL available hardware buffers before starting.
        while (driver->IsReadyForData()) {
            ProcessAudio();
        }
        driver->Start();
    }
}

void AudioMixer::Update() {
    if (!driver || !mixBuffer) return;

    // Keep filling as long as hardware has space
    while (driver->IsReadyForData()) {
        ProcessAudio();
    }
}

void AudioMixer::ProcessAudio() {
    memset(mixBuffer, 0, bufferSize);

    int16_t* out = (int16_t*)mixBuffer;
    uint32_t samples = bufferSize / sizeof(int16_t);
    bool activeStreams = false;

    for (int s = 0; s < 8; s++) {
        AudioStream& st = streams[s];
        if (!st.active) continue;

        activeStreams = true;
        int16_t* src = (int16_t*)(st.data + st.position);

        for (uint32_t i = 0; i < samples; i++) {
            if (st.position >= st.length) {
                if (st.looping) {
                    st.position = 0;
                    src = (int16_t*)st.data;
                } else {
                    st.active = false;
                    break;
                }
            }

            int32_t mixed = out[i] + src[i];

            // Hard Clipping prevention
            if (mixed > 32767) mixed = 32767;
            if (mixed < -32768) mixed = -32768;

            out[i] = (int16_t)mixed;
            st.position += sizeof(int16_t);
        }
    }

    driver->WriteData(mixBuffer, bufferSize);
}
