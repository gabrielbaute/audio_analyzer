#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "dsp_analyzer.h"

#define M_PI 3.14159265358979323846

/* Nombres de las notas de la escala cromática */
static const char *NOTE_NAMES[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

/* --- FUNCIONES HELPER / PRIVADAS --- */

/* Algoritmo Cooley-Tukey FFT Radix-2 In-Place */
static void fft_radix2(float *real, float *imag, size_t n) {
    size_t j = 0;
    for (size_t i = 0; i < n - 1; i++) {
        if (i < j) {
            float temp_r = real[i];
            float temp_i = imag[i];
            real[i] = real[j];
            imag[i] = imag[j];
            real[j] = temp_r;
            imag[j] = temp_i;
        }
        size_t k = n >> 1;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }

    for (size_t len = 2; len <= n; len <<= 1) {
        float ang = (float)(-2.0 * M_PI / len);
        float wlen_r = cosf(ang);
        float wlen_i = sinf(ang);

        for (size_t i = 0; i < n; i += len) {
            float w_r = 1.0f;
            float w_i = 0.0f;
            for (size_t k = 0; k < len / 2; k++) {
                size_t u = i + k;
                size_t v = i + k + len / 2;

                float vr = real[v] * w_r - imag[v] * w_i;
                float vi = real[v] * w_i + imag[v] * w_r;

                real[v] = real[u] - vr;
                imag[v] = imag[u] - vi;
                real[u] += vr;
                imag[u] += vi;

                float next_w_r = w_r * wlen_r - w_i * wlen_i;
                float next_w_i = w_r * wlen_i + w_i * wlen_r;
                w_r = next_w_r;
                w_i = next_w_i;
            }
        }
    }
}

/* Convierte frecuencia en Hz a nota temperada formal */
static void frequency_to_note(float freq, char *note_out, size_t max_len, float *cents_out) {
    if (freq <= 0.0f) {
        snprintf(note_out, max_len, "N/A");
        *cents_out = 0.0f;
        return;
    }

    float midi_number = 69.0f + 12.0f * log2f(freq / 440.0f);
    int rounded_midi = (int)roundf(midi_number);

    int note_index = rounded_midi % 12;
    if (note_index < 0) note_index += 12;
    
    int octave = (rounded_midi / 12) - 1;

    snprintf(note_out, max_len, "%s%d", NOTE_NAMES[note_index], octave);
    *cents_out = (midi_number - (float)rounded_midi) * 100.0f;
}

/**
 * Estima los BPM (Beats Per Minute) mediante Detección de Envolvente y Autocorrelación.
 */
static float calculate_bpm(const AudioBuffer *buffer) {
    if (buffer == NULL || buffer->total_samples < buffer->sample_rate) {
        return 0.0f; /* Se necesita al menos 1 segundo de audio */
    }

    /* 1. Submuestrear/Envolvente: Ventanas RMS cada ~10 ms (100 Hz de tasa de envolvente) */
    size_t hop_size = buffer->sample_rate / 100;
    if (hop_size == 0) hop_size = 1;
    
    size_t num_frames = buffer->total_samples / hop_size;
    float *envelope = (float *) calloc(num_frames, sizeof(float));
    float *onset_df = (float *) calloc(num_frames, sizeof(float));

    if (!envelope || !onset_df) {
        free(envelope);
        free(onset_df);
        return 0.0f;
    }

    /* Calcular la envolvente de energía RMS */
    for (size_t i = 0; i < num_frames; i++) {
        double sum = 0.0;
        size_t start = i * hop_size;
        for (size_t j = 0; j < hop_size && (start + j) < buffer->total_samples; j++) {
            float s = buffer->samples[start + j];
            sum += (double)s * (double)s;
        }
        envelope[i] = sqrtf((float)(sum / hop_size));
    }

    /* 2. Función de Detección de Novedades (Onsets): Diferencia rectificada */
    for (size_t i = 1; i < num_frames; i++) {
        float diff = envelope[i] - envelope[i - 1];
        onset_df[i] = (diff > 0.0f) ? diff : 0.0f;
    }

    /* 3. Autocorrelación para buscar periodicidades de tempo */
    float env_rate = (float)buffer->sample_rate / (float)hop_size;
    
    /* Rango de búsqueda: 60 BPM a 200 BPM */
    size_t min_lag = (size_t)(env_rate * 60.0f / 200.0f);
    size_t max_lag = (size_t)(env_rate * 60.0f / 60.0f);

    if (max_lag >= num_frames) max_lag = num_frames - 1;

    float max_autocorr = 0.0f;
    size_t best_lag = 0;

    for (size_t lag = min_lag; lag <= max_lag; lag++) {
        double autocorr = 0.0;
        for (size_t i = 0; i < num_frames - lag; i++) {
            autocorr += (double)onset_df[i] * (double)onset_df[i + lag];
        }

        if ((float)autocorr > max_autocorr) {
            max_autocorr = (float)autocorr;
            best_lag = lag;
        }
    }

    free(envelope);
    free(onset_df);

    if (best_lag == 0) return 0.0f;

    /* Convertir el lag ganador a BPM */
    float bpm = (60.0f * env_rate) / (float)best_lag;
    return bpm;
}

/* --- API PÚBLICA --- */

SoundAnalysisReport* dsp_analyzer_process(const AudioBuffer *buffer) {
    if (buffer == NULL || buffer->samples == NULL || buffer->total_samples == 0) {
        return NULL;
    }

    SoundAnalysisReport *report = (SoundAnalysisReport *) malloc(sizeof(SoundAnalysisReport));
    if (report == NULL) return NULL;

    /* 1. DOMINIO DEL TIEMPO */
    double sum_squares = 0.0;
    float max_peak = 0.0f;

    for (size_t i = 0; i < buffer->total_samples; i++) {
        float abs_val = fabsf(buffer->samples[i]);
        if (abs_val > max_peak) max_peak = abs_val;
        sum_squares += (double)buffer->samples[i] * (double)buffer->samples[i];
    }

    report->time_info.rms_energy = sqrtf((float)(sum_squares / buffer->total_samples));
    report->time_info.peak_amplitude = max_peak;
    
    if (report->time_info.rms_energy > 0.00001f) {
        report->time_info.crest_factor_db = 20.0f * log10f(max_peak / report->time_info.rms_energy);
    } else {
        report->time_info.crest_factor_db = 0.0f;
    }

    /* Estimación de BPM */
    report->time_info.bpm = calculate_bpm(buffer);

    /* 2. DOMINIO DE LA FRECUENCIA (FFT) */
    size_t fft_size = 4096;
    if (buffer->total_samples < fft_size) fft_size = 1024;

    float *real = (float *) calloc(fft_size, sizeof(float));
    float *imag = (float *) calloc(fft_size, sizeof(float));

    if (!real || !imag) {
        free(real); free(imag);
        free(report);
        return NULL;
    }

    for (size_t i = 0; i < fft_size; i++) {
        float hann = 0.5f * (1.0f - cosf((float)(2.0 * M_PI * i / (fft_size - 1))));
        real[i] = buffer->samples[i] * hann;
        imag[i] = 0.0f;
    }

    fft_radix2(real, imag, fft_size);

    float max_magnitude = 0.0f;
    size_t peak_bin = 0;
    double weighted_freq_sum = 0.0;
    double total_magnitude_sum = 0.0;

    for (size_t k = 0; k < fft_size / 2; k++) {
        float mag = sqrtf(real[k] * real[k] + imag[k] * imag[k]);
        float bin_freq = (float)k * buffer->sample_rate / (float)fft_size;

        if (mag > max_magnitude) {
            max_magnitude = mag;
            peak_bin = k;
        }

        weighted_freq_sum += (double)(bin_freq * mag);
        total_magnitude_sum += (double)mag;
    }

    report->freq_info.dominant_freq = (float)peak_bin * buffer->sample_rate / (float)fft_size;

    if (total_magnitude_sum > 0.0) {
        report->freq_info.spectral_centroid = (float)(weighted_freq_sum / total_magnitude_sum);
    } else {
        report->freq_info.spectral_centroid = 0.0f;
    }

    frequency_to_note(report->freq_info.dominant_freq, 
                      report->freq_info.note_name, 
                      sizeof(report->freq_info.note_name), 
                      &report->freq_info.cents_deviation);

    /* 3. ÍMPETU PERCIBIDO */
    float normalized_rms = report->time_info.rms_energy * 2.0f;
    if (normalized_rms > 1.0f) normalized_rms = 1.0f;

    float normalized_centroid = report->freq_info.spectral_centroid / (buffer->sample_rate / 2.0f);
    
    report->perceived_impetus = (normalized_rms * 60.0f) + (normalized_centroid * 40.0f);

    free(real);
    free(imag);

    return report;
}

void dsp_analyzer_free_report(SoundAnalysisReport *report) {
    if (report != NULL) {
        free(report);
    }
}