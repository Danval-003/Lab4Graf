#include <SDL2/SDL.h>
#include <glm/geometric.hpp>
#include <iostream>
#include <glm/glm.hpp>
#include <ostream>
#include <vector>
#include "color.h"
#include "framebuffer.h"
#include "vertexGML.h"
#include "objSettings.h"
#include "uniform.h"
#include <random>
#include <thread>
#include <mutex>
#include "render.h"
#include <glm/gtx/vector_angle.hpp>

using namespace std;

// Dimensiones de la ventana
const int SCREEN_WIDTH = pantallaAncho;
const int SCREEN_HEIGHT = pantallaAlto;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Software Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

Uniform uniform;
Uniform skyU;

std::vector<VertexGML> vertices_out;
std::vector<Facer> sphere;

std::vector<Facer> sun;
std::vector<Facer> skyblock;
std::vector<Facer> planet1;
std::vector<Facer> moon1;
std::vector<Fragment> concurrentFragments;
std::mutex zbufferMutex;
glm::vec3 cameraOrientation;

bool isInFrontOfCamera(glm::vec3 pointToCheck){
// Vector que apunta desde la cámara al punto
    glm::vec3 vectorToPoint = pointToCheck - cameraPosition;

// Calcula el ángulo entre los dos vectores
    float angle = glm::angle(cameraOrientation, vectorToPoint);

// Si el ángulo es mayor a 90 grados, el punto está detrás de la cámara
    if (angle > glm::radians(90.0f)) {
        return false;
    } else {
        return true;
    }
}

void renderSkyFragment(Fragment& fragment) {
    if (!(fragment.position.x < 0 || fragment.position.x >= pantallaAncho || fragment.position.y < 0 || fragment.position.y >= pantallaAlto) &&
        fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x]) {
        if (fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x] &&
            isInFrontOfCamera(fragment.position)) {
            skyFragmentShader(fragment);
            zbufferMutex.lock();
            framebuffer[fragment.position.y * pantallaAncho + fragment.position.x] = fragment.color;
            zbufferMutex.unlock();
        }
    }
}

void renderPlanetFragment(Fragment& fragment) {
    if (!(fragment.position.x < 0 || fragment.position.x >= pantallaAncho || fragment.position.y < 0 || fragment.position.y >= pantallaAlto) &&
        fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x]) {
        if (fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x]) {
            planetFragmentShader(fragment);
            zbufferMutex.lock();
            framebuffer[fragment.position.y * pantallaAncho + fragment.position.x] = fragment.color;
            zbuffer[fragment.position.y * pantallaAncho + fragment.position.x] = fragment.position.z;
            zbufferMutex.unlock();
        }
    }
}

void renderMoonFragment(Fragment& fragment) {
    if (!(fragment.position.x < 0 || fragment.position.x >= pantallaAncho || fragment.position.y < 0 || fragment.position.y >= pantallaAlto)) {
        if (fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x]) {
            moonFragmentShader(fragment);
            zbufferMutex.lock();
            framebuffer[fragment.position.y * pantallaAncho + fragment.position.x] = fragment.color;
            zbuffer[fragment.position.y * pantallaAncho + fragment.position.x] = fragment.position.z;
            zbufferMutex.unlock();
        }
    }
}

void renderSunFragment(Fragment& fragment) {
    if (!(fragment.position.x < 0 || fragment.position.x >= pantallaAncho || fragment.position.y < 0 || fragment.position.y >= pantallaAlto) &&
        fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x]) {
        if (fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x]) {
            sunFragmentShader(fragment);
            zbufferMutex.lock();
            framebuffer[fragment.position.y * pantallaAncho + fragment.position.x] = fragment.color;
            zbuffer[fragment.position.y * pantallaAncho + fragment.position.x] = fragment.position.z;
            zbufferMutex.unlock();
        }
    }
}

// ...

void renderSkyTriangles(const std::vector<Facer>& faces, const Uniform& shaderUniform) {
    for (const Facer& face : faces) {
        if (face.vertexIndices.size() >= 3) {
            VertexGML temA = face.vertexIndices[0];
            VertexGML temB = face.vertexIndices[1];
            VertexGML temC = face.vertexIndices[2];

            VertexGML a = vertexShader(temA, shaderUniform);
            VertexGML b = vertexShader(temB, shaderUniform);
            VertexGML c = vertexShader(temC, shaderUniform);

            if (!((a.position.x < 0 && b.position.x < 0 && c.position.x < 0) ||
                  (a.position.x >= pantallaAncho && b.position.x >= pantallaAncho && c.position.x >= pantallaAncho) ||
                  (a.position.y < 0 && b.position.y < 0 && c.position.y < 0) ||
                  (a.position.y >= pantallaAlto && b.position.y >= pantallaAlto && c.position.y >= pantallaAlto))) {

                a.normal = glm::normalize(glm::mat3(shaderUniform.model) * a.normal);
                b.normal = glm::normalize(glm::mat3(shaderUniform.model) * b.normal);
                c.normal = glm::normalize(glm::mat3(shaderUniform.model) * c.normal);

                std::vector<Fragment> rasterizedTriangle = triangle_F(a, b, c);

                for (Fragment& fragment : rasterizedTriangle) {
                    renderSkyFragment(fragment);
                }
            }
        }
    }
}

