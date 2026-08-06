#ifndef AUDIO_LOADER_H
#define AUDIO_LOADER_H

#include "dsp_analyzer.h"

/**
 * Carga un archivo de audio (MP3, WAV o FLAC) desde disco y lo convierte 
 * a un AudioBuffer PCM en coma flotante.
 *
 * @param file_path Ruta al archivo de audio.
 * @return Puntero a la estructura AudioBuffer asignada dinámicamente o NULL si falla.
 */
AudioBuffer *load_audio_file(const char *file_path);

#endif // AUDIO_LOADER_H