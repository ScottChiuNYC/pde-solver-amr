#pragma once
#include <cmath>
#include <stdexcept>
#include <string>
namespace PDE { namespace detail {
inline double finite(double v, std::string const& context) {
    if (!std::isfinite(v)) throw std::runtime_error(context+": nonfinite result");
    return v;
}
// Volatile binary64 stores force a rounding boundary between each scheduled
// operation, including for clients whose compiler otherwise contracts multiply/add.
inline double add(double a,double b,std::string const& c) { volatile double r=a+b; return finite(r,c+" addition"); }
inline double sub(double a,double b,std::string const& c) { volatile double r=a-b; return finite(r,c+" subtraction"); }
inline double mul(double a,double b,std::string const& c) { volatile double r=a*b; return finite(r,c+" product"); }
inline double div(double a,double b,std::string const& c) { volatile double r=a/b; return finite(r,c+" division"); }
inline double positive(double v,std::string const& c) { finite(v,c); if (!(v>0)) throw std::runtime_error(c+": must be positive"); return v; }
}}
