#pragma once
#include "Domain.h"
#include <cstddef>
#include <vector>
namespace PDE {
class Grid1D {
public:
    Grid1D(Domain1D const& d,std::size_t nx,std::size_t nt):m_domain(d),m_nx(nx),m_nt(nt) {
        if(nx<3 || nx>std::vector<double>().max_size() || nt<1)throw std::runtime_error("grid nx/nt cardinality");
        m_domain.validate();
        m_dx=detail::positive(detail::div(detail::sub(d.xR,d.xL,"grid spatial span"),double(nx-1),"grid dx"),"grid dx");
        m_dt=detail::positive(detail::div(detail::sub(d.t1,d.t0,"grid time span"),double(nt),"grid dt"),"grid dt");
    }
    std::size_t nx() const {return m_nx;} std::size_t nt() const {return m_nt;}
    double dx() const {return m_dx;} double dt() const {return m_dt;}
    double x(std::size_t i) const {
        if(i>=m_nx)throw std::runtime_error("grid x index "+std::to_string(i));
        if(i==0)return m_domain.xL;if(i==m_nx-1)return m_domain.xR;
        return detail::add(m_domain.xL,detail::mul(double(i),m_dx,"grid x coordinate"),"grid x coordinate");
    }
    double t(std::size_t k) const {
        if(k>m_nt)throw std::runtime_error("grid t index "+std::to_string(k));
        if(k==0)return m_domain.t0;if(k==m_nt)return m_domain.t1;
        return detail::add(m_domain.t0,detail::mul(double(k),m_dt,"grid t coordinate"),"grid t coordinate");
    }
    Domain1D const& domain() const {return m_domain;}
private:
    Domain1D m_domain;std::size_t m_nx,m_nt;double m_dx,m_dt;
};
}
