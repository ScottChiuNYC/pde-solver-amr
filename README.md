# pde_solver_amr

Header-only C++17 solver for one-dimensional parabolic PDEs with Dirichlet boundaries, a fixed time grid, and optional spatial refinement/coarsening. The theta scheme supports Crank–Nicolson (`theta=0.5`, the default) through backward Euler (`theta=1`) and uses checked Thomas elimination.

```cpp
#include <pde_solver_amr_core/PDE.h>
#include <cmath>

int main() {
    const double pi = std::acos(-1.0);
    PDE::Domain1D domain(0, 1, 0, 0.1);
    PDE::Grid1D grid(domain, 101, 1000);
    PDE::PDE1D pde; // diffusion=1; advection/reaction/source=0
    pde.initial = [pi](double, double x) { return std::sin(pi*x); };
    pde.boundaryL = pde.boundaryR = [](double, double) { return 0.0; };
    auto solver = PDE::makeSolver1D(grid, pde);
    auto uniform = solver.solve();

    PDE::SolveOptions1D options;
    options.mesh_mode = PDE::MeshMode::Adaptive;
    options.amr = PDE::AMROptions{3, 2049, 1e-4, 2e-5};
    auto adaptive = solver.solve(options);
    return uniform.size() == grid.nx() && adaptive.x.size() == adaptive.values.size() ? 0 : 1;
}
```

The solver retains references to the grid and PDE: keep both alive, along with objects referenced by their callbacks, for every solve. Returned arrays and `Solution1D` own their storage. `solve(SolveOptions1D{})` returns the uniform values plus coordinates and an empty event list; its optional AMR controls are ignored. Adaptive results include chronological accepted split/merge events and a sticky `refinement_limited` flag when the depth or all-or-none node cap blocks refinement. Refinement tolerances control a solution-curvature heuristic; they do not bound PDE error. The uniform second-order spatial convergence claim does not extend to arbitrary adaptive meshes.

All callbacks take `(t,x)` and must return finite values; sampled diffusion must be positive. Missing callbacks, unsupported boundaries/options, invalid geometry, and unusable pivots throw `std::runtime_error` with context. User callback exceptions propagate unchanged. Initial endpoint values come from Dirichlet callbacks. Arithmetic follows checked binary64 round-to-nearest operations with gradual underflow; builds that change that arithmetic model are outside these assumptions. The solver uses exactly the original fixed time grid. See [the numerical contract](docs/development/essence.md) for the full API and precise sampling/arithmetic rules, and [the executable example](examples/cpp/demo.cpp) for adaptive output.

The existing Python package retains its scaffold `hello()` API; the PDE solver is a C++ library.

## Reproducible setup and validation

From a fresh checkout:

```bash
python scripts/bootstrap.py
python scripts/validate.py --wheel
```

Requires Python 3.12+, Git and a platform C++17 toolchain (GCC/Clang on Linux; Visual Studio 2022 C++ tools and a Windows SDK on Windows). Linux also needs curl, zip, unzip and tar. Bootstrap installs pinned tools and the declared vcpkg baseline in `.dev-tools`. Validation configures, builds and runs CTest/pytest, then rebuilds a native wheel from the sdist and checks its installation in a clean environment, and validates an independent C++ port consumer.

The scripts supply tool paths automatically, including Ninja inside vcpkg compiler detection. Compiler tracking remains enabled. No shell activation or manual path setup is required.

## Development contract

Before adding implementation, dependencies, bindings, tests, or build-system changes, read [`docs/development/PROJECT_SKELETON_CONTRACT.md`](docs/development/PROJECT_SKELETON_CONTRACT.md). It is the normative guide to this project's C++/Python source layout, dependency wiring, packaging, build, test, and CI extension points.

Domain implementation documents should describe what to implement; the Project Skeleton Contract describes how that implementation fits into this repository.

## vcpkg dependencies

`vcpkg-configuration.json` keeps Microsoft vcpkg as the only package registry and declares the repository-local `vcpkg-ports` directory as an overlay. The `gtest` dependency is resolved from `vcpkg-ports/gtest` before registry lookup; all other ports continue to come from Microsoft vcpkg.

The local GoogleTest port preserves the standard `GTest::*` CMake targets but skips the pkg-config executable check, preventing vcpkg from downloading or executing `pkgconf`. No personal vcpkg registry is required. When this recipe is updated, change the files under `vcpkg-ports/gtest` and validate the project on Windows and Linux.

## Build presets

Validation uses `windows-vcpkg` / `windows-release` / `windows-release-tests` on Windows and `linux-vcpkg-release` / `linux-release` / `linux-release-tests` on Linux. Windows uses Visual Studio 2022; Linux uses Ninja. `CMakePresets.json` also contains Debug presets. Keep preset changes aligned with `scripts/validate.py` and the Poetry native build hook in `build.py`.

The pybind extension is emitted directly into `src/pde_solver_amr` on both platforms, so Python tests import the freshly built native module without generator-specific copy logic. pybind11 remains managed by the existing CMake `FetchContent` configuration.

## Continuous integration

GitHub automatically runs `.github/workflows/build-and-test.yml` on Windows and Linux for pushes and pull requests **unless all changed files are under `docs/development/`**. Changes outside that directory (including mixed documentation/code changes) still trigger it, and manual runs remain available via `workflow_dispatch`. Reserve `docs/development/` for specifications, audits and execution evidence—not executable build/test inputs or code-generation sources. The workflow invokes `python scripts/bootstrap.py` and `python scripts/validate.py --wheel`; those scripts build and test the project, validate the sdist/wheel, and check that `pkgconf` was neither installed nor acquired.

