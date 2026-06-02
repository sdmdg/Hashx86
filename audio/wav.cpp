/**
 * @file        wav.cpp
 * @brief       WAV Audio File Handler
 *
 * @date        11/02/2026
 * @version     1.0.0
 */

#define KDBG_COMPONENT "WAV"
#include <audio/wav.h>

// External reference to the mixer created in kernel.cpp
extern AudioMixer* g_AudioMixer;

Wav::Wav(File* file) {
    this->valid = false;
    this->buffer = 0;
    this->length = 0;
    this->sampleRate = 0;
    Load(file);
}

Wav::Wav(char* path) {
    this->valid = false;
    this->buffer = 0;
    this->length = 0;
    this->sampleRate = 0;

    // Get the active filesystem
    if (!MSDOSPartitionTable::activeInstance ||
        !MSDOSPartitionTable::activeInstance->partitions[0]) {
        KDBG1("Error: File system not ready.");
        return;
    }

    FAT32* fs = MSDOSPartitionTable::activeInstance->partitions[0];
    File* file = fs->Open(path);

    if (file == 0) {
        KDBG1("Error: File not found %s", path);
        return;
    }

    if (file->size == 0) {
        KDBG1("Error: File is empty %s", path);
        file->Close();
        delete file;
        return;
    }

    Load(file);
    file->Close();
    delete file;
}

Wav::~Wav() {
    if (buffer) {
        kfree(buffer);
        buffer = 0;
    }
}

void Wav::Load(File* file) {
    if (!file) return;

    WavHeader header;
    if (file->Read((uint8_t*)&header, sizeof(WavHeader)) != sizeof(WavHeader)) {
        KDBG1("Error: Header read failed.");
        return;
    }

    // signature checks
    if (strncmp(header.riff, "RIFF", 4) != 0 || strncmp(header.wave, "WAVE", 4) != 0) {
        KDBG1("Error: Invalid RIFF/WAVE signature.");
        return;
    }

    WavFmt fmt;
    bool fmtFound = false;
    bool dataFound = false;
    uint32_t dataOffset = 0;
    uint32_t dataSize = 0;

    // Parse Chunks
    while (file->position < file->size) {
        ChunkHeader chunk;
        if (file->Read((uint8_t*)&chunk, sizeof(ChunkHeader)) != sizeof(ChunkHeader)) break;

        uint32_t chunkStart = file->position;

        // "fmt " chunk
        if (strncmp(chunk.id, "fmt ", 4) == 0) {
            if (chunk.size < sizeof(WavFmt)) {
                KDBG1("Error: FMT chunk too small.");
                return;
            }
            file->Read((uint8_t*)&fmt, sizeof(WavFmt));
            fmtFound = true;

            // Skip extra bytes if fmt chunk is larger than expected
            if (chunk.size > sizeof(WavFmt)) {
                file->Seek(chunkStart + chunk.size);
            }
        }
        // "data" chunk
        else if (strncmp(chunk.id, "data", 4) == 0) {
            dataOffset = file->position;
            dataSize = chunk.size;
            dataFound = true;
            break;
        }
        // Unknown chunk -> Skip
        else {
            file->Seek(chunkStart + chunk.size);
        }

        // Handle padding byte if chunk size is odd
        if (chunk.size % 2 != 0) {
            file->Seek(file->position + 1);
        }
    }

    if (!fmtFound || !dataFound) {
        KDBG1("Error: Missing FMT or DATA chunk.");
        return;
    }

    // Validation (Enforce 16-bit PCM for now, as per your mixer limits)
    if (fmt.audioFormat != 1) {
        KDBG1("Error: Not PCM format (Format=%d).", fmt.audioFormat);
        return;
    }

    // Store properties
    this->sampleRate = fmt.sampleRate;
    this->channels = fmt.numChannels;
    this->bitsPerSample = fmt.bitsPerSample;
    this->length = dataSize;

    // Allocate Memory
    this->buffer = (uint8_t*)kmalloc(this->length);
    if (!this->buffer) {
        KDBG1("Error: Out of memory (Size=%d)", this->length);
        return;
    }

    // Read Data
    file->Seek(dataOffset);
    uint32_t read = file->Read(this->buffer, this->length);

    if (read != this->length) {
        KDBG2("Warning: Read mismatch (%d vs %d)", (int32_t)read, (int32_t)this->length);
    }

    this->valid = true;
    KDBG2("Loaded: %d Hz, %d-bit, %s (%d bytes)", sampleRate, bitsPerSample,
          (channels == 2) ? "Stereo" : "Mono", length);
}

void Wav::Play(bool loop) {
    if (!valid || !buffer || !g_AudioMixer) return;

    // Configure Hardware Rate
    g_AudioMixer->SetOutputSampleRate(this->sampleRate);

    // Play
    g_AudioMixer->PlayBuffer(this->buffer, this->length, loop);
}