void renderSunTriangles(const std::vector<Facer>& faces, const Uniform& shaderUniform) {
    for (const Facer& face : faces) {
        if (face.vertexIndices.size() >= 3) {
            VertexGML temA = face.vertexIndices[0];
            VertexGML temB = face.vertexIndices[1];
            VertexGML temC = face.vertexIndices[2];

            VertexGML a = vertexShader(temA, shaderUniform);
            VertexGML b = vertexShader(temB, shaderUniform);
            VertexGML c = vertexShader(temC, shaderUniform);

            if (!((a.position.x < 0 && b.position.x < 0 && c.position.x < 0) ||
                  (a.position.x >= pantallaAncho && b.position.x >= pantallaAncho && c.position.x >= pantallaAncho) ||
                  (a.position.y < 0 && b.position.y < 0 && c.position.y < 0) ||
                  (a.position.y >= pantallaAlto && b.position.y >= pantallaAlto && c.position.y >= pantallaAlto))) {

                a.normal = glm::normalize(glm::mat3(shaderUniform.model) * a.normal);
                b.normal = glm::normalize(glm::mat3(shaderUniform.model) * b.normal);
                c.normal = glm::normalize(glm::mat3(shaderUniform.model) * c.normal);

                std::vector<Fragment> rasterizedTriangle = triangle_F(a, b, c);

                for (Fragment& fragment : rasterizedTriangle) {
                    renderSunFragment(fragment);
                }
            }
        }
    }
}

void renderPlanetTriangles(const std::vector<Facer>& faces, const Uniform& shaderUniform) {
    for (const Facer& face : faces) {
        if (face.vertexIndices.size() >= 3) {
            VertexGML temA = face.vertexIndices[0];
            VertexGML temB = face.vertexIndices[1];
            VertexGML temC = face.vertexIndices[2];

            VertexGML a = vertexShader(temA, shaderUniform);
            VertexGML b = vertexShader(temB, shaderUniform);
            VertexGML c = vertexShader(temC, shaderUniform);

            if (!((a.position.x < 0 && b.position.x < 0 && c.position.x < 0) ||
                  (a.position.x >= pantallaAncho && b.position.x >= pantallaAncho && c.position.x >= pantallaAncho) ||
                  (a.position.y < 0 && b.position.y < 0 && c.position.y < 0) ||
                  (a.position.y >= pantallaAlto && b.position.y >= pantallaAlto && c.position.y >= pantallaAlto))) {

                a.normal = glm::normalize(glm::mat3(shaderUniform.model) * a.normal);
                b.normal = glm::normalize(glm::mat3(shaderUniform.model) * b.normal);
                c.normal = glm::normalize(glm::mat3(shaderUniform.model) * c.normal);

                std::vector<Fragment> rasterizedTriangle = triangle_F(a, b, c);

                for (Fragment& fragment : rasterizedTriangle) {
                    renderPlanetFragment(fragment);
                }
            }
        }
    }
}

void renderMoonTriangles(const std::vector<Facer>& faces, const Uniform& shaderUniform) {
    for (const Facer& face : faces) {
        if (face.vertexIndices.size() >= 3) {
            VertexGML temA = face.vertexIndices[0];
            VertexGML temB = face.vertexIndices[1];
            VertexGML temC = face.vertexIndices[2];

            VertexGML a = vertexShader(temA, shaderUniform);
            VertexGML b = vertexShader(temB, shaderUniform);
            VertexGML c = vertexShader(temC, shaderUniform);

            if (!((a.position.x < 0 && b.position.x < 0 && c.position.x < 0) ||
                  (a.position.x >= pantallaAncho && b.position.x >= pantallaAncho && c.position.x >= pantallaAncho) ||
                  (a.position.y < 0 && b.position.y < 0 && c.position.y < 0) ||
                  (a.position.y >= pantallaAlto && b.position.y >= pantallaAlto && c.position.y >= pantallaAlto))) {

                a.normal = glm::normalize(glm::mat3(shaderUniform.model) * a.normal);
                b.normal = glm::normalize(glm::mat3(shaderUniform.model) * b.normal);
                c.normal = glm::normalize(glm::mat3(shaderUniform.model) * c.normal);

                std::vector<Fragment> rasterizedTriangle = triangle_F(a, b, c);

                for (Fragment& fragment : rasterizedTriangle) {
                    renderMoonFragment(fragment);
                }
            }
        }
    }
}

