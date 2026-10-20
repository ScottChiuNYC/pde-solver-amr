#include <pde_solver_amr_core/PDE.h>
#include <gtest/gtest.h>
#include <limits>
TEST(Implementation,ArbitraryDepthSiblingIdentity) {
    PDE::detail::Leaf left{9,std::vector<unsigned char>(200,0),false};
    auto right=left;right.path.back()=1;
    EXPECT_TRUE(PDE::detail::siblings(left,right));
    right.path[80]=1;EXPECT_FALSE(PDE::detail::siblings(left,right));
    right=left;right.path.back()=1;right.root=10;
    EXPECT_FALSE(PDE::detail::siblings(left,right));
}
TEST(Implementation,IndicatorStrictZeroAndSubnormalRetention) {
    const double tiny=std::numeric_limits<double>::denorm_min();
    std::vector<double> x{0,tiny,2*tiny};PDE::Array1D u(3,0);
    auto indicators=PDE::detail::indicators(x,u,0,"pre-refinement");
    ASSERT_EQ(indicators.size(),2u);EXPECT_EQ(indicators[0],0);EXPECT_EQ(indicators[1],0);
}
TEST(Implementation,ThomasLaterOverflowKeepsAllRHSValues) {
    PDE::Array1D l(1,0),d(2,1),u(1,0),rhs(2,0);
    d[1]=std::numeric_limits<double>::denorm_min();rhs[0]=3;
    rhs[1]=std::numeric_limits<double>::max();
    EXPECT_THROW(PDE::TridiagonalSolver::solve(l,d,u,rhs),std::runtime_error);
    EXPECT_EQ(rhs[0],3);EXPECT_EQ(rhs[1],std::numeric_limits<double>::max());
}
