#pragma once
#include "Grid.h"
#include "Equation.h"
#include "Tridiagonal.h"
#include <algorithm>
#include <limits>
#include <string>
#include <utility>
namespace PDE { namespace detail {
inline void validatePDE(PDE1D const& p) {
    for (auto type:p.boundaryConfig.types)
        if(type!=BoundaryType::Dirichlet)throw std::runtime_error("unsupported boundary type");
    if(!p.diffusion || !p.advection || !p.reaction || !p.source || !p.initial || !p.boundaryL || !p.boundaryR)
        throw std::runtime_error("missing PDE callback");
}
inline double sample(Func::F1D const& f,double t,double x,std::string const& role) {
    return finite(f(t,x),role+" callback t="+std::to_string(t)+" x="+std::to_string(x));
}
inline double mean(double p,double q,std::string const& c) {
    const double halfMax=std::numeric_limits<double>::max()/2;
    if(std::abs(p)<=halfMax && std::abs(q)<=halfMax) {
        const double sum=add(p,q,c+" mean");return div(sum,2,c+" mean");
    }
    const double a=div(p,2,c+" mean");
    const double b=div(q,2,c+" mean");
    return add(a,b,c+" mean");
}
inline double coefficient(Func::F1D const& f,double oldTime,double newTime,double x,
                          std::string const& c,bool diffusion=false) {
    const double a=sample(f,oldTime,x,c);
    if(diffusion)positive(a,c+" diffusion sample");
    const double b=sample(f,newTime,x,c);
    if(diffusion)positive(b,c+" diffusion sample");
    return mean(a,b,c);
}
inline Array1D initialize(Grid1D const& g,PDE1D const& p,std::vector<double> const& x) {
    Array1D u(x.size());
    u[0]=sample(p.boundaryL,g.t(0),g.domain().xL,"boundaryL");
    for(std::size_t i=1;i+1<x.size();++i)u[i]=sample(p.initial,g.t(0),x[i],"initial");
    u[x.size()-1]=sample(p.boundaryR,g.t(0),g.domain().xR,"boundaryR");
    return u;
}
inline Array1D step(Grid1D const& g,PDE1D const& p,double theta,std::size_t n,
                    std::vector<double> const& x,Array1D const& old,bool adaptive) {
    const auto m=x.size()-2;
    Array1D lower(m-1),diag(m),upper(m-1),rhs(m),next(x.size());
    const double oldTime=g.t(n),newTime=g.t(n+1),dt=g.dt();
    const double left=sample(p.boundaryL,newTime,g.domain().xL,"boundaryL");
    const double right=sample(p.boundaryR,newTime,g.domain().xR,"boundaryR");
    const auto stepContext="step "+std::to_string(n+1);
    const double omega=sub(1,theta,stepContext+" theta weight");
    double r=0,s=0;
    if(!adaptive) {
        const double v=div(dt,g.dx(),stepContext+" uniform geometry");
        r=div(v,g.dx(),stepContext+" uniform geometry");
        s=div(v,2,stepContext+" uniform geometry");
    }
    for(std::size_t i=1;i+1<x.size();++i) {
        const auto c=stepContext+" row "+std::to_string(i);
        double hL=0,hR=0,H=0;
        if(adaptive) {
            hL=positive(sub(x[i],x[i-1],c+" hL"),c+" hL");
            hR=positive(sub(x[i+1],x[i],c+" hR"),c+" hR");
            H=positive(add(hL,hR,c+" H"),c+" H");
        }
        const double a=coefficient(p.diffusion,oldTime,newTime,x[i],c+" diffusion",true);
        const double b=coefficient(p.advection,oldTime,newTime,x[i],c+" advection");
        const double reaction=coefficient(p.reaction,oldTime,newTime,x[i],c+" reaction");
        double qL,qD,qR;
        if(adaptive) {
            const double vL=div(dt,hL,c+" vL");
            const double vR=div(dt,hR,c+" vR");
            const double rL=div(vL,H,c+" rL");
            const double rR=div(vR,H,c+" rR");
            const double rD=div(vL,hR,c+" rD");
            const double zL=div(hR,H,c+" zL");
            const double zR=div(hL,H,c+" zR");
            const double sL=mul(vL,zL,c+" sL");
            const double sR=mul(vR,zR,c+" sR");
            const double sD=sub(vL,vR,c+" sD");
            const double arL=mul(a,rL,c+" arL");
            const double arD=mul(a,rD,c+" arD");
            const double arR=mul(a,rR,c+" arR");
            const double bsL=mul(b,sL,c+" bsL");
            const double bsD=mul(b,sD,c+" bsD");
            const double bsR=mul(b,sR,c+" bsR");
            const double cdt=mul(reaction,dt,c+" cdt");
            const double dL=mul(2,arL,c+" dL");
            const double dD=mul(-2,arD,c+" dD");
            const double dR=mul(2,arR,c+" dR");
            qL=sub(dL,bsL,c+" qL");
            const double qD0=add(dD,bsD,c+" qD0");
            qD=add(qD0,cdt,c+" qD");
            qR=add(dR,bsR,c+" qR");
        } else {
            const double ar=mul(a,r,c+" ar");
            const double bs=mul(b,s,c+" bs");
            const double cdt=mul(reaction,dt,c+" cdt");
            qL=sub(ar,bs,c+" qL");
            const double dD=mul(-2,ar,c+" qD");
            qD=add(dD,cdt,c+" qD");
            qR=add(ar,bs,c+" qR");
        }
        const double tqL=mul(theta,qL,c+" implicit left");
        const double AL=finite(-tqL,c+" implicit left negation");
        const double tqD=mul(theta,qD,c+" implicit diagonal");
        const double AD=sub(1,tqD,c+" implicit diagonal");
        const double tqR=mul(theta,qR,c+" implicit right");
        const double AR=finite(-tqR,c+" implicit right negation");
        const double oqL=mul(omega,qL,c+" old left weight");
        const double oldL=mul(oqL,old[i-1],c+" old left term");
        const double oqD=mul(omega,qD,c+" old diagonal weight");
        const double oldDiagonalWeight=add(1,oqD,c+" old diagonal weight");
        const double oldD=mul(oldDiagonalWeight,old[i],c+" old diagonal term");
        const double oqR=mul(omega,qR,c+" old right weight");
        const double oldR=mul(oqR,old[i+1],c+" old right term");
        double oldSource=0;
        if(theta<1)oldSource=mul(omega,sample(p.source,oldTime,x[i],c+" source"),c+" old source");
        const double newSource=mul(theta,sample(p.source,newTime,x[i],c+" source"),c+" new source");
        const double sourceSum=add(oldSource,newSource,c+" source sum");
        const double sourceTerm=mul(sourceSum,dt,c+" source term");
        const double leftDiagonal=add(oldL,oldD,c+" RHS left/diagonal");
        const double oldSum=add(leftDiagonal,oldR,c+" RHS right");
        double value=add(oldSum,sourceTerm,c+" RHS source");
        if(i==1){const double correction=mul(AL,left,c+" left boundary correction");value=sub(value,correction,c+" left boundary correction");}
        else lower[i-2]=AL;
        if(i+2==x.size()){const double correction=mul(AR,right,c+" right boundary correction");value=sub(value,correction,c+" right boundary correction");}
        else upper[i-1]=AR;
        diag[i-1]=AD;rhs[i-1]=value;
    }
    TridiagonalSolver::solve(lower,diag,upper,rhs);
    next[0]=left;next[next.size()-1]=right;
    for(std::size_t i=0;i<m;++i)next[i+1]=rhs[i];
    return next;
}

// Each leaf has a root and an arbitrary-length dyadic path. This preserves
// sibling identity at all representable depths without integer bit overflow.
struct Leaf {
    std::size_t root;
    std::vector<unsigned char> path;
    bool fresh=false;
};
inline bool siblings(Leaf const& left,Leaf const& right) {
    if(left.root!=right.root || left.path.empty() || left.path.size()!=right.path.size() ||
       left.path.back()!=0 || right.path.back()!=1)return false;
    return std::equal(left.path.begin(),left.path.end()-1,right.path.begin());
}
inline std::vector<double> indicators(std::vector<double> const& x,Array1D const& u,
                                      std::size_t step,std::string const& pass) {
    const auto m=x.size();
    std::vector<double> h(m-1),s(m-1),curvature(m),indicator(m-1);
    const auto context="adaptive indicator step "+std::to_string(step)+" "+pass;
    for(std::size_t i=0;i+1<m;++i) {
        const auto c=context+" cell "+std::to_string(i);
        h[i]=positive(sub(x[i+1],x[i],c+" spacing"),c+" spacing");
        const double dU=sub(u[i+1],u[i],c+" value difference");
        s[i]=div(dU,h[i],c+" slope");
    }
    for(std::size_t i=1;i+1<m;++i) {
        const auto c=context+" node "+std::to_string(i);
        const double jump=sub(s[i],s[i-1],c+" slope jump");
        const double magnitude=finite(std::abs(jump),c+" magnitude");
        const double spacing=finite(std::min(h[i-1],h[i]),c+" minimum spacing");
        const double half=div(spacing,2,c+" half spacing");
        curvature[i]=mul(half,magnitude,c+" curvature");
    }
    curvature[0]=finite(curvature[1],context+" left curvature copy");
    curvature[m-1]=finite(curvature[m-2],context+" right curvature copy");
    for(std::size_t i=0;i+1<m;++i)
        indicator[i]=finite(std::max(curvature[i],curvature[i+1]),context+" cell "+std::to_string(i)+" maximum indicator");
    return indicator;
}
inline void adapt(Grid1D const& grid,PDE1D const& p,AMROptions const& options,
                  std::size_t step,Solution1D& result,std::vector<Leaf>& leaves) {
    auto const indicator=indicators(result.x,result.values,step,"pre-refinement");
    const auto cells=leaves.size();
    std::vector<bool> split(cells,false);
    std::vector<double> widths(cells),midpoints(cells);
    std::size_t count=0;
    for(std::size_t i=0;i<cells;++i) {
        leaves[i].fresh=false;
        if(indicator[i]>options.refine_tolerance) {
            if(leaves[i].path.size()>=options.max_refinement_level)result.refinement_limited=true;
            else {split[i]=true;++count;}
        }
    }
    // Complete the entire left-to-right midpoint preflight before capacity or
    // callbacks/transfer. Retain these exact rounded coordinates and widths.
    for(std::size_t i=0;i<cells;++i)if(split[i]) {
        const double left=result.x[i],right=result.x[i+1];
        const auto c="adaptive midpoint representability step "+std::to_string(step)+" parent ["+std::to_string(left)+","+std::to_string(right)+"]";
        widths[i]=positive(sub(right,left,c+" width"),c+" width");
        const double half=div(widths[i],2,c+" half");
        midpoints[i]=add(left,half,c+" midpoint");
        if(!(left<midpoints[i] && midpoints[i]<right))throw std::runtime_error(c);
    }
    if(count>options.max_nodes-result.x.size())result.refinement_limited=true;
    else if(count>0) {
        std::vector<double> x;Array1D u(result.x.size()+count);
        std::vector<Leaf> refined;std::vector<AMREvent> events;
        x.reserve(u.size());refined.reserve(cells+count);events.reserve(count);
        std::size_t j=0;
        for(std::size_t i=0;i<cells;++i) {
            x.push_back(result.x[i]);u[j++]=result.values[i];
            if(!split[i]){refined.push_back(leaves[i]);continue;}
            const double mid=midpoints[i],left=result.x[i],right=result.x[i+1];
            const auto c="adaptive later-refinement transfer step "+std::to_string(step)+" parent ["+std::to_string(left)+","+std::to_string(right)+"]";
            double inserted;
            if(step==0)inserted=sample(p.initial,grid.t(0),mid,"initial inserted midpoint");
            else {
                const double offset=sub(mid,left,c+" offset");
                const double alpha=div(offset,widths[i],c+" alpha");
                const double beta=sub(1,alpha,c+" beta");
                const double pL=mul(beta,result.values[i],c+" pL");
                const double pR=mul(alpha,result.values[i+1],c+" pR");
                inserted=add(pL,pR,c+" inserted");
            }
            x.push_back(mid);u[j++]=inserted;
            Leaf l=leaves[i],r=leaves[i];
            l.path.push_back(0);r.path.push_back(1);l.fresh=r.fresh=true;
            refined.push_back(std::move(l));refined.push_back(std::move(r));
            events.push_back({step,grid.t(step),AMRAction::refine,left,right});
        }
        x.push_back(result.x.back());u[j]=result.values[result.values.size()-1];
        // Commit batch only once every inserted value has succeeded.
        result.events.insert(result.events.end(),events.begin(),events.end());
        result.x.swap(x);result.values.swap(u);leaves.swap(refined);
    }
    const auto coarsenIndicators=indicators(result.x,result.values,step,"pre-coarsening");
    std::vector<double> x;std::vector<double> values;std::vector<Leaf> merged;
    std::vector<AMREvent> events;
    x.reserve(result.x.size());values.reserve(result.x.size());merged.reserve(leaves.size());
    for(std::size_t i=0;i<leaves.size();) {
        x.push_back(result.x[i]);values.push_back(result.values[i]);
        if(i+1<leaves.size() && !leaves[i].fresh && !leaves[i+1].fresh &&
           siblings(leaves[i],leaves[i+1]) &&
           coarsenIndicators[i]<options.coarsen_tolerance && coarsenIndicators[i+1]<options.coarsen_tolerance) {
            Leaf parent=leaves[i];parent.path.pop_back();
            merged.push_back(std::move(parent));
            events.push_back({step,grid.t(step),AMRAction::coarsen,result.x[i],result.x[i+2]});
            i+=2;
        } else {merged.push_back(leaves[i]);++i;}
    }
    x.push_back(result.x.back());values.push_back(result.values[result.values.size()-1]);
    Array1D u(values.size());for(std::size_t i=0;i<values.size();++i)u[i]=values[i];
    result.events.insert(result.events.end(),events.begin(),events.end());
    result.x.swap(x);result.values.swap(u);leaves.swap(merged);
}
} namespace Scheme {
class CrankNicholson1D {
public:
    CrankNicholson1D(Grid1D const& g,PDE1D const& p):m_grid(g),m_pde(p) {}
    CrankNicholson1D(CrankNicholson1D const&)=default;
    CrankNicholson1D(CrankNicholson1D&&)=default;
    CrankNicholson1D& operator=(CrankNicholson1D const&)=delete;
    CrankNicholson1D& operator=(CrankNicholson1D&&)=delete;
    void setTheta(double t) {if(!std::isfinite(t)||t<.5||t>1)throw std::runtime_error("theta outside finite [.5,1]");m_theta=t;}
    double theta() const {return m_theta;}
    Array1D solve() const {
        detail::validatePDE(m_pde);
        std::vector<double> x(m_grid.nx());
        for(std::size_t i=0;i<x.size();++i)x[i]=m_grid.x(i);
        auto u=detail::initialize(m_grid,m_pde,x);
        for(std::size_t n=0;n<m_grid.nt();++n)u=detail::step(m_grid,m_pde,m_theta,n,x,u,false);
        return u;
    }
    Solution1D solve(SolveOptions1D const& options) const {
        if(options.mesh_mode!=MeshMode::Uniform && options.mesh_mode!=MeshMode::Adaptive)
            throw std::runtime_error("unknown mesh mode");
        if(options.mesh_mode==MeshMode::Uniform) {
            Solution1D result;
            result.values=solve();
            result.x.resize(m_grid.nx());
            for(std::size_t i=0;i<result.x.size();++i)result.x[i]=m_grid.x(i);
            return result;
        }
        if(!options.amr)throw std::runtime_error("Adaptive mode requires AMR options");
        const auto& a=*options.amr;
        if(a.max_nodes<m_grid.nx() || !std::isfinite(a.refine_tolerance) || !(a.refine_tolerance>0) ||
           !std::isfinite(a.coarsen_tolerance) || a.coarsen_tolerance<0 || !(a.coarsen_tolerance<a.refine_tolerance))
            throw std::runtime_error("invalid AMR node cap or indicator tolerances");
        detail::validatePDE(m_pde);
        Solution1D result;result.x.resize(m_grid.nx());
        for(std::size_t i=0;i<result.x.size();++i)result.x[i]=m_grid.x(i);
        result.values=detail::initialize(m_grid,m_pde,result.x);
        std::vector<detail::Leaf> leaves;leaves.reserve(result.x.size()-1);
        for(std::size_t i=0;i+1<result.x.size();++i)leaves.push_back({i,{},false});
        detail::adapt(m_grid,m_pde,a,0,result,leaves);
        for(std::size_t n=0;n<m_grid.nt();++n) {
            result.values=detail::step(m_grid,m_pde,m_theta,n,result.x,result.values,true);
            if(n+1<m_grid.nt())detail::adapt(m_grid,m_pde,a,n+1,result,leaves);
        }
        return result;
    }
private:
    Grid1D const& m_grid;PDE1D const& m_pde;double m_theta=.5;
};
}
inline Scheme::CrankNicholson1D makeSolver1D(Grid1D const& g,PDE1D const& p) {return Scheme::CrankNicholson1D(g,p);}
}
