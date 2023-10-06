#include <SDL2/SDL.h>
#include <cmath>
#include <glm/geometric.hpp>
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>
#include "framebuffer.h"
#include "vertexGML.h"
#include "objSettings.h"
#include "uniform.h"
#include <unordered_map>
#include <random>
#include "render.h"

const int SCREEN_WIDTH = pantallaAncho;
const int SCREEN_HEIGHT = pantallaAlto;

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float scale;
    float life;
    glm::vec4 color; // Color de la partícula (RGBA)
};


using FragmentRenderFunction = std::function<void(Fragment&)>;

std::unordered_map<std::string, FragmentRenderFunction> renderFunctions;

void initializeRenderFunctions() {
    renderFunctions["sky"] = skyFragmentShader;
    renderFunctions["planet"] = planetFragmentShader;
    renderFunctions["moon"] = moonFragmentShader;
    renderFunctions["sun"] = sunFragmentShader;
    renderFunctions["planet2"] = planet2FragmentShader;
    renderFunctions["planet3"] = planet3FragmentShader;
    renderFunctions["ship"] = shipFragmentShader;
    // Agrega más funciones para otros objetos aquí si es necesario
}

struct RenderData {
    Uniform uniform{};
    bool changed = false;
    Uniform skyU{};
    Uniform shipU{};
    Uniform planet1U{};
    Uniform planet2U{};
    Uniform planet3U{};
    Uniform moon1U{};
    Uniform sunU{};
    std::vector<Facer> sphere;
    std::vector<Facer> ship;
    std::vector<Fragment> concurrentFragments;
    std::mutex zbufferMutex;
    glm::vec3 cameraOrientation{};
    glm::vec3 sunPosition{};
    glm::vec3 shipPos;
    float planetRotation1 = 0.0f;
    float planetRotation2 = 1.5f;
    float planetRotation3 = 0.0f;
    float bRotate = 0;
    bool running = true;
    float xRotate{};
    float yRotate{};
    glm::vec3 contraVector{};
    float angle{};
    float angley{};
    float moveShipX = 0;
    float moveShipZ = 0;
    float positionShip = 0;
    std::vector<Particle> particles;

    RenderData() {
        initializeRenderFunctions();
        loadSphereData();
        initializeCameraOrientation();
        sunPosition = glm::vec3(0, 0, 0);
    }

    void loadSphereData() {
        if (!loadOBJ("../sphere.obj", sphere)) {
            std::cout << "No funcionó" << std::endl;
        }
        if (!loadOBJ("../ship5.obj", ship)) {
            std::cout << "No funcionó" << std::endl;
        }

    }

    void initializeCameraOrientation() {
        xRotate = 180.0f;
        yRotate = 0.0f;
        orientation = glm::vec3(5.0f * sin(glm::radians(xRotate)) * cos(glm::radians(yRotate)), 5.0f * sin(glm::radians(yRotate)), -5.0f * cos(glm::radians(xRotate)) * cos(glm::radians(yRotate))) + cameraPosition;
    }

    void changeCamera(){
        if(!changed){
            cameraPosition = glm::vec3(0, 5, 0);
            orientation = glm::vec3(0,0,0);
            uper = glm::vec3(1, 0, 0);
            changed = !changed;
        }else{
            cameraPosition = glm::vec3(0, 0, 5);
            orientation = glm::vec3 (0, 0, 0);
            uper = glm::vec3(0, 1, 0);
            changed = !changed;
        }
    }

// ... (resto del código)

    void handleKeyPress(SDL_Keycode key) {
        // Variables para el desplazamiento y rotación
        float moveSpeed = 0.1f;
        float rotateSpeed = 4.0f;

        switch (key) {
            case SDLK_SPACE:
                cameraPosition += moveSpeed * uper * 5.0f;
                orientation += moveSpeed * uper* 5.0f;
                break;
            case SDLK_v:
                cameraPosition -= moveSpeed * uper * 5.0f;
                orientation -= moveSpeed * uper* 5.0f;
                break;
            case SDLK_d:
                // Mover hacia la derecha
                cameraPosition += moveSpeed * glm::normalize(glm::cross(cameraOrientation, uper)) * 5.0f;
                orientation += moveSpeed * glm::normalize(glm::cross(cameraOrientation, uper))* 5.0f;
                break;
            case SDLK_a:
                // Mover hacia la izquierda
                cameraPosition -= moveSpeed * glm::normalize(glm::cross(cameraOrientation, uper))* 5.0f;
                orientation -= moveSpeed * glm::normalize(glm::cross(cameraOrientation, uper))* 5.0f;
                break;
            case SDLK_w:
                // Mover hacia adelante
                cameraPosition += moveSpeed * cameraOrientation;
                orientation += moveSpeed * cameraOrientation;
                break;
            case SDLK_s:
                // Mover hacia atrás
                cameraPosition -= moveSpeed * cameraOrientation;
                orientation -= moveSpeed * cameraOrientation;
                break;
            case SDLK_LEFT:
                // Girar hacia la izquierda
                xRotate -= rotateSpeed;
                updateCameraOrientation();
                moveShipZ = -3.14f * 2.0f;
                break;
            case SDLK_RIGHT:
                // Girar hacia la derecha
                xRotate += rotateSpeed;
                updateCameraOrientation();
                moveShipZ = 3.14f * 2.0f;
                break;
            case SDLK_DOWN:
                // Mirar hacia abajo
                yRotate = glm::clamp(yRotate + rotateSpeed, -80.0f, 80.0f);
                updateCameraOrientation();
                moveShipX = 3.14f * 2.0f;
                break;
            case SDLK_UP:
                // Mirar hacia arriba
                yRotate = glm::clamp(yRotate - rotateSpeed, -80.0f, 80.0f);
                updateCameraOrientation();
                moveShipX = -3.14f * 2.0f;
                break;
            case SDLK_m:
                // Mirar hacia arriba
                changeCamera();
                break;
            default:
                break;
        }
    }


