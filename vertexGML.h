#pragma once
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include "color.h"
#include "uniform.h"

extern float timeAni;

struct Face {
    std::vector<int> vertexIndices;

    Face(): vertexIndices(){};
    Face(std::vector<int> temp_vertex): vertexIndices(temp_vertex){};
};



// En vertexGML.h, cambia la definición de la estructura VertexGML
struct VertexGML {
    glm::vec3 position;
    glm::vec3 original;
    Color color;
    glm::vec3 normal;
    glm::vec2 texture;
};

struct Facer {
    std::vector<VertexGML> vertexIndices;
    Facer(): vertexIndices(){};
    Facer(std::vector<VertexGML> temp_vertex): vertexIndices(temp_vertex){};
};

struct Fragment {
  glm::vec3 position;
  Color color;
  float intensity = 1.0f;
  glm::vec3 original;
};
std::vector<Fragment> triangle_F(VertexGML a, VertexGML b, VertexGML c);
VertexGML vertexShader(const VertexGML& vertex, const Uniform& u);
void sunFragmentShader(Fragment& fragment);
void skyFragmentShader(Fragment& fragment);
void configSunNoiseGenerator();
std::vector<Fragment> triangulateAndDrawCube(std::vector<Facer> Faces, SDL_Renderer* renderer);


