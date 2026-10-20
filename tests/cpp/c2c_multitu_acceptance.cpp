#include "c2c_acceptance_support.h"
double c2c_second_translation_unit_solver();
TEST(C2C_Integration,UmbrellaAcrossMultipleTranslationUnits){EXPECT_NEAR(c2c_second_translation_unit_solver(),1,1e-12);PDE::Domain1D d(0,1,0,.1);PDE::Grid1D g(d,5,2);auto p=c2c::constantData();auto s=PDE::makeSolver1D(g,p);EXPECT_NEAR(s.solve()[2],1,1e-12);}
