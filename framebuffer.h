#pragma once

#include <array>
#include "color.h"

// Tamaño de la pantalla
const int pantallaAncho = 400;
const int pantallaAlto = 400;

// Variable global: framebuffer
extern std::array<Color, pantallaAncho * pantallaAlto> framebuffer;
extern std::array<float, pantallaAncho * pantallaAlto> zbuffer;

// Variable global: clearColor
extern Color clearColor;

// Función para limpiar el framebuffer con el color clearColor
void clear();


