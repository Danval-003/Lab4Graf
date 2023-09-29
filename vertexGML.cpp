#include "color.h"
#include "framebuffer.h"
#include "vertexGML.h"
#include "uniform.h"
#include "FastNoiseLite.h"
#include <SDL2/SDL_timer.h>
#include <cmath>
#include <glm/common.hpp>
#include <random>
#include <mutex>

glm::vec3 L = glm::vec3(0.0f, 0, 1.0f);
glm::vec3 L2 = glm::vec3(0.0f, 0, 1.0f);
glm::vec3 L3 = glm::vec3(0.0f, 0, -1.0f);
glm::vec3 L4 = glm::vec3(0.0f, 0, 1.0f);
glm::vec3 L5 = glm::vec3(0.0f, 0, -1.0f);

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

    int minX = std::max(std::min(std::min(A.x, B.x), C.x), 0.0f);
    int minY = std::max(std::min(std::min(A.y, B.y), C.y),0.0f);
    int maxX = std::min(std::max(std::max(A.x, B.x), C.x), 1.0f* pantallaAncho-1);
    int maxY = std::min(std::max(std::max(A.y, B.y), C.y), 1.0f* pantallaAncho-1);


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
            double worldZ = a.z * w + b.z * v + c.z * u;
            if(worldZ<=0)
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
                        Fragment{glm::vec3(P.x, P.y, z), color * intensity, intensity, original, normal}
                );
            }
        }
    }

    return triangleFragments;
}



VertexGML vertexShader(const VertexGML& vertex, const Uniform& u) {

  glm::vec4 v = glm::vec4(vertex.position.x, vertex.position.y, vertex.position.z, 1);

  glm::vec4 r =  u.projection * u.view * u.model * v;
  double z = r.z;
  r= u.viewport * r;

  return VertexGML{
    glm::vec3( r.x/r.w , r.y/r.w, r.z/r.w),
    vertex.position,
    vertex.color,
    vertex.normal,
    vertex.texture,
    z
  };
};

float timeAni = 0.0f;

FastNoiseLite sunNoiseGenerator;
FastNoiseLite planetContientNoiseGenerator;
FastNoiseLite planetCloudNoiseGenerator;
FastNoiseLite planet2Generator;
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
    float intensity = glm::dot(fragment.normal, L);
    intensity = (intensity>1)? 1.0f: intensity;

    glm::vec3 tmpColor =  glm::mix(yellowSun,orangeSun,(1-noiseValue));
    tmpColor = glm::mix(iluminationSun,tmpColor, intensity);

    fragment.color = Color(tmpColor.x, tmpColor.y, tmpColor.z);
}

void planetFragmentShader(Fragment& fragment) {

    glm::vec3 groundColor = glm::vec3(0.0f/255.0f,105.0f/255.0f, 148.0f/255.0f);
    glm::vec3 oceanColor = glm::vec3(0.0f/255.0f,143.0f/255.0f, 57.0f/255.0f);
    glm::vec3 cloudColor = glm::vec3(255.0f/255.0f,255.0f/255.0f, 255.0f/255.0f);
    glm::vec3 uv = glm::vec3(fragment.original.x, fragment.original.y, fragment.original.z) ;
    float intensity = glm::dot(fragment.normal, L2);
    float intensity2 = glm::dot(fragment.normal, L3);
    //float intensity = fragment.intensity;
    float oxE = 3000;
    float oyE = 3000;
    float zoomE = 700;

    float noiseValue = abs(planetContientNoiseGenerator.GetNoise((uv.x + oxE) * zoomE, (uv.y + oyE) * zoomE, uv.z * zoomE)) ;
    float noiseValueC = abs(planetContientNoiseGenerator.GetNoise((uv.x + ox) * zoom/3.0f, (uv.y + oy) * zoom/3.0f, uv.z * zoom/3.0f)) ;

    intensity = (intensity< 0.05f)? 0.05f: intensity;
    intensity2 = (intensity2< 0.05f)? 0.05f: intensity2;

    glm::vec3 tmpColor =  (noiseValue>0.5f)? oceanColor: groundColor;
    tmpColor = (noiseValueC<0.8f && noiseValueC > 0.5f)? glm::mix(cloudColor, tmpColor, noiseValueC): tmpColor;
    tmpColor =(intensity<=1)? glm::mix(glm::vec3(0,0,0),tmpColor, intensity): tmpColor;
    if(intensity>0.05f){
        if(intensity2<1 && intensity2>0.58f){
            tmpColor = glm::mix(tmpColor,glm::vec3(0,0,0), intensity2);
        }
    }
    else{
        tmpColor = tmpColor;
    }

    fragment.color = Color(tmpColor.x, tmpColor.y, tmpColor.z);
}