    void updateCameraOrientation() const {
        orientation = glm::vec3(5.0f * sin(glm::radians(xRotate)) * cos(glm::radians(yRotate)), 5.0f * sin(glm::radians(yRotate)), -5.0f * cos(glm::radians(xRotate)) * cos(glm::radians(yRotate))) + cameraPosition;
    }

    void renderFragment(const std::string& objectName, Fragment& fragment) {
        int index = (int) (fragment.position.y * pantallaAncho + fragment.position.x);
        if (!(fragment.position.x < 0 || fragment.position.x >= pantallaAncho || fragment.position.y < 0 || fragment.position.y >= pantallaAlto) &&
            fragment.position.z < zbuffer[index]) {
            if (fragment.position.z < zbuffer[index]) {
                auto it = renderFunctions.find(objectName);
                if (it != renderFunctions.end()) {
                    FragmentRenderFunction renderFunction = it->second;
                    renderFunction(fragment);
                }
                zbufferMutex.lock();
                framebuffer[index] = fragment.color;
                if (objectName != "sky") {
                    zbuffer[index] = fragment.position.z;
                }
                zbufferMutex.unlock();
            }
        }
    }


    void renderTriangles(const std::vector<Facer>& faces, const Uniform& shaderUniform, const std::string& objectName) {
        for (const Facer& face : faces) {
            if (face.vertexIndices.size() >= 3) {
                VertexGML a = face.vertexIndices[0];
                VertexGML b = face.vertexIndices[1];
                VertexGML c = face.vertexIndices[2];

                a = vertexShader(a, shaderUniform);
                b = vertexShader(b, shaderUniform);
                c = vertexShader(c, shaderUniform);

                if (( a.z <= 1e-10 || a.w<=1e-10) && ( b.z <= 1e-10 || b.w<=1e-10) && (c.z <= 1e-10|| c.w<=1e-10))
                    continue;

                if (!((a.position.x < 0 && b.position.x < 0 && c.position.x < 0) ||
                      (a.position.x >= pantallaAncho && b.position.x >= pantallaAncho && c.position.x >= pantallaAncho) ||
                      (a.position.y < 0 && b.position.y < 0 && c.position.y < 0) ||
                      (a.position.y >= pantallaAlto && b.position.y >= pantallaAlto && c.position.y >= pantallaAlto))) {

                    a.normal = glm::normalize(glm::mat3(shaderUniform.model) * a.normal);
                    b.normal = glm::normalize(glm::mat3(shaderUniform.model) * b.normal);
                    c.normal = glm::normalize(glm::mat3(shaderUniform.model) * c.normal);

                    std::vector<Fragment> rasterizedTriangle = triangle_F(a, b, c);

                    for (Fragment& fragment : rasterizedTriangle) {
                        renderFragment(objectName, fragment);
                    }
                }
            }
        }
    }


