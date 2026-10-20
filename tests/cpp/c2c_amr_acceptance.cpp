#include "c2c_amr_oracle.h"
namespace {
PDE::SolveOptions1D adaptive(PDE::AMROptions a){PDE::SolveOptions1D o;o.mesh_mode=PDE::MeshMode::Adaptive;o.amr=a;return o;}
TEST(C2C_AMR,UniformIdentityAndIgnoredAMR){PDE::Domain1D d(0,1,0,.2);PDE::Grid1D g(d,11,7);auto f=c2c::fixture(0);auto s=PDE::makeSolver1D(g,f.p);for(double theta:{.5,.75,1.}){s.setTheta(theta);auto original=s.solve();for(bool supply:{false,true}){PDE::SolveOptions1D o;if(supply)o.amr=PDE::AMROptions{0,0,-1,std::numeric_limits<double>::quiet_NaN()};auto r=s.solve(o);EXPECT_EQ(c2c::vector(r.values),c2c::vector(original));ASSERT_EQ(r.x.size(),g.nx());for(std::size_t i=0;i<g.nx();++i)EXPECT_DOUBLE_EQ(r.x[i],g.x(i));EXPECT_TRUE(r.events.empty());EXPECT_FALSE(r.refinement_limited);}}}
TEST(C2C_AMR,OptionsBeforeCallbacks){PDE::Domain1D d(0,1,0,1);PDE::Grid1D g(d,3,1);auto p=c2c::constantData();int calls=0;for(auto f:c2c::fields())p.*f=[&](double,double){++calls;return 1.;};auto s=PDE::makeSolver1D(g,p);std::vector<PDE::SolveOptions1D> bad;PDE::SolveOptions1D missing;missing.mesh_mode=PDE::MeshMode::Adaptive;bad.push_back(missing);PDE::SolveOptions1D unknown;unknown.mesh_mode=static_cast<PDE::MeshMode>(73);bad.push_back(unknown);double inf=std::numeric_limits<double>::infinity(),nan=std::numeric_limits<double>::quiet_NaN();for(auto a:std::vector<PDE::AMROptions>{{0,2,1,0},{0,3,0,0},{0,3,-1,0},{0,3,inf,0},{0,3,nan,0},{0,3,1,-1},{0,3,1,1},{0,3,1,2},{0,3,1,inf},{0,3,1,nan}})bad.push_back(adaptive(a));for(auto const& o:bad){calls=0;
EXPECT_THROW(s.solve(o),std::runtime_error);EXPECT_EQ(calls,0);}}
TEST(C2C_AMR,ExistingAnalyticFixturesAgainstIndependentMeshAndStepOracle){
 bool observedSplit=false,observedMerge=false,observedLaterSplit=false;
 struct Case{int f;double T;std::size_t nx,nt,depth,cap;double refine,coarsen;};
 // These are direct adaptation/row-conformance probes using ONLY §7 analytic data.
 // No PDE error threshold, convergence ladder, or expected empirical event count.
 const Case cases[]={{0,.1,5,4,2,65,.06,.01},{1,.2,5,4,2,65,.02,.005},{2,.2,5,4,2,65,.02,.005},{3,.5,5,10,1,65,.1,.09},{3,.5,5,10,2,9,.02,.019},{0,.1,5,4,0,5,.001,0},{0,.1,5,4,3,6,.001,0},{0,.1,5,4,3,9,.001,0},{0,.1,5,4,3,65,.2,.01}};
 for(auto c:cases)for(double theta:{.5,.75,1.}){SCOPED_TRACE(::testing::Message()<<"fixture="<<c.f<<" theta="<<theta<<" cap="<<c.cap<<" refine="<<c.refine);auto f=c2c::fixture(c.f);PDE::Domain1D d(0,1,0,c.T);PDE::Grid1D g(d,c.nx,c.nt);PDE::AMROptions a{c.depth,c.cap,c.refine,c.coarsen};auto expected=c2c::reference(g,f.p,theta,a);auto s=PDE::makeSolver1D(g,f.p);s.setTheta(theta);auto actual=s.solve(adaptive(a));c2c::compare(actual,expected);ASSERT_EQ(actual.x.size(),actual.values.size());EXPECT_LE(actual.x.size(),a.max_nodes);EXPECT_DOUBLE_EQ(actual.x.front(),d.xL);EXPECT_DOUBLE_EQ(actual.x.back(),d.xR);EXPECT_DOUBLE_EQ(actual.values[0],f.p.boundaryL(d.t1,d.xL));EXPECT_DOUBLE_EQ(actual.values[actual.values.size()-1],f.p.boundaryR(d.t1,d.xR));for(std::size_t i=1;i<actual.x.size();++i){EXPECT_TRUE(std::isfinite(actual.x[i]));EXPECT_GT(actual.x[i],actual.x[i-1]);}for(auto const& e:actual.events){EXPECT_LT(e.step,g.nt());EXPECT_DOUBLE_EQ(e.time,g.t(e.step));EXPECT_LT(e.left,e.right);if(e.action==PDE::AMRAction::refine){observedSplit=true;observedLaterSplit|=e.step>0;}else{EXPECT_EQ(e.action,PDE::AMRAction::coarsen);observedMerge=true;}}
 auto owned=c2c::vector(actual.values);auto coords=actual.x;auto again=s.solve(adaptive(a));EXPECT_EQ(c2c::vector(actual.values),owned);EXPECT_EQ(actual.x,coords);EXPECT_EQ(c2c::vector(again.values),owned);EXPECT_EQ(g.nx(),c.nx);EXPECT_EQ(g.nt(),c.nt);EXPECT_DOUBLE_EQ(s.theta(),theta);
 double einf=0,trap=0;for(std::size_t i=0;i<actual.x.size();++i){double e=actual.values[i]-f.exact(c.T,actual.x[i]);einf=std::max(einf,std::abs(e));if(i){double prev=actual.values[i-1]-f.exact(c.T,actual.x[i-1]);trap+=(actual.x[i]-actual.x[i-1])*(e*e+prev*prev)/2;}}std::cout<<"AMR descriptive fixture="<<c.f<<" theta="<<theta<<" Einf="<<einf<<" trapezoidal_L2="<<std::sqrt(trap)<<" events="<<actual.events.size()<<"\n";
 }
 EXPECT_TRUE(observedSplit);EXPECT_TRUE(observedMerge);
 // Later-transfer coverage is required; if the chosen public fixture does not
 // exercise it on a conforming oracle, improve this probe independently before sealing.
 EXPECT_TRUE(observedLaterSplit);
}
TEST(C2C_AMR,StrictIndicatorComparisonsAndAllOrNothingCapacity){
 // §7.1 polynomial exact fixture at t0 on x={0,.5,1}: I=0.25 exactly.
 auto f=c2c::fixture(2);PDE::Domain1D d(0,1,0,.01);PDE::Grid1D g(d,3,1);auto s=PDE::makeSolver1D(g,f.p);
 auto equal=s.solve(adaptive({1,5,.25,0}));EXPECT_TRUE(equal.events.empty());EXPECT_FALSE(equal.refinement_limited);EXPECT_EQ(equal.x.size(),3u);
 auto low=std::nextafter(.25,0.);auto capped=s.solve(adaptive({1,4,low,0}));EXPECT_TRUE(capped.events.empty());EXPECT_TRUE(capped.refinement_limited);EXPECT_EQ(capped.x.size(),3u);
 auto accepted=s.solve(adaptive({1,5,low,0}));ASSERT_EQ(accepted.events.size(),2u);EXPECT_EQ(accepted.x,(std::vector<double>{0,.25,.5,.75,1}));EXPECT_FALSE(accepted.refinement_limited);
 auto depth=s.solve(adaptive({0,5,low,0}));EXPECT_TRUE(depth.events.empty());EXPECT_TRUE(depth.refinement_limited);
}
TEST(C2C_AMRArithmetic,IndicatorFailuresPrecedeCaps){
 double M=std::numeric_limits<double>::max(),tiny=std::numeric_limits<double>::denorm_min();
 struct Case{double right;std::array<double,3> values;};
 for(auto c:std::vector<Case>{{2,{M,-M,M}},{2,{M/2,-M/2,M/2}},{2*tiny,{0,1,0}}}){
 PDE::Domain1D d(0,c.right,0,1);PDE::Grid1D g(d,3,1);auto p=c2c::constantData(0);p.boundaryL=c2c::constant(c.values[0]);p.initial=c2c::constant(c.values[1]);p.boundaryR=c2c::constant(c.values[2]);int coefficients=0;p.diffusion=[&](double,double){++coefficients;return 1.;};auto s=PDE::makeSolver1D(g,p);for(std::size_t depth:{0u,1u}){coefficients=0;
EXPECT_THROW(s.solve(adaptive({depth,3,1,0})),std::runtime_error);}}
}
TEST(C2C_AMRArithmetic,RecomputedIndicatorFailureAndBlockedSampling){
 PDE::Domain1D d(0,2,0,1);PDE::Grid1D g(d,3,1);auto p=c2c::constantData(0);double M=std::numeric_limits<double>::max();int coefficients=0;
 p.initial=[M](double,double x){return x==1?1.:M;};p.diffusion=[&](double,double){++coefficients;return 1.;};auto s=PDE::makeSolver1D(g,p);
 EXPECT_THROW(s.solve(adaptive({1,5,.1,0})),std::runtime_error);
 // A blocked simultaneous batch samples no inserted initial midpoint.
 auto f=c2c::fixture(2);PDE::Domain1D d2(0,1,0,.01);PDE::Grid1D g2(d2,3,1);auto initial=f.p.initial;f.p.initial=[initial](double t,double x){if(x==.25||x==.75)throw std::logic_error("inserted sample");return initial(t,x);};auto s2=PDE::makeSolver1D(g2,f.p);
 EXPECT_NO_THROW(s2.solve(adaptive({1,4,.1,0})));
 EXPECT_THROW(s2.solve(adaptive({1,5,.1,0})),std::logic_error);
}
TEST(C2C_AMRArithmetic,MidpointPreflightBeforeNodeCapacity){
 // Four representable numbers above 1: each root width is one ULP. At least
 // one requested root has no interior binary64 midpoint. Finite indicators.
 double left=1,right=left;for(int i=0;i<4;++i)right=std::nextafter(right,2.);PDE::Domain1D d(left,right,0,std::ldexp(1.,-100));PDE::Grid1D g(d,5,1);auto p=c2c::constantData(0);p.initial=[&](double,double x){return x==g.x(2)?std::ldexp(1.,-100):0.;};auto s=PDE::makeSolver1D(g,p);
EXPECT_THROW(s.solve(adaptive({1,5,std::ldexp(1.,-102),0})),std::runtime_error);
 // Exclusion by depth cap means midpoint is not evaluated.
 auto allowed=s.solve(adaptive({0,5,std::ldexp(1.,-102),0}));EXPECT_TRUE(allowed.refinement_limited);EXPECT_TRUE(allowed.events.empty());
}
TEST(C2C_AMRArithmetic,TimeScaledStencilAndFiniteUnderflow){
 struct Case{double right,dt,a,value;};for(auto c:std::vector<Case>{{2e-200,1e-300,1e-200,0},{2e200,1e-200,1e200,1},{2,1e-309,std::numeric_limits<double>::max(),1}}){PDE::Domain1D d(0,c.right,0,c.dt);PDE::Grid1D g(d,3,1);auto p=c2c::constantData(c.value);p.diffusion=c2c::constant(c.a);PDE::AMROptions a{0,3,1,0};auto expected=c2c::reference(g,p,1,a);auto s=PDE::makeSolver1D(g,p);s.setTheta(1);auto actual=s.solve(adaptive(a));c2c::compare(actual,expected);}
 // Prescribed rD overflows even though a*rD could be finite after reassociation.
 PDE::Domain1D d(0,2e-200,0,1e-50);PDE::Grid1D g(d,3,1);auto p=c2c::constantData(0);p.diffusion=c2c::constant(1e-300);auto s=PDE::makeSolver1D(g,p);
EXPECT_THROW(s.solve(adaptive({0,3,1,0})),std::runtime_error);
}
TEST(C2C_AMRArithmetic,IndependentTransferScheduleEdgeOracles){
 // API-neutral oracle anchors. Public-API later transfer is checked end-to-end
 // by the analytic-fixture suite; direct reachability of every arithmetic edge
 // is explicitly assigned to source inspection in the manifest.
 double l=1,r=l;for(int i=0;i<3;++i)r=std::nextafter(r,2.);double m=c2c::midpoint(l,r);double alpha=(m-l)/(r-l);EXPECT_NE(alpha,.5);double u=c2c::transfer(l,r,m,2,8);EXPECT_DOUBLE_EQ(u,(1-alpha)*2+alpha*8);EXPECT_NE(u,5);
 double M=std::numeric_limits<double>::max();EXPECT_TRUE(std::isfinite(c2c::transfer(0,2,1,-M,M)));EXPECT_DOUBLE_EQ(c2c::transfer(0,2,1,-M,M),0);
EXPECT_THROW(c2c::indicators({0,1,2},{M,-M,M}),std::runtime_error);
}
}
