#ifndef C2C_ACCEPTANCE_SUPPORT_H
#define C2C_ACCEPTANCE_SUPPORT_H
#include <pde_solver_amr_core/PDE.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#if defined(__VERSION__)
#define C2C_COMPILER_ID __VERSION__
#elif defined(_MSC_VER)
#define C2C_STRINGIFY_IMPL(x) #x
#define C2C_STRINGIFY(x) C2C_STRINGIFY_IMPL(x)
#define C2C_COMPILER_ID "MSVC " C2C_STRINGIFY(_MSC_VER)
#else
#define C2C_COMPILER_ID "compiler identity recorded in build evidence"
#endif
namespace c2c {
inline char const* sourceRevision(){auto value=std::getenv("C2C_SOURCE_REVISION");return value?value:"see source-revision record in canonical execution evidence";}
inline char const* buildIdentity(){
#ifdef NDEBUG
return "Release/NDEBUG";
#else
return "Debug/assertions";
#endif
}
using PDE::Array1D;
inline double pi(){return std::acos(-1.0);}
inline PDE::Func::F1D constant(double v){return [v](double,double){return v;};}
inline PDE::PDE1D constantData(double v=1){PDE::PDE1D p;p.diffusion=constant(1);p.advection=constant(0);p.reaction=constant(0);p.source=constant(0);p.initial=constant(v);p.boundaryL=constant(v);p.boundaryR=constant(v);return p;}
inline std::vector<double> vector(Array1D const& a){return {a.begin(),a.end()};}
inline Array1D array(std::vector<double> const& v){Array1D a(v.size());std::copy(v.begin(),v.end(),a.begin());return a;}
using Field=PDE::Func::F1D PDE::PDE1D::*;
inline std::array<Field,7> fields(){return {&PDE::PDE1D::diffusion,&PDE::PDE1D::advection,&PDE::PDE1D::reaction,&PDE::PDE1D::source,&PDE::PDE1D::initial,&PDE::PDE1D::boundaryL,&PDE::PDE1D::boundaryR};}
struct Fixture { PDE::PDE1D p; PDE::Func::F1D exact; };
inline Fixture fixture(int which){
 Fixture f;f.p=constantData();
 if(which==0||which==1){
  f.p.diffusion=[](double t,double x){return 1+.2*t+.1*x;};
  f.p.advection=[](double t,double x){return .3+.1*t-.2*x;};
  f.p.reaction=[](double t,double x){return -.4+.2*t+.1*x;};
  f.exact=[which](double t,double x){return std::exp(t)*(which==0?1+x+std::sin(pi()*x):1+x+x*x);};
  f.p.source=[which](double t,double x){double a=1+.2*t+.1*x,b=.3+.1*t-.2*x,c=-.4+.2*t+.1*x;double P=which==0?1+x+std::sin(pi()*x):1+x+x*x;return std::exp(t)*((1-c)*P+(which==0?a*pi()*pi()*std::sin(pi()*x):-2*a)-b*(which==0?1+pi()*std::cos(pi()*x):1+2*x));};
 }else if(which==2){f.exact=[](double t,double x){return std::exp(t)*x*(1-x);};f.p.source=[](double t,double x){return std::exp(t)*(x*(1-x)+2);};}
 else {f.exact=[](double t,double x){return std::exp(-pi()*pi()*t)*std::sin(pi()*x);};}
 f.p.initial=f.exact;f.p.boundaryL=f.exact;f.p.boundaryR=f.exact;
 if(which==3){f.p.boundaryL=constant(0);f.p.boundaryR=constant(0);}
 return f;
}
struct Errors {double inf,l2;};
inline Errors errors(PDE::Grid1D const& g,Array1D const& a,PDE::Func::F1D const& exact){double emax=0,sum=0;for(std::size_t i=0;i<a.size();++i){double e=a[i]-exact(g.domain().t1,g.x(i));emax=std::max(emax,std::abs(e));sum+=e*e;}return {emax,std::sqrt(g.dx()*sum)};}
inline void report(char const* id,PDE::Grid1D const& g,double theta,Errors e){std::cout<<std::setprecision(17)<<"C2C fixture="<<id<<" domain=["<<g.domain().xL<<","<<g.domain().xR<<"]x["<<g.domain().t0<<","<<g.domain().t1<<"] nx="<<g.nx()<<" nt="<<g.nt()<<" theta="<<theta<<" Einf="<<e.inf<<" E2="<<e.l2<<" compiler="<<C2C_COMPILER_ID<<" build="<<buildIdentity()<<" source_revision="<<sourceRevision()<<"\n";}
inline double checked(double v){if(!std::isfinite(v))throw std::runtime_error("independent oracle nonfinite");return v;}
inline double add(double a,double b){volatile double v=a+b;return checked(v);}
inline double sub(double a,double b){volatile double v=a-b;return checked(v);}
inline double mul(double a,double b){volatile double v=a*b;return checked(v);}
inline double div(double a,double b){volatile double v=a/b;return checked(v);}
inline double mean(double a,double b){double h=std::numeric_limits<double>::max()/2;return std::abs(a)<=h&&std::abs(b)<=h?div(add(a,b),2):add(div(a,2),div(b,2));}
// Independent binary64 Thomas oracle. No production solver calls.
inline std::vector<double> thomas(std::vector<double> l,std::vector<double> d,std::vector<double> u,std::vector<double> y){auto orig=d;std::size_t n=d.size();double tau=64*std::numeric_limits<double>::epsilon();for(std::size_t j=0;j<n;++j){double rho=std::abs(orig[j]);if(j)rho=std::max(rho,std::abs(l[j-1]));if(j+1<n)rho=std::max(rho,std::abs(u[j]));if(j){double w=div(l[j-1],d[j-1]);d[j]=sub(d[j],mul(w,u[j-1]));if(rho==0||d[j]==0||(std::abs(d[j])<rho&&div(std::abs(d[j]),rho)<=tau))throw std::runtime_error("oracle pivot");y[j]=sub(y[j],mul(w,y[j-1]));}else if(rho==0||d[j]==0||(std::abs(d[j])<rho&&div(std::abs(d[j]),rho)<=tau))throw std::runtime_error("oracle pivot");}y[n-1]=div(y[n-1],d[n-1]);for(std::size_t j=n-1;j-->0;)y[j]=div(sub(y[j],mul(u[j],y[j+1])),d[j]);return y;}
}
#endif
