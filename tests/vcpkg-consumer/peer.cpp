#include <pde_solver_amr_core/PDE.h>
// A second translation unit also instantiates the header-only solver.
bool peer_solver_check() {
    PDE::Domain1D domain(0,1,0,.1);
    PDE::Grid1D grid(domain,5,2);
    PDE::PDE1D pde;
    pde.initial=pde.boundaryL=pde.boundaryR=[](double,double){return 2.;};
    auto solver=PDE::makeSolver1D(grid,pde);
    auto result=solver.solve();
    return result.size()==5 && std::abs(result[2]-2)<1e-12;
}
