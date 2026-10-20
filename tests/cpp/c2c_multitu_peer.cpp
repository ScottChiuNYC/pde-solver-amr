#include <pde_solver_amr_core/PDE.h>
// A second, independently compiled umbrella-using translation unit.
double c2c_second_translation_unit_solver(){
 PDE::Domain1D domain(0,1,0,.1);PDE::Grid1D grid(domain,5,2);PDE::PDE1D pde;
 pde.initial=pde.boundaryL=pde.boundaryR=[](double,double){return 1.;};
 auto solver=PDE::makeSolver1D(grid,pde);return solver.solve()[2];
}
