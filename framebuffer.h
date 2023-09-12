#pragma once

#include <array>
#include "color.h"

// Tamaño de la pantalla
const int pantallaAncho = 800;
const int pantallaAlto = 800;

// Variable global: framebuffer
extern std::array<Color, pantallaAncho * pantallaAlto> framebuffer;
extern std::array<float, pantallaAncho * pantallaAlto> zbuffer;

// Variable global: clearColor
extern Color clearColor;

// Función para limpiar el framebuffer con el color clearColor
void clear();

extern Color currentColor;  // Declaración de la variable currentColor

void setCurrentColor(Color changeColor);

void setClearColor(Color changeColor);

void point(int x, int y);

bool getCellState(int x, int y);

void clear(SDL_Renderer* renderer);
