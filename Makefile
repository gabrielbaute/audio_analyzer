# ==============================================================================
# Makefile para el Analizador Sonoro en C (Windows Nativo / Scoop)
# ==============================================================================

# Compilador y Flags
CC       = gcc
CFLAGS   = -Wall -Wextra -std=c11 -Iinclude -O2
LDFLAGS  = -lm

# Directorios
SRC_DIR   = src
INC_DIR   = include
BUILD_DIR = build
BIN_DIR   = bin

# Nombre del ejecutable de salida
TARGET    = $(BIN_DIR)/audio_analyzer.exe

# Detección automática de archivos fuente (.c) y objetos (.o)
SRCS      = $(wildcard $(SRC_DIR)/*.c)
OBJS      = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Configuración del Shell para Windows Nativo
SHELL     = cmd.exe

# ------------------------------------------------------------------------------
# Reglas Principales
# ------------------------------------------------------------------------------

# Regla por defecto: Construir el ejecutable
all: $(TARGET)

# Enlazado final (Linking)
$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo [LINK] $@
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compilación de cada módulo (.c -> .o)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo [CC] $<
	$(CC) $(CFLAGS) -c $< -o $@

# Creación de carpetas invocando a cmd.exe
$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

$(BIN_DIR):
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

# Limpieza de archivos de compilación
clean:
	@echo Limpiando archivos generados...
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@if exist "$(BIN_DIR)" rmdir /s /q "$(BIN_DIR)"

.PHONY: all clean