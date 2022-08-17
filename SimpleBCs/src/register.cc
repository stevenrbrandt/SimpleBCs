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
 * This class represents a grid function. Both the full name "MyThorn::a" and
 * the group id are stored.
 */
struct GF {
    std::string name;
    int groupId;
    GF(const std::string& n) : name(n), groupId(CCTK_GroupIndex(n.c_str())) {}
    ~GF() {}
};

inline std::ostream& operator<<(std::ostream& o, const GF& gf) {
    return o << gf.name << "(groupId=" << gf.groupId << ")";
}

/*
 * This class represents a Boundary Condition. It contains the name of the boundary
 * condition and a vector of the grid functions it applies to.
 */
struct BC {
    std::string name;
    std::vector<GF> gfs;
    BC(const std::string& n) : name(n) {
        if(n == "") name = "?";
    }
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
            std::string bcn = bc_name[i];
            BC bc(bcn);
            std::istringstream gstr(bc_groups[i]);
            std::string gn;
            while(gstr >> gn) {
                GF gf(gn);
                if(gf.groupId < 0) {
                    CCTKERRORstream() << "Invalid group name: " << gn;
                }
                bc.gfs.push_back(gf);
            }
            if(bcn.size() > 0 && bc.gfs.size() > 0) {
                bcs.push_back(bc);
                if(verbose) {
                    CCTKINFOstream() << "Adding " << bc;
                }
            } else if(bc.gfs.size() > 0) {
                CCTKERRORstream() << "Boundary condition with no name at index i=" << i <<": " << bc;
            }
        }
        init = false;
    }
    int ierr = 0;
    for(auto& bc : bcs) {
        const char *bc_name = bc.name.c_str();
        for(auto& gf : bc.gfs) {
            const char *gf_name = gf.name.c_str();
            if(is_local_mode()) {
                ierr = Boundary_SelectGroupForBC(cctkGH, CCTK_ALL_FACES, 1, -1, gf_name, bc_name);
                if(verbose)
                    CCTKINFOstream() << "select for bc: " << bc_name << " -> " << gf_name;
                if(ierr < 0)
                    CCTKERRORstream() << "select for bc: " << bc_name << " -> " << gf_name << " failed!";
            }
            if(is_level_mode()) {
                ierr = CCTK_SyncGroupI(cctkGH,gf.groupId);
                if(ierr < 0) CCTKERRORstream() << "Sync for group " << gf << " failed!";
            }
        }
    }
}
