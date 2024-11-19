#!/bin/bash

# Lista de archivos .c con paths completos
FILES=(
    "/home/utnso/tp-2024-2c-bombi-y-asociados/cpu/src/cpu.c"
    "/home/utnso/tp-2024-2c-bombi-y-asociados/cpu/src/instrucciones.c"
    "/home/utnso/tp-2024-2c-bombi-y-asociados/cpu/src/utilsCpu.c"
    "/home/utnso/tp-2024-2c-bombi-y-asociados/filesystem/src/filesystem.c"
    "/home/utnso/tp-2024-2c-bombi-y-asociados/kernel/src/kernel.c"
    "/home/utnso/tp-2024-2c-bombi-y-asociados/memoria/src/memoria.c"
)

# Directorio de salida para los ejecutables
OUTPUT_DIR="bin"
mkdir -p $OUTPUT_DIR

# Compilar y ejecutar cada archivo
for FILE in "${FILES[@]}"; do
    BASENAME=$(basename "$FILE" .c) # Nombre del archivo sin extensión
    OUTPUT="$OUTPUT_DIR/$BASENAME" # Ruta del ejecutable

    echo "Compilando $FILE..."
    gcc "$FILE" -o "$OUTPUT"
    if [ $? -eq 0 ]; then
        echo "Ejecución de $OUTPUT:"
        "./$OUTPUT"
        echo "----------------------------"
    else
        echo "Error al compilar $FILE"
        exit 1
    fi
done

echo "Todas las pruebas han terminado."
