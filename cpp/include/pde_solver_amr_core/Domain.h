#pragma once
#include "Checked.h"
namespace PDE {
struct Domain1D {
    double xL,xR,t0,t1;
    Domain1D(double l,double r,double a,double b):xL(l),xR(r),t0(a),t1(b) { validate(); }
private:
    friend class Grid1D;
    void validate() const {
        detail::finite(xL,"domain xL");detail::finite(xR,"domain xR");
        detail::finite(t0,"domain t0");detail::finite(t1,"domain t1");
        if(!(xR>xL && t1>t0))throw std::runtime_error("domain endpoint order");
        detail::positive(detail::sub(xR,xL,"domain spatial span"),"domain spatial span");
        detail::positive(detail::sub(t1,t0,"domain time span"),"domain time span");
    }
};
}