void planet2FragmentShader(Fragment& fragment) {

    glm::vec3 groundColor = glm::vec3(205.0f/255.0f,205.0f/255.0f, 205.0f/255.0f);
    glm::vec3 oceanColor = glm::vec3(105.0f/255.0f,105.0f/255.0f, 105.0f/255.0f);
    glm::vec3 cloudColor = glm::vec3(255.0f/255.0f,255.0f/255.0f, 255.0f/255.0f);
    glm::vec3 uv = glm::vec3(fragment.original.x, fragment.original.y, fragment.original.z) ;
    float intensity = glm::dot(fragment.normal, L);
    //float intensity2 = glm::dot(fragment.normal, L3 * 2.0f);
    //float intensity = fragment.intensity;
    float oxE = 3000;
    float oyE = 3000;
    float zoomE = 700;

    float noiseValue = abs(planet2Generator.GetNoise((uv.x + oxE) * zoomE/5.0f, (uv.y + oyE) * zoomE/5.0f, uv.z * zoomE/5.0f)) ;

    intensity = (intensity< 0.05f)? 0.05f: intensity;
    //intensity2 = (intensity2< 0.05f)? 0.05f: intensity2;

    glm::vec3 tmpColor =  glm::mix(oceanColor, groundColor, noiseValue);
    tmpColor =(intensity<=1)? glm::mix(glm::vec3(0,0,0),tmpColor, intensity): tmpColor;

    fragment.color = Color(tmpColor.x, tmpColor.y, tmpColor.z);
}

void moonFragmentShader(Fragment& fragment) {

    glm::vec3 groundColor = glm::vec3(176.0f/255.0f,176.0f/255.0f, 176.0f/255.0f);
    glm::vec3 oceanColor = glm::vec3(67.0f/255.0f,75.0f/255.0f, 77.0f/255.0f);
    glm::vec3 cloudColor = glm::vec3(255.0f/255.0f,255.0f/255.0f, 255.0f/255.0f);
    glm::vec3 uv = glm::vec3(fragment.original.x, fragment.original.y, fragment.original.z) ;
    float intensity = glm::dot(fragment.normal, L5);
    float intensity2 = glm::dot(fragment.normal, L4 * 2.5f);
    //float intensity = fragment.intensity;
    float oxE = 3000;
    float oyE = 3000;
    float zoomE = 700;

    float noiseValue = abs(planetContientNoiseGenerator.GetNoise((uv.x + oxE) * zoomE, (uv.y + oyE) * zoomE, uv.z * zoomE)) ;

    intensity = (intensity< 0.05f)? 0.05f: intensity;
    intensity2 = (intensity2> 0.95f)? 0.95f: intensity2;

    glm::vec3 tmpColor =  (noiseValue>0.5f)? oceanColor: groundColor;
    tmpColor =(intensity<=1)? glm::mix(glm::vec3(0,0,0),tmpColor, intensity): tmpColor;
    if(intensity>0.05f){
        if(intensity2<1 && intensity2>0.15f){
            tmpColor = glm::mix(glm::vec3(0,0,0), tmpColor, (1-intensity2));
        }
    }
    else{
        tmpColor = tmpColor;
    }

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

    ox = 3000.0f+ std::sin(timeAni)*0.02f;
    oy =3000.0f+ std::sin(timeAni*0.3f)*0.02f;
    zoom =1300.0f;

    sunNoiseGenerator = noiseGenerator;
}

