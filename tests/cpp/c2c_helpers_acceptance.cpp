#include "c2c_acceptance_support.h"
namespace {
using PDE::Array1D;
static_assert(std::is_copy_constructible<PDE::Scheme::CrankNicholson1D>::value,"scheme copy");
static_assert(std::is_move_constructible<PDE::Scheme::CrankNicholson1D>::value,"scheme move");
static_assert(!std::is_copy_assignable<PDE::Scheme::CrankNicholson1D>::value,"scheme no assignment");
static_assert(!std::is_move_assignable<PDE::Scheme::CrankNicholson1D>::value,"scheme no assignment");
TEST(C2C_Helpers,ArrayAllFormsOwnershipAndBounds){
 for(std::size_t n:{0u,1u,3u}){Array1D a(n,2.5);Array1D const& c=a;EXPECT_EQ(a.size(),n);EXPECT_EQ(std::distance(a.begin(),a.end()),static_cast<std::ptrdiff_t>(n));EXPECT_EQ(std::distance(c.begin(),c.end()),static_cast<std::ptrdiff_t>(n));if(n==0){EXPECT_EQ(a.data(),nullptr);EXPECT_EQ(c.data(),nullptr);}for(std::size_t i=0;i<n;++i){EXPECT_DOUBLE_EQ(c[i],2.5);EXPECT_DOUBLE_EQ(c.at(i),2.5);EXPECT_EQ(&a[i],a.data()+i);EXPECT_EQ(&c.at(i),c.data()+i);a.at(i)=3+i;a[i]=4+i;}
 auto before=c2c::vector(a);
EXPECT_THROW(a[n],std::runtime_error);
EXPECT_THROW(c[n],std::runtime_error);
EXPECT_THROW(a.at(n),std::runtime_error);
EXPECT_THROW(c.at(n),std::runtime_error);EXPECT_EQ(c2c::vector(a),before);
 auto ptr=a.data();a.fill(7);EXPECT_EQ(a.data(),ptr);for(double v:c)EXPECT_DOUBLE_EQ(v,7);Array1D copy(a);if(n){copy[0]=8;EXPECT_DOUBLE_EQ(a[0],7);}Array1D assigned;assigned=a;EXPECT_EQ(c2c::vector(assigned),c2c::vector(a));if(n)EXPECT_NE(assigned.data(),a.data());auto copyValues=c2c::vector(copy);Array1D moved(std::move(copy));EXPECT_EQ(c2c::vector(moved),copyValues);EXPECT_EQ(copy.size(),0u);EXPECT_EQ(copy.data(),nullptr);Array1D movedAssigned;movedAssigned=std::move(assigned);EXPECT_EQ(c2c::vector(movedAssigned),c2c::vector(a));EXPECT_EQ(assigned.size(),0u);EXPECT_EQ(assigned.data(),nullptr);auto old=c2c::vector(movedAssigned);movedAssigned=std::move(movedAssigned);EXPECT_EQ(c2c::vector(movedAssigned),old);Array1D other(1,99);double* ref=n?&a[0]:nullptr;a.swap(other);EXPECT_EQ(a.size(),1u);EXPECT_DOUBLE_EQ(a[0],99);EXPECT_EQ(other.size(),n);if(n){EXPECT_EQ(&other[0],ref);EXPECT_DOUBLE_EQ(*ref,7);}}
 Array1D empty;EXPECT_EQ(empty.size(),0u);EXPECT_EQ(empty.data(),nullptr);Array1D nonfinite(1,std::numeric_limits<double>::infinity());EXPECT_TRUE(std::isinf(nonfinite[0]));nonfinite[0]=std::numeric_limits<double>::quiet_NaN();EXPECT_TRUE(std::isnan(nonfinite[0]));
}
TEST(C2C_Helpers,GridCopiesIndicesAndCoordinates){
 PDE::Grid1D g=[](){PDE::Domain1D d(-2,3,4,6);PDE::Grid1D v(d,6,4);d.xL=-999;return v;}();PDE::Grid1D copy=g;EXPECT_DOUBLE_EQ(copy.dx(),1);EXPECT_DOUBLE_EQ(copy.dt(),.5);EXPECT_EQ(copy.nx(),6u);EXPECT_EQ(copy.nt(),4u);for(std::size_t i=0;i<6;++i)EXPECT_DOUBLE_EQ(copy.x(i),-2+double(i));for(std::size_t k=0;k<=4;++k)EXPECT_DOUBLE_EQ(copy.t(k),4+.5*double(k));EXPECT_DOUBLE_EQ(copy.domain().xL,-2);
EXPECT_THROW(copy.x(6),std::runtime_error);
EXPECT_THROW(copy.t(5),std::runtime_error);
EXPECT_THROW(copy.x(std::numeric_limits<std::size_t>::max()),std::runtime_error);
EXPECT_THROW(copy.t(std::numeric_limits<std::size_t>::max()),std::runtime_error);
 PDE::Domain1D rounded(.1,.9,.3,.8);PDE::Grid1D r(rounded,8,7);EXPECT_DOUBLE_EQ(r.x(0),rounded.xL);EXPECT_DOUBLE_EQ(r.x(7),rounded.xR);EXPECT_DOUBLE_EQ(r.t(0),rounded.t0);EXPECT_DOUBLE_EQ(r.t(7),rounded.t1);
}
TEST(C2C_Helpers,InvalidDomainsAndGrids){
 double nan=std::numeric_limits<double>::quiet_NaN(),inf=std::numeric_limits<double>::infinity(),M=std::numeric_limits<double>::max(),tiny=std::numeric_limits<double>::denorm_min();
 for(auto vals:std::vector<std::array<double,4>>{{1,0,0,1},{0,0,0,1},{0,1,1,0},{0,1,0,0},{-M,M,0,1},{0,1,-M,M}})
EXPECT_THROW(PDE::Domain1D(vals[0],vals[1],vals[2],vals[3]),std::runtime_error);
 for(double bad:{nan,inf,-inf})for(int role=0;role<4;++role){std::array<double,4> v{0,1,0,1};v[role]=bad;
EXPECT_THROW(PDE::Domain1D(v[0],v[1],v[2],v[3]),std::runtime_error);}
 PDE::Domain1D d(0,1,0,1);for(std::size_t nx:{0u,1u,2u})
EXPECT_THROW(PDE::Grid1D(d,nx,1),std::runtime_error);
EXPECT_THROW(PDE::Grid1D(d,3,0),std::runtime_error);if(std::vector<double>().max_size()<std::numeric_limits<std::size_t>::max())
EXPECT_THROW(PDE::Grid1D(d,std::vector<double>().max_size()+1,1),std::runtime_error);
 PDE::Domain1D s(0,tiny,0,1),t(0,1,0,tiny);
EXPECT_THROW(PDE::Grid1D(s,3,1),std::runtime_error);
EXPECT_THROW(PDE::Grid1D(t,3,2),std::runtime_error);
 for(int role=0;role<4;++role){auto changed=d;// Mutate each public field separately.
 if(role==0)changed.xL=2;if(role==1)changed.xR=-1;if(role==2)changed.t0=2;if(role==3)changed.t1=-1;
EXPECT_THROW(PDE::Grid1D(changed,3,1),std::runtime_error);}
 auto changed=d;changed.xL=-M;changed.xR=M;
EXPECT_THROW(PDE::Grid1D(changed,3,1),std::runtime_error);
}
TEST(C2C_Helpers,DefaultsReferencesFactoryAndTheta){
 PDE::Domain1D d(0,1,0,.1);PDE::Grid1D g(d,5,2);PDE::PDE1D p;EXPECT_TRUE(bool(p.diffusion));EXPECT_TRUE(bool(p.advection));EXPECT_TRUE(bool(p.reaction));EXPECT_TRUE(bool(p.source));EXPECT_FALSE(bool(p.initial));EXPECT_FALSE(bool(p.boundaryL));EXPECT_FALSE(bool(p.boundaryR));EXPECT_DOUBLE_EQ(p.diffusion(3,4),1);EXPECT_DOUBLE_EQ(p.advection(3,4),0);EXPECT_DOUBLE_EQ(p.reaction(3,4),0);EXPECT_DOUBLE_EQ(p.source(3,4),0);EXPECT_EQ(p.boundaryConfig.types[0],PDE::BoundaryType::Dirichlet);EXPECT_EQ(p.boundaryConfig.types[1],PDE::BoundaryType::Dirichlet);
 auto s=PDE::makeSolver1D(g,p);EXPECT_DOUBLE_EQ(s.theta(),.5);for(double good:{.5,.75,1.,std::nextafter(.5,1.)}){s.setTheta(good);EXPECT_DOUBLE_EQ(s.theta(),good);}for(double bad:{.49,1.01,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()}){double old=s.theta();
EXPECT_THROW(s.setTheta(bad),std::runtime_error);EXPECT_DOUBLE_EQ(s.theta(),old);}s.setTheta(.5);
 p.initial=p.boundaryL=p.boundaryR=c2c::constant(1);auto a=s.solve(),again=s.solve();EXPECT_EQ(c2c::vector(a),c2c::vector(again));for(double v:a)EXPECT_NEAR(v,1,1e-12);auto original=c2c::vector(a);p.initial=p.boundaryL=p.boundaryR=c2c::constant(2);auto copy=s;auto moved=std::move(copy);EXPECT_DOUBLE_EQ(moved.theta(),.5);for(double v:moved.solve())EXPECT_NEAR(v,2,1e-12);EXPECT_EQ(c2c::vector(a),original);
 // Assignment of the referenced grid is observable through the same scheme.
 PDE::Grid1D replacement(d,7,3);g=replacement;EXPECT_EQ(s.solve().size(),7u);
}
}
