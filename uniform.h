#pragma once
#include <glm/glm.hpp>


extern glm::vec3 cameraPosition;
extern glm::vec3 orientation;
extern glm::vec3 uper;

struct Uniform {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 viewport;
};

glm::mat4 createModelMatrix();

glm::mat4 createViewMatrix();

glm::mat4 createProjectionMatrix();

glm::mat4 createViewportMatrix();

void configureUniform(Uniform* u);
glm::mat4 createModelMatrixPlanet();
void configureUniformPlanet(Uniform* u);


glm::mat4 createViewMatrix(glm::vec3 positionInMap, float rotation);
glm::mat4 createModelMatrix(glm::vec3 positionInMap, float scale_);
glm::mat4 createModelMatrixPlanet(glm::vec3 positionInMap, float scale_, float b);
glm::mat4 createModelMatrixPlanet(glm::vec3 positionInMap, float scale_, float b, float rotateX, float rotateZ);
