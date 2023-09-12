#include "color.h"
#include "framebuffer.h"
#include "vertexGML.h"
#include "uniform.h"
#include "FastNoiseLite.h"
#include <SDL2/SDL_timer.h>
#include <algorithm>
#include <cmath>
#include <glm/common.hpp>
#include <iostream>
#include <random>
#include <tbb/tbb.h>
#include <mutex>

glm::vec3 L = glm::vec3(0, 0, 1.0f);

#include <glm/gtc/matrix_transform.hpp>

std::pair<float, float> barycentricCoordinates(const glm::ivec2& P, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    glm::vec3 bary = glm::cross(
        glm::vec3(C.x - A.x, B.x - A.x, A.x - P.x),
        glm::vec3(C.y - A.y, B.y - A.y, A.y - P.y)
    );

    if (abs(bary.z) < 1) {
        return std::make_pair(-1, -1);
    }

    return std::make_pair(
        bary.y / bary.z,
        bary.x / bary.z
    );
}


std::vector<Fragment> triangle_F(VertexGML a, VertexGML b, VertexGML c) {
    glm::vec3 A = a.position;
    glm::vec3 B = b.position;
    glm::vec3 C = c.position;

    std::vector<Fragment> triangleFragments;


    // build bounding box

    int minX = std::min(std::min(A.x, B.x), C.x);
    int minY = std::min(std::min(A.y, B.y), C.y);
    int maxX = std::max(std::max(A.x, B.x), C.x);
    int maxY = std::max(std::max(A.y, B.y), C.y);

    for (int y = minY; y <= maxY; y++) {
      for (int x = minX; x <= maxX; x++) {
        if (x < 0 || y < 0 || y > pantallaAlto-1 || x > pantallaAncho-1)
          continue;

        glm::ivec2 P(x, y);
        auto barycentric = barycentricCoordinates(P, A, B, C);
        float w = 1 - barycentric.first - barycentric.second;
        float v = barycentric.first;
        float u = barycentric.second;
        float epsilon = 1e-10;

        if (w < epsilon || v < epsilon || u < epsilon)
          continue;

        double z = A.z * w + B.z * v + C.z * u;

        if (
          w <= 1 && w >= 0 &&
          v <= 1 && v >= 0 &&
          u <= 1 && u >= 0
        ) {
          glm::vec3 normal = a.normal * w + b.normal * v + c.normal * u;
          glm::vec3 original = a.original * w + b.original * v + c.original * u;
          Color color = a.color * w + b.color * v + c.color * u;

          float intensity = glm::dot(normal, L);
          if (intensity< 0) {
              continue;
            }

          triangleFragments.push_back(
            Fragment{glm::vec3(P.x, P.y, z), color * intensity, intensity, original}
          );
        }
      }
    }

    return triangleFragments;
}



VertexGML vertexShader(const VertexGML& vertex, const Uniform& u) {

  glm::vec4 v = glm::vec4(vertex.position.x, vertex.position.y, vertex.position.z, 1);

  glm::vec4 r = u.viewport * u.projection * u.view * u.model * v;

  return VertexGML{
    glm::vec3( r.x/r.w , r.y/r.w, r.z/r.w),
    vertex.position,
    vertex.color,
    vertex.normal,
    vertex.texture
  };
};

float timeAni = 0.0f;

FastNoiseLite sunNoiseGenerator;
float ox;
float oy;
float zoom;
glm::vec3 orangeSun = glm::vec3(255.0f/255.0f,160.0f/255.0f, 0.0f/255.0f);
glm::vec3 yellowSun = glm::vec3(200.0f/255.0f,0.0f/255.0f, 0.0f/255.0f);
glm::vec3 iluminationSun = glm::vec3(1.0f, 110.0f/255.0f, 0.0f);

