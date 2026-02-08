//
// Created by Vivek Ramadhar on 12/13/25.
//

#ifndef RAYTRACER2_RENDER_TO_FILE_H
#define RAYTRACER2_RENDER_TO_FILE_H
#include "renderer.h"

void save_via_ffmpeg(const unsigned char* data, int width, int height, const char* filename);
void SaveAsPPM(const std::string& filename, const uchar* bgr, int width, int height);
void frameCamera(Camera& cam, const AABB& bbox,
                 float fovy_rads, float aspect,
                 float3 view_dir, float3 up_hint, float margin);
int render(int argc, char** argv);
#endif //RAYTRACER2_RENDER_TO_FILE_H