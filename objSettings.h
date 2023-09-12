#pragma once

#include "vertexGML.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>

bool loadOBJ(const std::string& path, std::vector<VertexGML>& out_vertices, std::vector<Face>& out_faces, std::vector<Face>& normal_faces, std::vector<VertexGML>& normal_vertex, std::vector<Face>& texture_faces, std::vector<VertexGML>& texture_vertex);

bool loadOBJ(const std::string& path, std::vector<Facer>& obj);

void triangulateFaces(const std::vector<Face>& faces, const std::vector<VertexGML>& vertices, std::vector<VertexGML>& outVertices, std::vector<Face>& out_faces);
