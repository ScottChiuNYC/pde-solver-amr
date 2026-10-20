#!/usr/bin/env python3
"""Pre-code syntax and reference-oracle validation, without implementing PDE API.

A temporary include tree is populated solely with the frozen public declarations.
API-neutral grid/data views execute the independent oracle; no production API
function is defined or called by this probe. This is NOT a domain implementation
pass. Full acceptance requires the real public headers and canonical builds.
"""
import argparse
from pathlib import Path
import subprocess
import tempfile

PROBE = '#include "c2c_amr_oracle.h"\n// API-neutral data views: not definitions of, or implementations behind, the public API.\nstruct Data {std::function<double(double,double)> diffusion,advection,reaction,source,initial,boundaryL,boundaryR;};\nstruct Grid {double T;std::size_t nxv,ntv;std::size_t nx()const{return nxv;}std::size_t nt()const{return ntv;}double x(std::size_t i)const{return i==nxv-1?1.:double(i)/double(nxv-1);}double t(std::size_t k)const{return k==ntv?T:double(k)*dt();}double dt()const{return T/double(ntv);}};\nint main(){bool split=false,merge=false,later=false;for(int which:{0,1,2,3}){Data p;p.diffusion=[](double t,double x){return 1+.2*t+.1*x;};p.advection=[](double t,double x){return .3+.1*t-.2*x;};p.reaction=[](double t,double x){return -.4+.2*t+.1*x;};p.source=[which](double t,double x){double a=1+.2*t+.1*x,b=.3+.1*t-.2*x,c=-.4+.2*t+.1*x,P=which==0?1+x+std::sin(c2c::pi()*x):1+x+x*x;return std::exp(t)*((1-c)*P+(which==0?a*c2c::pi()*c2c::pi()*std::sin(c2c::pi()*x):-2*a)-b*(which==0?1+c2c::pi()*std::cos(c2c::pi()*x):1+2*x));};p.initial=p.boundaryL=p.boundaryR=[which](double t,double x){return std::exp(t)*(which==0?1+x+std::sin(c2c::pi()*x):1+x+x*x);};double T=which==0?.1:.2;PDE::AMROptions o{2,65,which==0?.06:.02,which==0?.01:.005};Grid g{T,5,4};if(which>=2){p.diffusion=c2c::constant(1);p.advection=p.reaction=c2c::constant(0);if(which==2){p.initial=p.boundaryL=p.boundaryR=[](double t,double x){return std::exp(t)*x*(1-x);};p.source=[](double t,double x){return std::exp(t)*(x*(1-x)+2);};}else{p.initial=[](double,double x){return std::sin(c2c::pi()*x);};p.boundaryL=p.boundaryR=p.source=c2c::constant(0);o={1,65,.1,.09};g={.5,5,10};}}for(double theta:{.5,.75,1.}){auto r=c2c::reference(g,p,theta,o);std::cout<<"fixture "<<which<<" theta "<<theta<<" events "<<r.events.size()<<"\\n";for(auto e:r.events){if(e.action==PDE::AMRAction::refine){split=true;later|=e.step>0;}else merge=true;}}}std::cout<<"coverage split="<<split<<" merge="<<merge<<" later_split="<<later<<"\\n";return split&&merge&&later?0:1;}\n'

def run(command):
    print("COMMAND:", " ".join(map(str, command)), flush=True)
    subprocess.run(list(map(str, command)), check=True)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--gtest-include", required=True, type=Path)
    args = parser.parse_args()
    tests = Path(__file__).resolve().parent
    root = tests.parents[1]
    essence = (root / "docs/development/essence.md").read_text()
    start = essence.index("#include <array>")
    declarations = essence[start:essence.index("```", start)]
    run([args.compiler, "--version"])
    with tempfile.TemporaryDirectory(prefix="c2c-precode-") as dirname:
        temp = Path(dirname)
        include = temp / "pde_solver_amr_core/PDE.h"
        include.parent.mkdir()
        include.write_text("#pragma once\n" + declarations)
        sources = sorted(tests.glob("c2c_*acceptance.cpp")) + [tests / "c2c_multitu_peer.cpp"]
        flags = ["-std=c++17", "-ffp-contract=off", "-I", temp, "-I", tests, "-I", args.gtest_include]
        run([args.compiler, *flags, "-fsyntax-only", *sources])
        probe = temp / "oracle_probe.cpp"
        probe.write_text(PROBE)
        executable = temp / "oracle-probe"
        run([args.compiler, *flags, "-O2", probe, "-o", executable])
        run([executable])
    print("PASS: declarations-only syntax and API-neutral oracle coverage. Production API untested.", flush=True)

if __name__ == "__main__":
    main()
