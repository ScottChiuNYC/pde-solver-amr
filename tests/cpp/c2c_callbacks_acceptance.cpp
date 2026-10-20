#include "c2c_acceptance_support.h"
#include <set>
namespace {
struct UserFailure {int payload;};
TEST(C2C_Callbacks,ValidationPrecedesAllSamples){
 PDE::Domain1D d(0,1,0,.1);PDE::Grid1D g(d,5,2);int calls=0;PDE::PDE1D missing;missing.diffusion=[&](double,double){++calls;return 1.;};auto m=PDE::makeSolver1D(g,missing);
EXPECT_THROW(m.solve(),std::runtime_error);EXPECT_EQ(calls,0);
 for(auto field:c2c::fields()){auto p=c2c::constantData();for(auto f:c2c::fields())p.*f=[&](double,double){++calls;return 1.;};p.*field={};calls=0;auto s=PDE::makeSolver1D(g,p);EXPECT_EQ(calls,0);
EXPECT_THROW(s.solve(),std::runtime_error);EXPECT_EQ(calls,0);}
 for(std::size_t side:{0u,1u})for(auto type:{PDE::BoundaryType::Neumann,PDE::BoundaryType::Gamma,static_cast<PDE::BoundaryType>(-7)}){auto p=c2c::constantData();for(auto f:c2c::fields())p.*f=[&](double,double){++calls;return 1.;};p.boundaryConfig.types[side]=type;calls=0;auto s=PDE::makeSolver1D(g,p);EXPECT_EQ(calls,0);
EXPECT_THROW(s.solve(),std::runtime_error);EXPECT_EQ(calls,0);}
}
TEST(C2C_Callbacks,EachNonfiniteRoleAndUserException){
 PDE::Domain1D d(0,1,0,.1);PDE::Grid1D g(d,5,2);for(auto role:c2c::fields()){
 for(double bad:{std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()}){auto p=c2c::constantData();auto valid=p.*role;
 bool left=role==&PDE::PDE1D::boundaryL,right=role==&PDE::PDE1D::boundaryR;
 double targetx=left?g.x(0):right?g.x(g.nx()-1):g.x(1);
 p.*role=[valid,bad,targetx](double t,double x){return t==0&&x==targetx?bad:valid(t,x);};auto s=PDE::makeSolver1D(g,p);
EXPECT_THROW(s.solve(),std::runtime_error);}
 auto p=c2c::constantData();p.*role=[](double,double)->double{throw UserFailure{731};};auto s=PDE::makeSolver1D(g,p);try{(void)s.solve();FAIL()<<"callback exception absent";}catch(UserFailure const& e){EXPECT_EQ(e.payload,731);}catch(...){FAIL()<<"callback exception altered";}}
 for(double a:{0.,-1.}){auto p=c2c::constantData();p.diffusion=c2c::constant(a);auto s=PDE::makeSolver1D(g,p);
EXPECT_THROW(s.solve(),std::runtime_error);}
}
TEST(C2C_Callbacks,ExactRequiredSampleSetsAndUnusedArguments){
 for(double theta:{.5,1.}){
 PDE::Domain1D d(0,1,0,.1);PDE::Grid1D g(d,5,2);auto p=c2c::constantData();using Sample=std::pair<double,double>;std::array<std::set<Sample>,7> seen;std::array<std::set<Sample>,7> expected;
 for(std::size_t i=1;i+1<g.nx();++i){expected[4].insert({g.t(0),g.x(i)});for(std::size_t k=0;k<=g.nt();++k){for(int r=0;r<3;++r)expected[r].insert({g.t(k),g.x(i)});if(theta<1||k>0)expected[3].insert({g.t(k),g.x(i)});}}
 for(std::size_t k=0;k<=g.nt();++k){expected[5].insert({g.t(k),g.x(0)});expected[6].insert({g.t(k),g.x(g.nx()-1)});}
 auto fields=c2c::fields();for(std::size_t r=0;r<7;++r)p.*fields[r]=[&,r](double t,double x){Sample point{t,x};if(!expected[r].count(point))throw UserFailure{static_cast<int>(r)};seen[r].insert(point);return r==0||r>=4?1.:0.;};auto s=PDE::makeSolver1D(g,p);s.setTheta(theta);
EXPECT_NO_THROW(s.solve());for(int r=0;r<7;++r)EXPECT_EQ(seen[r],expected[r])<<"role "<<r;
 }
}
TEST(C2C_Callbacks,GeneratedPDERowFailuresAndRecovery){
 PDE::Domain1D d(0,1,0,1);PDE::Grid1D g(d,3,1);auto p=c2c::constantData(0);p.diffusion=c2c::constant(std::numeric_limits<double>::max());auto s=PDE::makeSolver1D(g,p);
EXPECT_THROW(s.solve(),std::runtime_error);EXPECT_DOUBLE_EQ(s.theta(),.5);p.diffusion=c2c::constant(1);
EXPECT_NO_THROW(s.solve());
 PDE::Domain1D d2(0,2,0,1);PDE::Grid1D g2(d2,3,1);auto p2=c2c::constantData(0);p2.initial=c2c::constant(1);p2.reaction=c2c::constant(4);auto s2=PDE::makeSolver1D(g2,p2);
EXPECT_THROW(s2.solve(),std::runtime_error);
}
TEST(C2C_Callbacks,AdaptiveSharesCallbackAndBoundaryFailureRules){
 PDE::Domain1D d(0,1,0,.1);PDE::Grid1D g(d,5,2);PDE::SolveOptions1D o;o.mesh_mode=PDE::MeshMode::Adaptive;o.amr=PDE::AMROptions{0,5,1,0};
 for(auto role:c2c::fields()){
  auto missing=c2c::constantData();int calls=0;for(auto f:c2c::fields())missing.*f=[&](double,double){++calls;return 1.;};missing.*role={};auto s=PDE::makeSolver1D(g,missing);
  EXPECT_THROW(s.solve(o),std::runtime_error);EXPECT_EQ(calls,0);
  for(double bad:{std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()}){auto p=c2c::constantData();p.*role=c2c::constant(bad);auto solver=PDE::makeSolver1D(g,p);
   EXPECT_THROW(solver.solve(o),std::runtime_error);}
  auto p=c2c::constantData();p.*role=[](double,double)->double{throw UserFailure{990};};auto solver=PDE::makeSolver1D(g,p);try{(void)solver.solve(o);FAIL();}catch(UserFailure const& e){EXPECT_EQ(e.payload,990);}catch(...){FAIL()<<"callback exception altered";}
 }
 for(std::size_t side:{0u,1u})for(auto type:{PDE::BoundaryType::Neumann,PDE::BoundaryType::Gamma,static_cast<PDE::BoundaryType>(91)}){auto p=c2c::constantData();int calls=0;for(auto role:c2c::fields())p.*role=[&](double,double){++calls;return 1.;};p.boundaryConfig.types[side]=type;auto s=PDE::makeSolver1D(g,p);
  EXPECT_THROW(s.solve(o),std::runtime_error);EXPECT_EQ(calls,0);}
 for(double diffusion:{0.,-1.}){auto p=c2c::constantData();p.diffusion=c2c::constant(diffusion);auto s=PDE::makeSolver1D(g,p);
  EXPECT_THROW(s.solve(o),std::runtime_error);}
 auto p=c2c::constantData();p.source=[](double t,double){if(t==0)throw UserFailure{123};return 0.;};auto s=PDE::makeSolver1D(g,p);s.setTheta(1);
 EXPECT_NO_THROW(s.solve(o));
}
}
