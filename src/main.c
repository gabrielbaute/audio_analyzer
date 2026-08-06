/*
 * Audio Analyzer - Herramienta CLI para análisis de DSP en audio.
 * Copyright (C) 2026 Gabriel
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include "dsp_analyzer.h"
#include "audio_loader.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <ruta_al_archivo_audio>\n", argv[0]);
        return 1;
    }

    const char *audio_path = argv[1];
    AudioBuffer *audio_data = load_audio_file(audio_path);
    
    if (!audio_data) {
        fprintf(stderr, "Error al cargar el archivo de audio.\n");
        return 1;
    }

    SoundAnalysisReport *report = dsp_analyzer_process(audio_data);

    if (report) {
        printf("{\n");
        printf("  \"file_path\": \"%s\",\n", audio_path);
        printf("  \"audio_info\": {\n");
        printf("    \"sample_rate\": %u,\n", audio_data->sample_rate);
        printf("    \"channels\": %u,\n", audio_data->channels);
        printf("    \"duration_sec\": %.2f\n", audio_data->duration_sec);
        printf("  },\n");
        printf("  \"dsp_analysis\": {\n");
        printf("    \"bpm\": %.1f,\n", report->time_info.bpm);
        printf("    \"rms_energy\": %.4f,\n", report->time_info.rms_energy);
        printf("    \"peak_amplitude\": %.4f,\n", report->time_info.peak_amplitude);
        printf("    \"crest_factor_db\": %.2f,\n", report->time_info.crest_factor_db);
        printf("    \"spectral_centroid_hz\": %.2f,\n", report->freq_info.spectral_centroid);
        printf("    \"dominant_freq_hz\": %.2f,\n", report->freq_info.dominant_freq);
        printf("    \"detected_note\": \"%s\",\n", report->freq_info.note_name);
        printf("    \"cents_deviation\": %.1f,\n", report->freq_info.cents_deviation);
        printf("    \"perceived_impetus\": %.1f\n", report->perceived_impetus);
        printf("  }\n");
        printf("}\n");

        dsp_analyzer_free_report(report);
    }

    free(audio_data->samples);
    free(audio_data);

    return 0;
}