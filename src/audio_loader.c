#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

#include "audio_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Obtiene la extensión de un archivo en minúsculas.
 */
static const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

AudioBuffer *load_audio_file(const char *file_path) {
    const char *ext = get_filename_ext(file_path);
    char ext_lower[10] = {0};

    for (size_t i = 0; ext[i] && i < sizeof(ext_lower) - 1; i++) {
        ext_lower[i] = (char)tolower((unsigned char)ext[i]);
    }

    AudioBuffer *buffer = (AudioBuffer *)malloc(sizeof(AudioBuffer));
    if (!buffer) return NULL;

    // 1. Decodificación MP3
    if (strcmp(ext_lower, "mp3") == 0) {
        drmp3 mp3;
        if (!drmp3_init_file(&mp3, file_path, NULL)) {
            free(buffer);
            return NULL;
        }
        drmp3_uint64 total_frames = drmp3_get_pcm_frame_count(&mp3);
        buffer->sample_rate = mp3.sampleRate;
        buffer->channels = mp3.channels;
        buffer->total_samples = (uint32_t)(total_frames * mp3.channels);
        buffer->samples = (float *)malloc(buffer->total_samples * sizeof(float));

        drmp3_read_pcm_frames_f32(&mp3, total_frames, buffer->samples);
        drmp3_uninit(&mp3);

    // 2. Decodificación WAV
    } else if (strcmp(ext_lower, "wav") == 0) {
        drwav wav;
        if (!drwav_init_file(&wav, file_path, NULL)) {
            free(buffer);
            return NULL;
        }
        buffer->sample_rate = wav.sampleRate;
        buffer->channels = wav.channels;
        buffer->total_samples = (uint32_t)(wav.totalPCMFrameCount * wav.channels);
        buffer->samples = (float *)malloc(buffer->total_samples * sizeof(float));

        drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, buffer->samples);
        drwav_uninit(&wav);

    // 3. Decodificación FLAC
    } else if (strcmp(ext_lower, "flac") == 0) {
        drflac *pFlac = drflac_open_file(file_path, NULL);
        if (!pFlac) {
            free(buffer);
            return NULL;
        }
        buffer->sample_rate = pFlac->sampleRate;
        buffer->channels = pFlac->channels;
        buffer->total_samples = (uint32_t)(pFlac->totalPCMFrameCount * pFlac->channels);
        buffer->samples = (float *)malloc(buffer->total_samples * sizeof(float));

        drflac_read_pcm_frames_f32(pFlac, pFlac->totalPCMFrameCount, buffer->samples);
        drflac_close(pFlac);

    } else {
        fprintf(stderr, "Error: Formato '%s' no soportado.\n", ext_lower);
        free(buffer);
        return NULL;
    }

    buffer->duration_sec = (float)buffer->total_samples / (buffer->sample_rate * buffer->channels);
    return buffer;
}