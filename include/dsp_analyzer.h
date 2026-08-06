#ifndef DSP_ANALYZER_H
#define DSP_ANALYZER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Estructura para representar el buffer PCM decodificado */
typedef struct {
    uint32_t sample_rate;    /* Ej: 44100 Hz */
    uint16_t channels;       /* Ej: 1 (mono) o 2 (estéreo) */
    size_t total_samples;    /* Número total de muestras por canal */
    float *samples;          /* Array de muestras normalizadas [-1.0f, 1.0f] */
    float duration_sec;      /* Duración total del audio en segundos */
} AudioBuffer;

/* Métricas del Dominio del Tiempo */
typedef struct {
    float rms_energy;        /* Energía RMS promedio [0.0 - 1.0] */
    float peak_amplitude;    /* Valor pico absoluto [0.0 - 1.0] */
    float crest_factor_db;   /* Relación Pico/RMS en dB (Rango dinámico) */
    float bpm;               /* Tempo estimado en Beats Per Minute */
} TimeDomainFeatures;

/* Métricas del Dominio de la Frecuencia (Espectral) */
typedef struct {
    float spectral_centroid; /* Frecuencia centroide en Hz (Brillo) */
    float dominant_freq;     /* Frecuencia pico en Hz */
    char note_name[8];       /* Nota musical aproximada (Ej: "F4", "C#3") */
    float cents_deviation;   /* Desviación de afinación en cents [-50.0 a +50.0] */
} FrequencyDomainFeatures;

/* Informe completo del Análisis Sonoro */
typedef struct {
    TimeDomainFeatures time_info;
    FrequencyDomainFeatures freq_info;
    float perceived_impetus; /* Índice de ímpetu combinado [0.0 - 100.0] */
} SoundAnalysisReport;

/**
 * Procesa un AudioBuffer PCM y genera un informe de análisis sonoro.
 * 
 * @param buffer Puntero a la estructura AudioBuffer con los datos de entrada.
 * @return SoundAnalysisReport* Puntero al informe generado o NULL en caso de error.
 */
SoundAnalysisReport* dsp_analyzer_process(const AudioBuffer *buffer);

/**
 * Libera la memoria asignada para un SoundAnalysisReport.
 * 
 * @param report Puntero al informe a liberar.
 */
void dsp_analyzer_free_report(SoundAnalysisReport *report);

#endif /* DSP_ANALYZER_H */