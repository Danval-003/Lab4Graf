#include "framebuffer.h"
#include <fstream>

std::array<Color, pantallaAncho * pantallaAlto> framebuffer;
std::array<float, pantallaAncho * pantallaAlto> zbuffer;
Color clearColor(0, 0, 0);
Color currentColor(255, 255, 255);


#include <thread>
#include <vector>

void clear() {
    const unsigned int numThreads = std::thread::hardware_concurrency();

    auto clearPortion = [](unsigned int startY, unsigned int endY) {
        for (unsigned int y = startY; y < endY; y++) {
            unsigned int startPixelIndex = y * pantallaAncho;
            unsigned int endPixelIndex = startPixelIndex + pantallaAncho;

            for (unsigned int pixelIndex = startPixelIndex; pixelIndex < endPixelIndex; pixelIndex++) {
                framebuffer[pixelIndex] = clearColor;
            }
        }
    };

    std::vector<std::thread> threads;

    const unsigned int rowsPerThread = pantallaAlto / numThreads;
    unsigned int startY = 0;
    unsigned int endY = rowsPerThread;

    for (unsigned int i = 0; i < numThreads - 1; i++) {
        threads.emplace_back(clearPortion, startY, endY);
        startY = endY;
        endY += rowsPerThread;
    }

    threads.emplace_back(clearPortion, startY, pantallaAlto);

    for (auto& thread : threads) {
        thread.join();
    }

    for (int i = 0; i < pantallaAncho*pantallaAlto; ++i) {
        zbuffer[i] = 99999.0f;
    }
}

void clear(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);


    for (int i = 0; i < pantallaAncho*pantallaAlto; ++i) {
        zbuffer[i] = 99999.0f;
    }
}

void setCurrentColor(Color changeColor) {
    currentColor = changeColor;
}

void setClearColor(Color changeColor) {
    clearColor = changeColor;
}

