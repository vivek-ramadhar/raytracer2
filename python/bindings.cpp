//
// Created by Vivek Ramadhar on 12/14/25.
//
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include "render_to_file.h"

namespace py = pybind11;

PYBIND11_MODULE(sh_render, m, py::mod_gil_not_used()) {
    m.doc() = "pybind11 basic example plugin. Pass in path to obj file and it will output a .ppm image\n";
    m.def("render", &render, "A function that renders using");
}