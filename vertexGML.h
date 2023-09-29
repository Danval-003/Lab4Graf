#pragma once
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include "color.h"
#include "uniform.h"

extern float timeAni;
extern glm::vec3 L;
extern glm::vec3 L2;
extern glm::vec3 L3;
extern glm::vec3 L4;
extern glm::vec3 L5;
extern bool moonToPlanet;
extern float multiplier;

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
    double z;
};

struct Facer {
    std::vector<VertexGML> vertexIndices;
    Facer(): vertexIndices(){};
    Facer(std::vector<VertexGML> temp_vertex): vertexIndices(temp_vertex){};
};

struct Fragment {
  glm::vec3 position;
  Color color;
  float intensity;
  glm::vec3 original;
  glm::vec3 normal;
};
std::vector<Fragment> triangle_F(VertexGML a, VertexGML b, VertexGML c);
VertexGML vertexShader(const VertexGML& vertex, const Uniform& u);
void sunFragmentShader(Fragment& fragment);
void skyFragmentShader(Fragment& fragment);
void configSunNoiseGenerator();
void planetFragmentShader(Fragment& fragment);
void planet2FragmentShader(Fragment& fragment);
void configPlanetNoiseGenerator();
void moonFragmentShader(Fragment& fragment);
void shipFragmentShader(Fragment& fragment);
Color skyFragmentShader2(glm::vec3& pos, glm::vec3& offset );


