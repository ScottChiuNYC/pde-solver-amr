#include <pde_solver_amr_core/PDE.h>
#include <cmath>
#include <iostream>
int main() {
    const double pi=std::acos(-1.);
    PDE::Domain1D domain(0,1,0,.1);
    PDE::Grid1D grid(domain,21,100);
    PDE::PDE1D pde;
    pde.initial=[pi](double,double x){return std::sin(pi*x);};
    pde.boundaryL=pde.boundaryR=[](double,double){return 0.;};
    auto solver=PDE::makeSolver1D(grid,pde);
    PDE::SolveOptions1D options;
    options.mesh_mode=PDE::MeshMode::Adaptive;
    options.amr=PDE::AMROptions{3,161,1e-3,2e-4};
    const auto result=solver.solve(options);
    std::cout<<"Final nodes: "<<result.x.size()<<", adaptation events: "<<result.events.size()
             <<", refinement limited: "<<result.refinement_limited<<'\n';
    for(std::size_t i=0;i<result.x.size();++i)std::cout<<result.x[i]<<' '<<result.values[i]<<'\n';
}
