#pragma once
#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <vector>
#include "Array.h"
namespace PDE {
namespace Func { using F1D = std::function<double(double, double)>; }
enum class BoundaryType { Dirichlet, Neumann, Gamma };
struct BoundaryConfig1D {
    static constexpr std::size_t Left=0, Right=1;
    std::array<BoundaryType,2> types{BoundaryType::Dirichlet,BoundaryType::Dirichlet};
};
enum class AMRAction { refine, coarsen };
struct AMROptions { std::size_t max_refinement_level, max_nodes; double refine_tolerance, coarsen_tolerance; };
enum class MeshMode { Uniform, Adaptive };
struct SolveOptions1D { MeshMode mesh_mode=MeshMode::Uniform; std::optional<AMROptions> amr; };
struct AMREvent { std::size_t step; double time; AMRAction action; double left,right; };
struct Solution1D { std::vector<double> x; Array1D values; std::vector<AMREvent> events; bool refinement_limited=false; };
}