## Installable C++ library

The regular presets build the C++ static core, the pybind11 extension and the examples/tests. Public headers install under `include/pde_solver_amr_core/`, compiled libraries under `lib/` (Windows DLLs under `bin/`), and CMake config/version/targets under `lib/cmake/pde_solver_amr_core/`. Consumers use `find_package(pde_solver_amr_core CONFIG REQUIRED)` and `pde_solver_amr_core::core`. The CMake package version uses the numeric release component of the `version` Copier answer (for example, `1.2.3rc1` becomes `1.2.3`); Poetry and the port's `version-string` retain the full version; version compatibility is within the same major version. Public include paths distinguish the build tree from the install tree, and the target propagates C++17.

The local port is `vcpkg-ports/pde-solver-amr-core`. Its `vcpkg.json` describes the library to install and depends only on vcpkg CMake helpers. The root `vcpkg.json` continues to describe development dependencies such as GoogleTest; it is not the port manifest. Keep the port inside this source checkout: it locates the library using `../..`, builds the current working tree, and installs no tests or examples. Future production dependencies must also be declared in the port and its exported config where appropriate.

The core is always static. `vcpkg_check_linkage(ONLY_STATIC_LIBRARY)` keeps it static even for a dynamic library triplet such as `x64-windows`; CRT linkage still follows that triplet. The port passes `BUILD_PYTHON_BINDINGS=OFF`, so it never discovers Python or fetches pybind11. Normal builds retain the extension and Poetry hook. The independent consumer runs uniform and adaptive solves in two translation units and compiles every installed public header by itself in both configurations.

To test packaging alone after bootstrap:

```bash
python scripts/validate_vcpkg.py
```

For a direct CMake install, configure with `BUILD_TESTING=OFF` and `BUILD_EXAMPLES=OFF` and `BUILD_PYTHON_BINDINGS=OFF`, build, then run `cmake --install <build-directory> --config Release --prefix <install-directory>`. The vcpkg recipe performs this for both Debug and Release and combines their imported targets.

## Independent downstream consumer

A C++ consumer needs CMake, vcpkg and the platform compiler. Python, Poetry and pybind11 are not consumer dependencies. Place the consumer beside the generated project, rather than inside its source/build tree. Create `consumer/vcpkg.json`:

```json
{ "dependencies": ["pde-solver-amr-core"] }
```

Create `consumer/vcpkg-configuration.json`, with the generated project's pinned baseline and a relative overlay path (replace `my-project` with this checkout's directory):

```json
{
  "default-registry": {
    "kind": "git",
    "baseline": "00807e89724847a866033a2f042c38fafae3c534",
    "repository": "https://github.com/microsoft/vcpkg"
  },
  "overlay-ports": ["../my-project/vcpkg-ports"]
}
```

Create `consumer/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.25)
project(consumer LANGUAGES CXX)
find_package(pde_solver_amr_core CONFIG REQUIRED)
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE pde_solver_amr_core::core)
```

Create `consumer/main.cpp`:

```cpp
#include <pde_solver_amr_core/hello.h>
#include <iostream>
int main() {
    const auto message = hello();
    std::cout << message << std::endl;
    return message == "Hello, World!" ? 0 : 1;
}
```

Linux, from `consumer/`, with `$VCPKG_ROOT` pointing to a bootstrapped checkout:

```bash
"$VCPKG_ROOT/vcpkg" install --triplet x64-linux --binarysource=clear
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-linux -DVCPKG_INSTALLED_DIR="$PWD/vcpkg_installed" -DVCPKG_MANIFEST_INSTALL=OFF
cmake --build build
./build/consumer
```

Windows PowerShell, from `consumer/`, with `$env:VCPKG_ROOT` set:

```powershell
& "$env:VCPKG_ROOT/vcpkg.exe" install --triplet x64-windows --binarysource=clear
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows "-DVCPKG_INSTALLED_DIR=$PWD/vcpkg_installed" -DVCPKG_MANIFEST_INSTALL=OFF
cmake --build build --config Release
./build/Release/consumer.exe
```

For Windows Debug, use `--config Debug` and `./build/Debug/consumer.exe`. The static core can use either `x64-windows` (dynamic CRT) or `x64-windows-static-md` (also dynamic CRT); library linkage remains static.

Local source changes do not change vcpkg's port recipe hash. Use a fresh consumer install directory, or remove the already installed port before reinstalling; keep `--binarysource=clear` to force compilation from the working tree. Moving this port away from its source checkout is unsupported. CI covers Linux x64 and Windows x64 with Visual Studio 2022; other platforms/toolchains and cross compilation are not validated.

## Template updates

Commit `.copier-answers.pypackage-cpp.yml` and project changes before `copier update -a .copier-answers.pypackage-cpp.yml --vcs-ref <template-tag>`. Template Git tags version updates independently of your project's version.
## Infrastructure provenance

This project composes two independent Copier overlays:

- `.copier-answers.pypackage-cpp.yml` — Python/C++ build, packaging, dependency, test, and CI skeleton;
- `.copier-answers.art.yml` — agent skill routing and repository-PDF publication infrastructure.

For the canonical repository map, ownership boundaries, CodeBinder/GHCR dependency pinning, review-PDF design, rollout/rollback playbooks, and cold-start checklist, see:

https://github.com/ScottChiuNYC/agentic-repo-template/blob/main/docs/ecosystem-architecture.md

The project-local `docs/development/PROJECT_SKELETON_CONTRACT.md` remains the authority for build/package/layout conventions. Domain implementation authority remains the applicable frozen Essence and its audit/C2C evidence.