    void render(SDL_Renderer* renderer) {
        L = cameraPosition - orientation;
        concurrentFragments.clear();
        configPlanetNoiseGenerator();
        cameraOrientation = orientation - cameraPosition;
        glm::vec3 planet1Position = glm::vec3(-2.5f * std::cos(planetRotation1), 0, 2.5f * std::sin(planetRotation1)) + sunPosition;
        shipPos = cameraOrientation /8.5f + cameraPosition +uper*0.15f;
        glm::vec3 planet2Position = glm::vec3(-1.5f * std::cos(planetRotation2), 0, 1.5f * std::sin(planetRotation2)) + sunPosition;
        glm::vec3 planet3Position = glm::vec3(0,0,10);
        glm::vec3 moon1Position = glm::vec3(-0.4f * cos(glm::radians(bRotate)), 0, 0.4f * sin(glm::radians(bRotate))) + planet1Position;
        L2 = (glm::vec3(0, 0, 0) - planet1Position);
        L6 = (glm::vec3(0, 0, 0) - planet2Position);
        L3 = (moon1Position - planet1Position);
        L5 = (glm::vec3(0, 0, 0) - moon1Position);
        L4 = (planet1Position - moon1Position);

        sunPos = sunPosition - shipPos;


        uniform.projection = createProjectionMatrix();
        uniform.view = createViewMatrix();
        uniform.viewport = createViewportMatrix();

        sunU.model = createModelMatrixPlanet(sunPosition, 2.0f, bRotate);
        sunU.view = uniform.view;
        sunU.viewport = uniform.viewport;
        sunU.projection = uniform.projection;

        shipU.model = createModelMatrixPlanet(shipPos, 0.1f, -xRotate, moveShipX + yRotate, moveShipZ);
        shipU.view = uniform.view;
        shipU.viewport = uniform.viewport;
        shipU.projection = uniform.projection;

        planet1U.model = createModelMatrixPlanet(planet1Position, 0.4f, bRotate);
        planet1U.view = uniform.view;
        planet1U.viewport = uniform.viewport;
        planet1U.projection = uniform.projection;

        planet2U.model = createModelMatrixPlanet(planet2Position, 0.25f, bRotate);
        planet2U.view = uniform.view;
        planet2U.viewport = uniform.viewport;
        planet2U.projection = uniform.projection;

        planet3U.model = createModelMatrixPlanet(planet3Position, 1.0f, bRotate);
        planet3U.view = uniform.view;
        planet3U.viewport = uniform.viewport;
        planet3U.projection = uniform.projection;

        moon1U.model = createModelMatrixPlanet(moon1Position, 0.2f, bRotate);
        moon1U.view = uniform.view;
        moon1U.viewport = uniform.viewport;
        moon1U.projection = uniform.projection;

        skyU.model = createModelMatrixPlanet((orientation-cameraPosition)*0.2f + cameraPosition, 3.0f, -xRotate);
        skyU.view = uniform.view;
        skyU.viewport = uniform.viewport;
        skyU.projection = uniform.projection;
        configSunNoiseGenerator();

        std::vector<std::thread> sunThreads;
        std::vector<std::thread> skyblockThreads;
        const size_t numThreads = std::thread::hardware_concurrency();

        std::vector<std::vector<Facer>> threadWork(numThreads);
        std::vector<std::vector<Facer>> threadWorkShip(numThreads);

        for (size_t i = 0; i < sphere.size(); ++i) {
            threadWork[i % numThreads].push_back(sphere[i]);
        }

        for (size_t i = 0; i < ship.size(); ++i) {
            threadWorkShip[i % numThreads].push_back(ship[i]);
        }

       // renderBackground();

        /*for (size_t i = 0; i < numThreads; ++i) {
            skyblockThreads.emplace_back(&RenderData::renderTriangles, this, std::ref(threadWork[i]), std::ref(skyU), "sky");
        }

        for (std::thread& t : skyblockThreads) {
            t.join();
        }*/

        for (size_t i = 0; i < numThreads; ++i) {
            sunThreads.emplace_back(&RenderData::renderTriangles, this,
                                    std::ref(threadWork[i]),
                                    std::ref(sunU), "sun");
            sunThreads.emplace_back(&RenderData::renderTriangles, this,
                                    std::ref(threadWork[i]),
                                    std::ref(planet1U), "planet");
            sunThreads.emplace_back(&RenderData::renderTriangles, this,
                                    std::ref(threadWork[i]),
                                    std::ref(planet2U), "planet2");
            sunThreads.emplace_back(&RenderData::renderTriangles, this,
                                    std::ref(threadWork[i]),
                                    std::ref(planet3U), "planet3");
            sunThreads.emplace_back(&RenderData::renderTriangles, this,
                                    std::ref(threadWork[i]),
                                    std::ref(moon1U), "moon");
            sunThreads.emplace_back(&RenderData::renderTriangles, this,
                                    std::ref(threadWorkShip[i]),
                                    std::ref(shipU), "ship");
        }

        for (std::thread& t : sunThreads) {
            t.join();
        }

        renderBuffer(renderer);
    }

};

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("Planetoide", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED
    );

    RenderData renderData;
    Uint32 frameStart = SDL_GetTicks();;      // Tiempo de inicio del cuadro actual
    Uint32 frameTime;       // Tiempo transcurrido en el cuadro actual
    int frameCount = 0;     // Contador de cuadros renderizados
    int fps = 0;

    while (renderData.running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            renderData.moveShipZ = 0.0f;
            renderData.moveShipX = 0.0f;
            switch (event.type) {
                case SDL_QUIT:
                    renderData.running = false;
                    break;
                case SDL_KEYDOWN:
                    renderData.handleKeyPress(event.key.keysym.sym);
                    break;
            }
        }

        clear();
        renderData.render(renderer);
        SDL_RenderPresent(renderer);
        renderData.planetRotation1 += 0.02f;
        renderData.planetRotation2 += 0.01f;
        frameTime = SDL_GetTicks() - frameStart;
        frameCount++;
        if (frameTime >= 1000) {
            fps = frameCount;
            frameCount = 0;
            frameStart = SDL_GetTicks(); // Reinicia el tiempo de inicio para el siguiente segundo
        }
        std::string fpsText = "FPS: " + std::to_string(fps);
        SDL_SetWindowTitle(window, fpsText.c_str());
        renderData.bRotate++;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
