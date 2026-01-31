//
// Created by vivek on 10/31/2025.
//
#pragma once
#include <string>
#include <vector>
#include "mesh.h"

#ifndef LOAD_OBJ_FAST_H
#define LOAD_OBJ_FAST_H

bool load_meshes_fastobj(const std::string& path, std::vector<Mesh>& out, AABB& sceneBounds);

bool load_triangle_meshes_fastobj(const std::string& path, std::vector<TriangleMesh>& outMeshes, Materials& outMats, AABB& sceneBounds);
#endif //LOAD_OBJ_FAST_H
