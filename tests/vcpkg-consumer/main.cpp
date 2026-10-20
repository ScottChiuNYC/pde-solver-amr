#include <pde_solver_amr_core/PDE.h>
#include <pde_solver_amr_core/hello.h>
#include <cmath>
#include <iostream>
bool peer_solver_check();
int main() {
    const double pi=std::acos(-1.);
    PDE::Domain1D domain(0,1,0,.1);
    PDE::Grid1D grid(domain,101,1000);
    PDE::PDE1D pde;
    pde.initial=[pi](double,double x){return std::sin(pi*x);};
    pde.boundaryL=pde.boundaryR=[](double,double){return 0.;};
    auto solver=PDE::makeSolver1D(grid,pde);
    const auto result=solver.solve();
    double error=0;
    for(std::size_t i=0;i<grid.nx();++i)
        error=std::max(error,std::abs(result[i]-std::exp(-pi*pi*.1)*std::sin(pi*grid.x(i))));
    PDE::SolveOptions1D options;
    options.mesh_mode=PDE::MeshMode::Adaptive;
    options.amr=PDE::AMROptions{2,401,1e-4,2e-5};
    const auto adaptive=solver.solve(options);
    std::cout<<hello()<<" heat error="<<error<<" adaptive nodes="<<adaptive.x.size()<<'\n';
    return error<=4e-5 && result[0]==0 && result[100]==0 &&
        adaptive.values.size()==adaptive.x.size() && !adaptive.events.empty() && peer_solver_check()?0:1;
}
