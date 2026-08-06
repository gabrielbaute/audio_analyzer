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

## Explicación Detallada de Parámetros y Conceptos

### ¿Qué es DSP y qué hacemos aquí?

**DSP** corresponde a las siglas de ***Digital Signal Processing*** (Procesamiento Digital de Señales).

En nuestro contexto, implica tomar la señal de audio continua en el tiempo, muestrearla a una frecuencia determinada ($f_s = 44100\text{ Hz}$, por ejemplo) y aplicar transformaciones matemáticas (como la Transformada Rápida de Fourier o *FFT*) para analizar el dominio del tiempo y de la frecuencia. Lo que hacemos es cuantificar características físicas y acústicas objetivas del audio para convertirlas en métricas numéricas estructuradas.

---

### Conceptos e Impacto en Recomendación / Listas de Reproducción

#### 1. `rms_energy` (Root Mean Square Energy)

* **¿Qué es?**: La energía de la raíz media cuadrática mide el valor eficaz de la onda de audio. Se calcula mediante la expresión:

$$x_{\text{rms}} = \sqrt{\frac{1}{N} \sum_{i=1}^{N} x_i^2}$$


* **¿Por qué es útil en playlists?**: A diferencia de los picos puntuales, el RMS mide la potencia constante que el oído humano percibe como **volumen percibido o "fuerza" sonora**. Es vital para evitar saltos bruscos de intensidad entre canciones (volumen coherente) y para clasificar pistas con gran energía constante (rock, EDM, pasos marciales) versus canciones tenues o acústicas.

#### 2. `peak_amplitude` (Amplitud Máxima)

* **¿Qué es?**: El valor absoluto del punto más alto que alcanza la onda en el intervalo $[-1.0, 1.0]$.
* **¿Por qué es útil en playlists?**: Sirve para detectar si una pista fue grabada con limitación de pico (*clipping*) o si posee picos transitorios muy agresivos (impactos de batería, platillos). En combinación con el RMS, ayuda a evaluar si dos canciones van a "chocar" visual o acústicamente en la transición.

#### 3. `crest_factor_db` (Factor de Cresta)

* **¿Qué es?**: La relación entre el valor pico y el valor RMS expresada en decibelios:

$$\text{Crest Factor (dB)} = 20 \log_{10} \left( \frac{x_{\text{peak}}}{x_{\text{rms}}} \right)$$


* **¿Por qué es útil en playlists?**: Es el **medidor directo del rango dinámico**. Un factor de cresta alto (ej. $>15\text{ dB}$) indica música con mucha dinámica (música clásica, jazz acústico) donde hay partes muy suaves y picos fuertes. Un factor de cresta bajo (ej. $<6\text{ dB}$) indica música muy comprimida modernamente (*loudness wars*). Agrupar por factor de cresta asegura que la lista mantenga un "estilo de producción" consistente.

#### 4. `spectral_centroid_hz` (Centroide Espectral)

* **¿Qué es?**: Representa el "centro de masa" del espectro de frecuencias. Se calcula como la media ponderada de las frecuencias presentes en la señal:

$$\text{Centroide} = \frac{\sum_{k=1}^{M} f(k) \cdot \vert{}X(k)\vert{}}{\sum_{k=1}^{M} \vert{}X(k)\vert{}}$$


* **¿Por qué es útil en playlists?**: Es el indicador directo del **brillo o timbre**. Pistas con un centroide espectral alto se perciben como brillantes, agudas, nítidas o estridentes (guitarras eléctricas, platillos, voces agudas). Un centroide bajo indica sonidos cálidos, oscuros o pesados (bajos, contrabajos, música de ambiente). Agrupar por centroide permite crear listas "cálidas" o "brillantes".

#### 5. `dominant_freq_hz`, `detected_note` y `cents_deviation`

* **¿Qué son?**:
* `dominant_freq_hz`: La frecuencia $f$ con mayor magnitud $\vert{}X(f)\vert{}$ en el espectro.
* `detected_note`: Conversión de esa frecuencia a la nota musical más cercana en la escala temperada mediante la relación $f = 440 \cdot 2^{(n-69)/12}$.
* `cents_deviation`: La desviación en *cents* (100 cents = 1 semitono) respecto al tono exacto de la nota.


* **¿Por qué es útil en playlists?**: Permite realizar **mezclas armónicas**. Si dos canciones terminan e inician en notas compatibles (misma tonalidad o tonalidades vecinas en el Círculo de Quintas), la transición fluirá sin discordancias armónicas.

#### 6. `bpm` (Beats Per Minute) y `perceived_impetus`

* **¿Qué son?**: `bpm` mide la cadencia del pulso rítmico, mientras que `perceived_impetus` mide la densidad de ataques o la fuerza impulsiva percibida en la pista.
* **¿Por qué es útil en playlists?**: Son las métricas clave para listas enfocadas en la **actividad del usuario** (ej. entrenamientos, concentración). Un tema puede tener $120\text{ BPM}$ pero ser suave (bajo ímpetu) o extremadamente agresivo (alto ímpetu). Combinar ambas variables garantiza listas con la energía adecuada en el ritmo.

---
## 📚 Agradecimientos y Créditos

Este proyecto utiliza las excepcionales librerías decodificadoras *single-header* de código abierto creadas por **David Reid**:

* [dr_libs](https://github.com/mackron/dr_libs) (`dr_mp3.h`, `dr_wav.h`, `dr_flac.h`), liberadas bajo dominio público / licencia MIT.

### Resumen de términos:
* **Libertad de uso:** Puedes ejecutar, estudiar y modificar este software libremente.
* **Obligación de Copyleft:** Si distribuyes una versión modificada o un trabajo derivado de este proyecto, **debes** publicar el código fuente completo bajo la misma licencia GPLv3.
* **Sin garantías:** El software se proporciona "tal cual", sin garantías explícitas o implícitas.

Consulta el archivo [`LICENSE`](./LICENSE) para leer el texto completo de la licencia.