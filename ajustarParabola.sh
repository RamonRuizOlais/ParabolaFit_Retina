#!/bin/bash

# Compilar el código C++ si es necesario
g++ parabola.cpp -o parabola_cpp

# Ejecutar el código C++
./parabola_cpp

# Ejecutar el código Python
python3 graficar.py
