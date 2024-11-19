#!/bin/bash

# Ruta base del proyecto
PROJECT_PATH="/home/utnso/tp-2024-2c-bombi-y-asociados"

# Subdirectorio donde se generan los binarios
BIN_DIR="bin"

# Rutas de los módulos y sus ejecutables
MODULES=(
    "memoria"
    "cpu"
    "kernel"
    "filesystem"
)

# Compilar los módulos en orden
for MODULE in "${MODULES[@]}"; do
    MODULE_PATH="$PROJECT_PATH/$MODULE"
    BIN_PATH="$MODULE_PATH/$BIN_DIR/$MODULE"
    if [ -f "$MODULE_PATH/Makefile" ]; then
        echo "Compilando módulo: $MODULE"
        make -C "$MODULE_PATH"
        if [ $? -ne 0 ]; then
            echo "Error al compilar el módulo: $MODULE"
            exit 1
        fi
    else
        echo "No se encontró Makefile en $MODULE_PATH"
        exit 1
    fi
done

echo "Compilación completada. Iniciando ejecución en paralelo..."

# Ejecutar los módulos en el orden requerido
# Primero inicia memoria
MEMORIA_BIN="$PROJECT_PATH/memoria/$BIN_DIR/memoria"
if [ -f "$MEMORIA_BIN" ]; then
    echo "Iniciando memoria..."
    "$MEMORIA_BIN" & # Ejecuta en segundo plano
    MEMORIA_PID=$!   # Captura el PID del proceso
    echo "Memoria corriendo con PID $MEMORIA_PID"
else
    echo "No se encontró el ejecutable: $MEMORIA_BIN"
    exit 1
fi

# Esperar a que memoria inicie correctamente antes de seguir
sleep 2

# Luego inicia cpu, kernel y filesystem en paralelo
CPU_BIN="$PROJECT_PATH/cpu/$BIN_DIR/cpu"
KERNEL_BIN="$PROJECT_PATH/kernel/$BIN_DIR/kernel"
FILESYSTEM_BIN="$PROJECT_PATH/filesystem/$BIN_DIR/filesystem"

# Inicia CPU
if [ -f "$CPU_BIN" ]; then
    echo "Iniciando CPU..."
    "$CPU_BIN" & # Ejecuta en segundo plano
else
    echo "No se encontró el ejecutable: $CPU_BIN"
    exit 1
fi

# Inicia Kernel
if [ -f "$KERNEL_BIN" ]; then
    echo "Iniciando Kernel..."
    "$KERNEL_BIN" & # Ejecuta en segundo plano
else
    echo "No se encontró el ejecutable: $KERNEL_BIN"
    exit 1
fi

# Inicia Filesystem
if [ -f "$FILESYSTEM_BIN" ]; then
    echo "Iniciando Filesystem..."
    "$FILESYSTEM_BIN" & # Ejecuta en segundo plano
else
    echo "No se encontró el ejecutable: $FILESYSTEM_BIN"
    exit 1
fi

# Esperar a que todos los procesos terminen
wait

echo "Todos los módulos han terminado."
