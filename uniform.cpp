#include "uniform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "framebuffer.h"



glm::vec3 cameraPosition = glm::vec3(0, 0, 5);
glm::vec3 orientation = glm::vec3 (0, 0, 0);
glm::vec3 uper = glm::vec3(0, 1, 0);

glm::mat4 createModelMatrix() {
    glm::mat4 transtation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1.0f, 1.0f, 1.0f));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    return transtation * scale * rotation;
}

glm::mat4 createModelMatrixPlanet() {
    glm::mat4 transtation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1.0f, 1.0f, 1.0f));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    return transtation * scale * rotation;
}

glm::mat4 createModelMatrixPlanet(glm::vec3 positionInMap, float scale_, float b) {
    glm::mat4 transtation = glm::translate(glm::mat4(1), positionInMap);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(scale_, scale_, scale_));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(b), glm::vec3(0.0f, 1.0f, 0.0f));

    return transtation * scale * rotation;
}

glm::mat4 createModelMatrixPlanet(glm::vec3 positionInMap, float scale_, float b, float rotateX, float rotateZ) {
    glm::mat4 transtation = glm::translate(glm::mat4(1), positionInMap);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(scale_, scale_, scale_));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(b), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotation2 = glm::rotate(glm::mat4(1), glm::radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotation3 = glm::rotate(glm::mat4(1), glm::radians(rotateZ), glm::vec3(0.0f, 0.0f, 1.0f));

    return transtation * scale * rotation * rotation2 * rotation3;
}


glm::mat4 createModelMatrix(glm::vec3 positionInMap, float scale_) {
    glm::mat4 transtation = glm::translate(glm::mat4(1), positionInMap);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(scale_, scale_, scale_));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    return transtation * scale * rotation;
}


glm::mat4 createViewMatrix() {
    return glm::lookAt(
        // donde esta
        cameraPosition,
        // hacia adonde mira
        orientation,
        // arriba
        uper
    );
}


glm::mat4 createViewMatrix(glm::vec3 positionInMap, float rotation) {
    // Calcular la dirección hacia la que mira la cámara
    glm::vec3 front;
    front.x = cos(glm::radians(rotation));
    front.y = 0.0f; // La cámara mira horizontalmente en este caso
    front.z = sin(glm::radians(rotation));
    front = glm::normalize(front);

    // Calcular la posición de la cámara después de la rotación
    glm::vec3 newPosition = positionInMap;
    orientation = newPosition + front;

    return glm::lookAt(
        newPosition,
        orientation,
        glm::vec3(0, 1, 0)
    ) ;
}

glm::mat4 createProjectionMatrix() {
  float fovInDegrees = 45.0f;
  float aspectRatio = static_cast<float>(pantallaAncho) / pantallaAlto;
  float nearClip = 0.1f;
  float farClip = 100.0f;

  return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createProjectionMatrix(float scale) {
    float fovInDegrees = 45.0f;
    float aspectRatio = static_cast<float>(pantallaAncho) / pantallaAlto;
    float nearClip = 0.1f * scale;
    float farClip = 100.0f;

    return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createViewportMatrix() {
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale
    viewport = glm::scale(viewport, glm::vec3(pantallaAncho / 2.0f, pantallaAlto / 2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}


void configureUniform(Uniform* u){
    u->model = createModelMatrix();
    u->view = createViewMatrix();
    u->projection = createProjectionMatrix();
    u->viewport = createViewportMatrix();
}

void configureUniformPlanet(Uniform* u){
    u->model = createModelMatrixPlanet();
    u->view = createViewMatrix();
    u->projection = createProjectionMatrix();
    u->viewport = createViewportMatrix();
}

