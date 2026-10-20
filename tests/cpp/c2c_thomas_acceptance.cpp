#include "c2c_acceptance_support.h"
namespace {
void success(std::vector<double> lv,std::vector<double> dv,std::vector<double> uv,std::vector<double> rv,std::vector<double> expected){auto l=c2c::array(lv),d=c2c::array(dv),u=c2c::array(uv),r=c2c::array(rv);PDE::TridiagonalSolver::solve(l,d,u,r);ASSERT_EQ(r.size(),expected.size());for(std::size_t i=0;i<r.size();++i)EXPECT_NEAR(r[i],expected[i],1e-12);EXPECT_EQ(c2c::vector(l),lv);EXPECT_EQ(c2c::vector(d),dv);EXPECT_EQ(c2c::vector(u),uv);}
bool same(std::vector<double> const& a,std::vector<double> const& b){if(a.size()!=b.size())return false;for(std::size_t i=0;i<a.size();++i)if(!(a[i]==b[i]||(std::isnan(a[i])&&std::isnan(b[i]))))return false;return true;}
void failure(std::vector<double> lv,std::vector<double> dv,std::vector<double> uv,std::vector<double> rv){auto l=c2c::array(lv),d=c2c::array(dv),u=c2c::array(uv),r=c2c::array(rv);
EXPECT_THROW(PDE::TridiagonalSolver::solve(l,d,u,r),std::runtime_error);EXPECT_TRUE(same(c2c::vector(l),lv));EXPECT_TRUE(same(c2c::vector(d),dv));EXPECT_TRUE(same(c2c::vector(u),uv));EXPECT_TRUE(same(c2c::vector(r),rv));}
TEST(C2C_Thomas,SuccessAndUniformScaling){for(int power:{0,-500,500}){double scale=std::ldexp(1.,power);success({},{2*scale},{},{6*scale},{3});success({scale,scale},{4*scale,4*scale,4*scale},{scale,scale},{6*scale,12*scale,14*scale},{1,2,3});}auto both=c2c::array({1,1}),d=c2c::array({4,4,4}),r=c2c::array({6,12,14});PDE::TridiagonalSolver::solve(both,d,both,r);EXPECT_EQ(c2c::vector(both),(std::vector<double>{1,1}));for(int i=0;i<3;++i)EXPECT_NEAR(r[i],i+1,1e-12);}
TEST(C2C_Thomas,ShapeAliasAndNonfiniteFailures){failure({},{},{},{});failure({1},{2,2},{1},{1});failure({1},{2,2},{1},{1,1,1});failure({},{2,2},{1},{1,1});failure({1},{2,2},{},{1,1});failure({1,1},{2,2},{1},{1,1});failure({1},{2,2},{1,1},{1,1});for(int role=0;role<4;++role)for(double bad:{std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()}){std::vector<double> l{1},d{4,4},u{1},r{1,1};if(role==0)l[0]=bad;if(role==1)d[0]=bad;if(role==2)u[0]=bad;if(role==3)r[0]=bad;failure(l,d,u,r);}
 // Each coefficient role aliases RHS; valid dimensions cannot all coexist for off-diagonal aliases,
 // but the required rejection must preserve every object regardless of overlapping failure causes.
 for(int role=0;role<3;++role){auto l=c2c::array({1}),d=c2c::array({4,4}),u=c2c::array({1});auto lb=c2c::vector(l),db=c2c::vector(d),ub=c2c::vector(u);auto& r=role==0?l:role==1?d:u;
EXPECT_THROW(PDE::TridiagonalSolver::solve(l,d,u,r),std::runtime_error);EXPECT_EQ(c2c::vector(l),lb);EXPECT_EQ(c2c::vector(d),db);EXPECT_EQ(c2c::vector(u),ub);}
}
TEST(C2C_Thomas,NoPivotingAndGeneratedFailures){failure({},{0},{},{1});failure({0},{1,0},{0},{1,1});failure({1},{0,1},{1},{1,1});failure({1},{1,1},{1},{2,2});failure({},{std::numeric_limits<double>::denorm_min()},{},{std::numeric_limits<double>::max()});
 // Finite recurrence product overflows before a subtraction could cancel it.
 double M=std::numeric_limits<double>::max();failure({M},{1,M},{2},{1,M});
}
TEST(C2C_Thomas,OriginalRowPivotThresholdAndEquality){double tau=64*std::numeric_limits<double>::epsilon();for(int power:{0,-500,500}){double s=std::ldexp(1.,power);for(int exponent:{-48,-46,-44}){double delta=std::ldexp(1.,exponent);if(exponent==-44)success({s},{s,(1+delta)*s},{s},{2*s,(2+delta)*s},{1,1});else failure({s},{s,(1+delta)*s},{s},{2*s,(2+delta)*s});}failure({s},{s,s},{(1-tau)*s},{(2-tau)*s,2*s});}}
}
