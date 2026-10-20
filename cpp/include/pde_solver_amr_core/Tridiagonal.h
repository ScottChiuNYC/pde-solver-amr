#pragma once
#include "Array.h"
#include "Checked.h"
#include <algorithm>
#include <limits>
#include <string>
namespace PDE {
class TridiagonalSolver {
public:
    static void solve(Array1D const& lower,Array1D const& diagonal,Array1D const& upper,Array1D& rhs) {
        if(&rhs==&lower || &rhs==&diagonal || &rhs==&upper)throw std::runtime_error("Thomas RHS/coefficient alias");
        const auto n=diagonal.size();
        if(n==0 || rhs.size()!=n || lower.size()!=n-1 || upper.size()!=n-1)throw std::runtime_error("Thomas shape");
        // Validate all original inputs before scratch allocation or elimination.
        for(std::size_t j=0;j<n;++j) {
            const auto c="Thomas input row "+std::to_string(j);
            detail::finite(diagonal[j],c+" diagonal");detail::finite(rhs[j],c+" RHS");
            if(j>0)detail::finite(lower[j-1],c+" lower");
            if(j+1<n)detail::finite(upper[j],c+" upper");
        }
        Array1D p(n),y(n),x(n);
        const double tau=64*std::numeric_limits<double>::epsilon();
        const auto pivot=[&](std::size_t j) {
            const auto c="Thomas pivot row "+std::to_string(j);
            double rho=std::abs(diagonal[j]);
            if(j>0)rho=std::max(rho,std::abs(lower[j-1]));
            if(j+1<n)rho=std::max(rho,std::abs(upper[j]));
            detail::finite(p[j],c);
            const double a=std::abs(p[j]);
            if(rho==0 || a==0 || (a<rho && !(detail::div(a,rho,c+" relative test")>tau)))throw std::runtime_error(c+": unusable original-row relative pivot");
        };
        p[0]=diagonal[0];pivot(0);y[0]=rhs[0];
        for(std::size_t j=1;j<n;++j) {
            const auto c="Thomas elimination row "+std::to_string(j);
            const double w=detail::div(lower[j-1],p[j-1],c);
            const double wu=detail::mul(w,upper[j-1],c);
            p[j]=detail::sub(diagonal[j],wu,c);pivot(j);
            const double wy=detail::mul(w,y[j-1],c);
            y[j]=detail::sub(rhs[j],wy,c);
        }
        x[n-1]=detail::div(y[n-1],p[n-1],"Thomas substitution row "+std::to_string(n-1));
        for(std::size_t j=n-1;j>0;) {
            --j;const auto c="Thomas substitution row "+std::to_string(j);
            const double ux=detail::mul(upper[j],x[j+1],c);
            const double numerator=detail::sub(y[j],ux,c);
            x[j]=detail::div(numerator,p[j],c);
        }
        rhs.swap(x);
    }
};
}