void sunFragmentShader(Fragment& fragment) {
    glm::vec3 uv = glm::vec3(fragment.original.x, fragment.original.y, fragment.original.z) ;
    /* glm::vec2 uv = glm::vec2(fragment.texCoords.x, fragment.texCoords.y) */;
    float noiseValue = abs(sunNoiseGenerator.GetNoise((uv.x + ox) * zoom, (uv.y + oy) * zoom, uv.z * zoom)) ;

    glm::vec3 tmpColor =  glm::mix(yellowSun,orangeSun,(1-noiseValue));
    tmpColor = glm::mix(iluminationSun,tmpColor, fragment.intensity);


    fragment.color = Color(tmpColor.x, tmpColor.y, tmpColor.z);
}

void configSunNoiseGenerator(){

    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    noiseGenerator.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Div);
    noiseGenerator.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    noiseGenerator.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    noiseGenerator.SetRotationType3D(FastNoiseLite::RotationType3D_ImproveXYPlanes);
    noiseGenerator.SetSeed(1337);
    noiseGenerator.SetFrequency(0.03f+ sin(timeAni)*0.00000005f);
    noiseGenerator.SetCellularJitter(0.9f - sin(timeAni)*0.1f);
    noiseGenerator.SetDomainWarpAmp(160.0f);

    ox = 3000.0f+ sin(timeAni)*0.02f;
    oy =3000.0f+ sin(timeAni*0.3f)*0.02f;
    zoom =1300.0f;

    sunNoiseGenerator = noiseGenerator;
}

const double starDensity = 0.0005; // Mayor densidad = más estrellas


uint32_t fnv1aHash(float x, float y, float z) {
    uint32_t hash = 2166136261u; // Valor inicial del hash
    const uint32_t prime = 16777619u; // Factor primo

    // Combina las coordenadas en el hash
    hash ^= *(reinterpret_cast<const uint32_t*>(&x));
    hash ^= *(reinterpret_cast<const uint32_t*>(&y));
    hash ^= *(reinterpret_cast<const uint32_t*>(&z));

    // Aplica la multiplicación por el factor primo
    hash *= prime;

    return hash;
}

void skyFragmentShader(Fragment& fragment) {

    uint32_t hash = fnv1aHash(fragment.original.x, fragment.original.y, fragment.original.z);
    float normalizedHash = static_cast<float>(hash) / static_cast<float>(UINT32_MAX);

    //glm::vec3 tmpColor = (noiseValue < 0.6f) ? oceanColor : groundColor;
    Color color = (normalizedHash < starDensity) ? WHITE: BLACK;

    fragment.color = color;
}


std::mutex fragments_mutex;

std::vector<Fragment> triangulateAndDrawCube(std::vector<Facer> Faces, SDL_Renderer* renderer) {
    std::vector<Fragment> fragments;

    tbb::parallel_for_each(Faces.begin(), Faces.end(), [&](const Facer& face) {
        if (face.vertexIndices.size() >= 3) {
            // Triangular la cara utilizando los vértices A, B y C
          bool inX = face.vertexIndices[0].position.x<0 && face.vertexIndices[1].position.x<0 && face.vertexIndices[2].position.x<0;
          bool inY = face.vertexIndices[0].position.y<0 && face.vertexIndices[1].position.y<0 && face.vertexIndices[2].position.y<0;
          bool onY = face.vertexIndices[0].position.y<pantallaAlto-1 && face.vertexIndices[1].position.y<pantallaAlto-1 && face.vertexIndices[2].position.y<pantallaAlto-1;
          bool onX = face.vertexIndices[0].position.x<pantallaAncho-1 && face.vertexIndices[1].position.x<pantallaAncho-1 && face.vertexIndices[2].position.x<pantallaAncho-1;

          if (!(inX||inY||onX||onY)){
            std::vector<Fragment> rasterizedTriangle = triangle_F(
                face.vertexIndices[0],
                face.vertexIndices[1],
                face.vertexIndices[2]);

            std::lock_guard<std::mutex> lock(fragments_mutex); // Utiliza std::mutex aquí
            fragments.insert(
                fragments.end(),
                rasterizedTriangle.begin(),
                rasterizedTriangle.end()
            );
          }
        }
    });

    return fragments;
}


