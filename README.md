# Audio Analyzer

Esta es una utilidad que permite analizar de forma sonora archivos mp3, wav y flac y produce una salida en formato json con el objetivo de facilitar el análisis en scripts de python en un futuro.

Hice todo el proceso de compilación y desarrollo en Windows, así que no sé como se comportaría aún en otros entornos.

NOTA SOBRE IA: Este no es un proyecto vibecoding, pero sí usé a Gemini como una suerte de compañero con el que se iban discutiendo cada archivo y cada función, y sobre todo para corregir resultados y hacer debug, ajustar funcionamientos y salidas.

---

## 🚀 Características

* **Decodificación Multiformato Nativa:** Procesamiento de audio comprimido y sin compresión sin dependencias de DLLs externas.
* **Salida Estructurada (JSON):** Diseñado para integrarse directamente con `subprocess` y `json.loads` en Python o cualquier lenguaje script.
* **Análisis Temporal:** Estimación de tempo (BPM), cálculo de energía RMS, pico de amplitud y factor de cresta.
* **Análisis Espectral:** Determinación del centroide espectral, frecuencia dominante y mapeo de tono hacia notas musicales estándar con desviación en cents.

---

## 📊 Descripción de Campos del JSON

| Sección | Campo | Tipo | Descripción |
| :--- | :--- | :--- | :--- |
| **Grip** | `file_path` | `string` | Ruta relativa o absoluta del archivo analizado. |
| **`audio_info`** | `sample_rate` | `integer` | Frecuencia de muestreo del audio en Hertz (Hz). |
| | `channels` | `integer` | Cantidad de canales (1 = Mono, 2 = Estéreo). |
| | `duration_sec` | `float` | Duración total de la pista en segundos. |
| **`dsp_analysis`** | `bpm` | `float` | Estimación del tempo de la canción en Pulsaciones Por Minuto. |
| | `rms_energy` | `float` | Energía de la raíz media cuadrática (nivel promedio de potencia del audio). |
| | `peak_amplitude` | `float` | Amplitud máxima absoluta alcanzada en la señal $[-1.0, 1.0]$. |
| | `crest_factor_db` | `float` | Factor de cresta en dB (relación pico/RMS; indicador de rango dinámico). |
| | `spectral_centroid_hz` | `float` | Centroide espectral en Hz (indica el "brillo" tímbrico del audio). |
| | `dominant_freq_hz` | `float` | Frecuencia de mayor magnitud detectada en la Transformada de Fourier. |
| | `detected_note` | `string` | Nota musical equivalente a la frecuencia dominante (ej. `F4`, `A4`). |
| | `cents_deviation` | `float` | Desviación de afinación en cents respecto a la nota estándar ($A_4 = 440\text{ Hz}$). |
| | `perceived_impetus` | `float` | Índice estimado de ímpetu / densidad rítmica percibida $[0.0 - 100.0]$. |

---

## 🛠️ Requisitos e Instalación

### Requisitos Previos
* Compilador de C con soporte para C11 (GCC / MinGW)
* GNU Make

### Compilación

Clona el repositorio y ejecuta `make` en la raíz del proyecto:

```bash
make
```

El binario ejecutable se generará automáticamente en la carpeta `bin/audio_analyzer.exe` (o `bin/audio_analyzer` en sistemas POSIX).

---

## 💻 Modo de Uso

Pasa la ruta del archivo de audio como argumento de la línea de comandos:

```bash
./bin/audio_analyzer.exe "ruta/a/tu/cancion.mp3"
```

### Ejemplo de Salida:

```json
{
  "file_path": "./cancion.mp3",
  "audio_info": {
    "sample_rate": 44100,
    "channels": 2,
    "duration_sec": 211.57
  },
  "dsp_analysis": {
    "bpm": 171.4,
    "rms_energy": 0.1577,
    "peak_amplitude": 0.8865,
    "crest_factor_db": 14.99,
    "spectral_centroid_hz": 10890.62,
    "dominant_freq_hz": 22017.70,
    "detected_note": "F10",
    "cents_deviation": -26.0,
    "perceived_impetus": 38.7
  }
}
```

---

## 📚 Agradecimientos y Créditos

Este proyecto utiliza las excepcionales librerías decodificadoras *single-header* de código abierto creadas por **David Reid**:

* [dr_libs](https://github.com/mackron/dr_libs) (`dr_mp3.h`, `dr_wav.h`, `dr_flac.h`), liberadas bajo dominio público / licencia MIT.