void configPlanetNoiseGenerator(){

    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    noiseGenerator.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
    noiseGenerator.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Hybrid);
    noiseGenerator.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    noiseGenerator.SetRotationType3D(FastNoiseLite::RotationType3D_ImproveXYPlanes);
    noiseGenerator.SetSeed(1337);
    noiseGenerator.SetFrequency(0.010f);
    noiseGenerator.SetFractalType(FastNoiseLite::FractalType_DomainWarpIndependent);
    noiseGenerator.SetFractalOctaves(3);
    noiseGenerator.SetFractalLacunarity(2.0f);
    noiseGenerator.SetFractalGain(0.50f);

    FastNoiseLite noiseGen;
    noiseGen.SetSeed(1337);
    noiseGen.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Hybrid);
    noiseGen.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
    noiseGen.SetDomainWarpType(FastNoiseLite::DomainWarpType_BasicGrid);
    noiseGen.SetDomainWarpAmp(50.0f);
    noiseGen.SetFrequency(0.010f);
    noiseGen.SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
    noiseGen.SetFractalOctaves(5);
    noiseGen.SetFractalLacunarity(2.0f);
    noiseGen.SetFractalGain(0.60);

    FastNoiseLite noiseGen2;
    noiseGen2.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noiseGen2.SetSeed(1337);
    noiseGen2.SetFrequency(0.010f);
    noiseGen2.SetFractalType(FastNoiseLite::FractalType_FBm);
    noiseGen2.SetFractalOctaves(5);
    noiseGen2.SetFractalLacunarity(2.00f);
    noiseGen2.SetFractalGain(0.50f);
    noiseGen2.SetFractalWeightedStrength(0.00f);
    noiseGen2.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    noiseGen2.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
    noiseGen2.SetCellularJitter(1.0f);
    noiseGen2.SetDomainWarpAmp(50.0f);


    planetCloudNoiseGenerator = noiseGen;
    planetContientNoiseGenerator = noiseGenerator;
    planet2Generator = noiseGen2;
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

    Color color = (normalizedHash < starDensity) ? Color(244, 244, 244): BLACK;

    fragment.color = color;
}

FastNoiseLite noise;  // Crea una instancia de FastNoiseLite

Color skyFragmentShader2(glm::vec3& pos, glm::vec3& offset ) {
    // Escala las coordenadas para ajustar la densidad y distribución de estrellas
    float x = pos.x + offset.x * 100.0f;
    float y = pos.y + offset.y * 100.0f;
    float z = pos.z + offset.z * 100.0f;

    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    // Utiliza FastNoiseLite para obtener un valor de ruido
    float density = noise.GetNoise(x, y);

    // Ajusta el umbral para controlar la densidad de estrellas
    float threshold = 0.97f;  // Puedes ajustar este valor

    // Si el valor de ruido es mayor que el umbral, muestra una estrella, de lo contrario, muestra el cielo oscuro
    Color color = (density > threshold) ? Color(244, 244, 244) : BLACK;

    return color;
}


void shipFragmentShader(Fragment& fragment) {
    float intensity = abs(glm::dot(fragment.normal, L));
    glm::vec3 oceanColor = glm::vec3(67.0f/255.0f,75.0f/255.0f, 77.0f/255.0f);

    intensity= (intensity>1)? 1.0f: intensity;

    glm::vec3 tmpColor = glm::mix(oceanColor, glm::vec3(0,0,0), 1-intensity);


    fragment.color = Color(tmpColor.x, tmpColor.y, tmpColor.z);
}