Uniform planet1U;
Uniform moon1U;

glm::vec3 sunPosition = glm::vec3(0,0,0);

void render(SDL_Renderer* renderer) {
    concurrentFragments.clear();
    configPlanetNoiseGenerator();
    // Vector de orientación de la cámara (de la cámara al punto de mira)
    cameraOrientation = orientation - cameraPosition;
    glm::vec3 planet1Position = glm::vec3(-1.5f * cos(timeAni), 0, 1.5f * sin(timeAni)) + sunPosition;
    glm::vec3 moon1Position = glm::vec3(-0.4f * cos(glm::radians(b)), 0, 0.4f * sin(glm::radians(b))) + planet1Position;
    L2 = (glm::vec3(0,0,0) - planet1Position);
    L3 = (moon1Position - planet1Position);
    L5 = (glm::vec3(0,0,0) - moon1Position);
    L4 = (planet1Position - moon1Position);

    planet1U.model = createModelMatrixPlanet(planet1Position, 0.4f);
    planet1U.projection = createProjectionMatrix();
    planet1U.view = createViewMatrix();
    planet1U.viewport = createViewportMatrix();

    moon1U.model = createModelMatrixPlanet(moon1Position, 0.2f);
    moon1U.projection = createProjectionMatrix();
    moon1U.view = createViewMatrix();
    moon1U.viewport = createViewportMatrix();

    uniform.model = createModelMatrixPlanet(sunPosition, 1.5f);
    uniform.projection = createProjectionMatrix();
    uniform.view = createViewMatrix();
    uniform.viewport = createViewportMatrix();

    skyU.model = createModelMatrix(glm::vec3(0.0f, 0.0f, 0.0f), 10.0f);
    skyU.projection = createProjectionMatrix();
    skyU.viewport = createViewportMatrix();
    skyU.view = createViewMatrix();
    configSunNoiseGenerator();

    std::vector<std::thread> sunThreads;
    std::vector<std::thread> skyblockThreads;

    // Divide el trabajo en partes iguales para los hilos
    const size_t numThreads = std::thread::hardware_concurrency(); // Obtén el número de núcleos de la CPU
    std::vector<std::vector<Facer>> threadWork(numThreads);

    for (size_t i = 0; i < skyblock.size(); ++i) {
        threadWork[i % numThreads].push_back(skyblock[i]);
    }

    // Inicia hilos para procesar los triángulos
    for (size_t i = 0; i < numThreads; ++i) {
        skyblockThreads.emplace_back(renderSkyTriangles, std::ref(threadWork[i]), std::ref(skyU));
    }

    for (std::thread& t : skyblockThreads) {
        t.join();
    }

    for (size_t i = 0; i < numThreads; ++i) {
        sunThreads.emplace_back(renderSunTriangles, std::ref(threadWork[i]), std::ref(uniform));
        sunThreads.emplace_back(renderPlanetTriangles, std::ref(threadWork[i]), std::ref(planet1U));
        sunThreads.emplace_back(renderMoonTriangles, std::ref(threadWork[i]), std::ref(moon1U));
    }
    

    for (std::thread& t : sunThreads) {
        t.join();
    }



    renderBuffer(renderer);
}



int main(int argc, char* argv[]) {

    if (!loadOBJ("../sphere.obj", sphere)) {
        cout << "No funcionó" << endl;
    }

    for (Facer face : sphere) {
        skyblock.push_back(face);
    }

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("Planetoide", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED
    );

    bool running = true;
    SDL_Event event;
    // Clear the framebuffer
    Uint32 frameStart, frameTime;

    while (running) {
        while (SDL_PollEvent(&event)) {

            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.sym == SDLK_w) {
                        cameraPosition.y += 0.000001f;
                    }
                    else if (event.key.keysym.sym == SDLK_s) {
                        cameraPosition.y -= 0.000001f;
                    }
                    else if (event.key.keysym.sym == SDLK_d) {
                        cameraPosition.x += 0.000001f;
                    }
                    else if (event.key.keysym.sym == SDLK_a) {
                        cameraPosition.x -= 0.000001f;
                    }
                    break;
            }
        }

        clear();

        // Call our render function
        render(renderer);

        // Present the frame buffer to the screen
        SDL_RenderPresent(renderer);
        timeAni += 0.05f;
        frameTime = SDL_GetTicks() - frameStart;

        // Calculate frames per second and update window title
        if (frameTime > 0) {
            std::ostringstream titleStream;
            titleStream << "FPS: " << 1000.0 / frameTime;  // Milliseconds to seconds
            SDL_SetWindowTitle(window, titleStream.str().c_str());
        }
        b++;
        if(!moonToPlanet && multiplier < 0){
            multiplier+=0.1f;
        } else if (moonToPlanet && multiplier > 1.0f){
            multiplier-=0.1f;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
