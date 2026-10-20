#include "c2c_acceptance_support.h"
namespace {
TEST(C2C_Section7,AllNumericalLadders){
 struct Case {int fixture;double T,theta;std::array<std::size_t,3> nx,nt;double lo,hi,ei,e2;char const* name;};
 const Case cases[]={
 {0,.1,.5,{21,41,81},{4096,4096,4096},1.85,2.15,2e-4,2e-4,"spatial-all-terms"},
 {1,.2,.5,{41,41,41},{20,40,80},1.8,2.2,2e-6,2e-6,"temporal-CN"},
 {2,.2,.75,{41,41,41},{10,20,40},.85,1.15,1e-4,7e-5,"temporal-theta075"},
 {2,.2,1,{41,41,41},{10,20,40},.85,1.15,1e-4,7e-5,"temporal-theta1"}};
 for(auto const& c:cases){SCOPED_TRACE(c.name);std::array<c2c::Errors,3> e;for(int j=0;j<3;++j){auto f=c2c::fixture(c.fixture);PDE::Domain1D d(0,1,0,c.T);PDE::Grid1D g(d,c.nx[j],c.nt[j]);auto s=PDE::makeSolver1D(g,f.p);s.setTheta(c.theta);auto a=s.solve();ASSERT_EQ(a.size(),g.nx());e[j]=c2c::errors(g,a,f.exact);c2c::report(c.name,g,c.theta,e[j]);ASSERT_TRUE(std::isfinite(e[j].inf));ASSERT_TRUE(std::isfinite(e[j].l2));ASSERT_GT(e[j].inf,0);ASSERT_GT(e[j].l2,0);}for(int j=0;j<2;++j){double ri=std::log2(e[j].inf/e[j+1].inf),r2=std::log2(e[j].l2/e[j+1].l2);std::cout<<c.name<<" adjacent="<<j<<" rate_inf="<<ri<<" rate_l2="<<r2<<"\n";EXPECT_GE(ri,c.lo);EXPECT_LE(ri,c.hi);EXPECT_GE(r2,c.lo);EXPECT_LE(r2,c.hi);}EXPECT_LE(e[2].inf,c.ei);EXPECT_LE(e[2].l2,c.e2);}
}
TEST(C2C_Section7,FixedHeatBenchmark){auto f=c2c::fixture(3);PDE::Domain1D d(0,1,0,.1);PDE::Grid1D g(d,101,1000);auto s=PDE::makeSolver1D(g,f.p);auto e=c2c::errors(g,s.solve(),f.exact);c2c::report("fixed-heat",g,.5,e);ASSERT_TRUE(std::isfinite(e.inf));ASSERT_TRUE(std::isfinite(e.l2));EXPECT_LE(e.inf,4e-5);EXPECT_LE(e.l2,3e-5);}
TEST(C2C_Section7,OneRowCornerPrecedenceAndWeights){PDE::Domain1D d(0,2,0,1);PDE::Grid1D g(d,3,1);auto p=c2c::constantData();p.diffusion=[](double t,double){return 1+t;};p.advection=[](double t,double){return 2*t;};p.reaction=[](double t,double){return -2*t;};p.source=[](double t,double){return 2+4*t;};p.initial=[](double,double x){if(x==0||x==2)throw std::logic_error("initial endpoint");return 2.;};p.boundaryL=[](double t,double){return 1+2*t;};p.boundaryR=[](double t,double){return 5-t;};auto s=PDE::makeSolver1D(g,p);for(double theta:{.5,.75,1.}){s.setTheta(theta);auto a=s.solve();ASSERT_EQ(a.size(),3u);EXPECT_DOUBLE_EQ(a[0],3);EXPECT_DOUBLE_EQ(a[2],4);EXPECT_NEAR(a[1],(7+12*theta)/(1+4*theta),1e-12);}}
TEST(C2C_Section7,HeatModeAmplification){PDE::Domain1D d(0,1,0,1);PDE::Grid1D g(d,21,1);auto p=c2c::constantData(0);p.initial=[](double,double x){return std::sin(19*c2c::pi()*x);};auto s=PDE::makeSolver1D(g,p);double h=1./20,lambda=-4*std::pow(std::sin(19*c2c::pi()/40),2)/(h*h);for(double theta:{.5,.75,1.}){s.setTheta(theta);double G=(1+(1-theta)*lambda)/(1-theta*lambda);EXPECT_LE(std::abs(G),1);auto a=s.solve();for(std::size_t i=1;i<20;++i)EXPECT_NEAR(a[i],G*std::sin(19*c2c::pi()*g.x(i)),2e-11);EXPECT_DOUBLE_EQ(a[0],0);EXPECT_DOUBLE_EQ(a[20],0);}}
}
