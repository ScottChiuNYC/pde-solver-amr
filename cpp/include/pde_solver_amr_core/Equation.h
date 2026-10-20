#pragma once
#include "Types.h"
namespace PDE {
struct PDE1D {
    PDE1D():diffusion([](double,double){return 1.;}),advection([](double,double){return 0.;}),source([](double,double){return 0.;}),reaction([](double,double){return 0.;}) {}
    Func::F1D diffusion,advection,source,reaction,initial,boundaryL,boundaryR;
    BoundaryConfig1D boundaryConfig;
};
}
