#include <pde_solver_amr_core/PDE.h>
#include <gtest/gtest.h>
#include <limits>
#include <string>
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
TEST(Implementation,ValidationFailureNamesRoleAndBoundarySide) {
    PDE::Domain1D domain(0,1,0,.1);PDE::Grid1D grid(domain,3,1);
    using Field=PDE::Func::F1D PDE::PDE1D::*;
    const std::pair<Field,char const*> fields[]={
        {&PDE::PDE1D::diffusion,"diffusion"},{&PDE::PDE1D::advection,"advection"},
        {&PDE::PDE1D::reaction,"reaction"},{&PDE::PDE1D::source,"source"},
        {&PDE::PDE1D::initial,"initial"},{&PDE::PDE1D::boundaryL,"boundaryL"},
        {&PDE::PDE1D::boundaryR,"boundaryR"}};
    for(auto mode:{PDE::MeshMode::Uniform,PDE::MeshMode::Adaptive}) {
        PDE::SolveOptions1D options;options.mesh_mode=mode;
        options.amr=PDE::AMROptions{0,3,1,0};
        for(auto const& field:fields) {
            int calls=0;PDE::PDE1D p;
            for(auto const& configured:fields)p.*configured.first=[&](double,double){++calls;return 1.;};
            p.*field.first={};auto solver=PDE::makeSolver1D(grid,p);
            try{(void)solver.solve(options);FAIL()<<"missing callback accepted";}
            catch(std::runtime_error const& e){EXPECT_NE(std::string(e.what()).find(field.second),std::string::npos)<<e.what();}
            EXPECT_EQ(calls,0);
        }
        for(std::size_t side:{PDE::BoundaryConfig1D::Left,PDE::BoundaryConfig1D::Right}) {
            int calls=0;PDE::PDE1D p;
            for(auto const& configured:fields)p.*configured.first=[&](double,double){++calls;return 1.;};
            p.boundaryConfig.types[side]=PDE::BoundaryType::Neumann;
            auto solver=PDE::makeSolver1D(grid,p);
            try{(void)solver.solve(options);FAIL()<<"unsupported boundary accepted";}
            catch(std::runtime_error const& e){EXPECT_NE(std::string(e.what()).find(side==0?"left":"right"),std::string::npos)<<e.what();}
            EXPECT_EQ(calls,0);
        }
    }
}
