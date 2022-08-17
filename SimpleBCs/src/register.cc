#include <cctk.h>
#include <cctk_Arguments.h>
#include <cctk_Parameters.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

const size_t max_num_bc = 10;

/*
 * Provide an iostream for generating a CCTK_INFO() message.
 * Output is generated when the destructor is called.
 */
struct CCTKINFOstream : public std::ostringstream {
    CCTKINFOstream() {}
    ~CCTKINFOstream() {
        std::string s = str();
        CCTK_INFO(s.c_str());
    }
};

/*
 * Provide an iostream for generating a CCTK_ERROR() call.
 * Output is generated when the destructor is called.
 * The code will also exit when the destructor is called.
 */
struct CCTKERRORstream : public std::ostringstream {
    CCTKERRORstream() {}
    ~CCTKERRORstream() {
        std::string s = str();
        CCTK_ERROR(s.c_str());
    }
};

/*
 * This class represents a grid function. If the gridfunction is named "MyThorn::a",
 * then the full_name is "MyThorn::a", the name is "a", and the thorn name is "MyThorn".
 */
struct GF {
    std::string name;
    int groupId;
    GF(const std::string& n) : name(n), groupId(CCTK_GroupIndex(n.c_str()) {}
    ~GF() {}
};

inline std::ostream& operator<<(std::ostream& o, const GF& gf) {
    return o << gf.name << "(" << gf.groupId << ")";
}

/*
 * This class represents a Boundary Condition. It contains the name of the boundary
 * condition and a vector of the grid functions it applies to.
 */
struct BC {
    std::string name;
    std::vector<GF> gfs;
    BC(const std::string& n) : name(n) {}
    ~BC() {}
};

inline std::ostream& operator<<(std::ostream& o,const BC& bc) {
    o << "BC(name=" << bc.name;
    for(size_t i=0;i<bc.gfs.size();i++)
        o << "," << bc.gfs[i];
    return o << ")";
}

/*
 * Register all the boundary conditions described by the bc_string.
 */
void RegisterSimpleBCs(CCTK_ARGUMENTS) {
    DECLARE_CCTK_ARGUMENTS_RegisterSimpleBCs;
    DECLARE_CCTK_PARAMETERS;

    static bool init = true;
    static std::vector<BC> bcs;
    if(init) {
        for(size_t i=0;i<max_num_bc;i++) {
            std::string bc = bc_name[i];
            if(bc.size() > 0) {
                BC bc(bc);
                std::istringstream gstr(bc_groups[i]);
                std::strin gn;
                while(gstr >> gn) {
                    GF gf(gn);
                    if(gf.groupId < 0) {
                        CCTKERRORstream() << "Invalid group name: " << gn;
                    }
                    bc.gfs.push_back(gf);
                }
                if(bc.gfs.size() > 0) {
                    bcs.push_back(bc);
                    CCTKINFOstream() << "Adding " << bc_name[i];
                }
            }
        }
        init = false;
    }
    int ierr = 0;
    for(auto& bc : bcs) {
        const char *bc_name = bc.name.c_str();
        for(auto& gf : bc.gfs) {
            const char *gf_name = gf.full_name.c_str();
            ierr = Boundary_SelectGroupForBC(cctkGH, CCTK_ALL_FACES, 1, -1, gf_name, bc_name);
            if(verbose) {
                CCTKINFOstream sout;
                sout << "select for bc: " << bc_name << " -> " << gf_name << std::endl;
            }
        }
    }
}
