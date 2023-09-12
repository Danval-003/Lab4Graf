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
#include <tbb/tbb.h>
#include <mutex>
#include "render.h"


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
tbb::concurrent_vector<Fragment> concurrentFragments;
std::mutex zbufferMutex;


void render(SDL_Renderer* renderer) {
    concurrentFragments.clear();

    configureUniform(&uniform);

    skyU.model = createModelMatrix(glm::vec3(0.0f, 0.0f, 0.0f), 10.0f);
    skyU.projection = createProjectionMatrix();
    skyU.viewport = createViewportMatrix();
    skyU.view = createViewMatrix();
    configSunNoiseGenerator();

    tbb::parallel_for_each(skyblock.begin(), skyblock.end(), [&](const Facer& face) {
        if (face.vertexIndices.size() >= 3) {
            VertexGML temA = face.vertexIndices[0];
            VertexGML temB = face.vertexIndices[1];
            VertexGML temC = face.vertexIndices[2];

            VertexGML a = vertexShader(temA, skyU);
            VertexGML b = vertexShader(temB, skyU);
            VertexGML c = vertexShader(temC,skyU);

            if(!((a.position.x<0 && b.position.x<0 && c.position.x<0)||(a.position.x>=pantallaAncho && b.position.x>=pantallaAncho && c.position.x>=pantallaAncho)||(a.position.y<0 && b.position.y<0 && c.position.y<0)||(a.position.y>=pantallaAlto && b.position.y>=pantallaAlto && c.position.y>=pantallaAlto))){

                a.normal = glm::normalize(glm::mat3(skyU.model) * a.normal);
                b.normal = glm::normalize(glm::mat3(skyU.model) * b.normal);
                c.normal = glm::normalize(glm::mat3(skyU.model) * c.normal);


                std::vector<Fragment> rasterizedTriangle = triangle_F(
                    a,
                    b,
                    c);

                for (Fragment& fragment: rasterizedTriangle) {
                    if(!(fragment.position.x<0 || fragment.position.x>=pantallaAncho|| fragment.position.y<0 || fragment.position.y>= pantallaAlto) && fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x]){
                        if (fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x]){
                            skyFragmentShader(fragment);
                            zbufferMutex.lock();
                            framebuffer[fragment.position.y * pantallaAncho + fragment.position.x] = fragment.color;
                            zbufferMutex.unlock();
                        }
                    }
                }
            }


        }
    });

    tbb::parallel_for_each(sun.begin(), sun.end(), [&](const Facer& face) {
        if (face.vertexIndices.size() >= 3) {
            VertexGML temA = face.vertexIndices[0];
            VertexGML temB = face.vertexIndices[1];
            VertexGML temC = face.vertexIndices[2];

            VertexGML a = vertexShader(temA, uniform);
            VertexGML b = vertexShader(temB, uniform);
            VertexGML c = vertexShader(temC,uniform);

            if(!((a.position.x<0 && b.position.x<0 && c.position.x<0)||(a.position.x>=pantallaAncho && b.position.x>=pantallaAncho && c.position.x>=pantallaAncho)||(a.position.y<0 && b.position.y<0 && c.position.y<0)||(a.position.y>=pantallaAlto && b.position.y>=pantallaAlto && c.position.y>=pantallaAlto))){

                a.normal = glm::normalize(glm::mat3(skyU.model) * a.normal);
                b.normal = glm::normalize(glm::mat3(skyU.model) * b.normal);
                c.normal = glm::normalize(glm::mat3(skyU.model) * c.normal);


                std::vector<Fragment> rasterizedTriangle = triangle_F(
                    a,
                    b,
                    c);

                for (Fragment& fragment: rasterizedTriangle) {
                    if(!(fragment.position.x<0 || fragment.position.x>=pantallaAncho|| fragment.position.y<0 || fragment.position.y>= pantallaAlto) && fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x]){
                        if (fragment.position.z < zbuffer[fragment.position.y * pantallaAncho + fragment.position.x]){
                            sunFragmentShader(fragment);
                            zbufferMutex.lock();
                            framebuffer[fragment.position.y * pantallaAncho + fragment.position.x] = fragment.color;
                            zbuffer[fragment.position.y * pantallaAncho + fragment.position.x] = fragment.position.z;
                            zbufferMutex.unlock();
                        }
                    }
                }
            }


        }
    });

    renderBuffer(renderer);

}


int main(int argc, char* argv[]) {

    if(!loadOBJ("../sphere.obj", sphere)){

        cout<<"No funciono"<<endl;
    }

    for (Facer face : sphere) {
        sun.push_back(face);
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
                        cameraPosition.z += 0.000001f;
                    }
                    else if (event.key.keysym.sym == SDLK_a) {
                        cameraPosition.z -= 0.000001f;
                    }
                    break;
            }
        }

        clear();

        // Call our render function
        render(renderer);

        // Present the frame buffer to the screen
        SDL_RenderPresent(renderer);
        timeAni += 0.1f;
        frameTime = SDL_GetTicks() - frameStart;

        // Calculate frames per second and update window title
        if (frameTime > 0) {
            std::ostringstream titleStream;
            titleStream << "FPS: " << 1000.0 / frameTime;  // Milliseconds to seconds
            SDL_SetWindowTitle(window, titleStream.str().c_str());
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
