#!/bin/bash

# Nombre del directorio de compilación
build_dir="build"

# Directorio donde se encuentra el archivo CMakeLists.txt
source_dir="../"

# Verificar si el directorio de compilación existe, si no, crearlo
if [ ! -d "$build_dir" ]; then
    mkdir "$build_dir"
fi

# Ingresar al directorio de compilación
cd "$build_dir"

# Ejecutar CMake para generar los archivos de construcción
cmake "$source_dir"

# Compilar el proyecto
make

# Verificar si la compilación tuvo éxito
if [ $? -eq 0 ]; then
    echo "Compilación exitosa."

    # Ejecutar el ejecutable (ajusta el nombre del ejecutable si es necesario)
    ./SR1
else
    echo "La compilación falló."
fi
