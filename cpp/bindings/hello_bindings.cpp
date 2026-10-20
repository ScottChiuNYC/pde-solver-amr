#include "pde_solver_amr_core/hello.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(pde_solver_amr_core, m) {
  m.doc() = "pybind11 hello world module";

  m.def("hello", &hello, R"pbdoc(
      A function that returns a Hello World string.
  )pbdoc");
}