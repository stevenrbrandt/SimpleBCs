#include <cctk.h>
#include <cctk_Arguments.h>
#include <cctk_Parameters.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

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
    std::string thorn, name, full_name;
    GF(const std::string& t,const std::string n) : thorn(t), name(n), full_name(t+"::"+n) {}
    ~GF() {}
};

inline std::ostream& operator<<(std::ostream& o, const GF& gf) {
    return o << gf.full_name;
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
 * Determine whether character c is a c-identifier character.
 */
inline bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c=='_';
}

/*
 * Break a string into tokens that are either (1) a c-identifier string,
 * a single colon (:), or a double colon. Commas and whitespace are ignored.
 */
std::vector<std::string> parse_bc_string(std::string s) {
    char last_c=0;
    std::string buf;
    std::vector<std::string> tokens;
    for(size_t i=0;i<s.size();i++) {
        char c = s[i];
        if(c == ':' && last_c == ':') {
            buf.push_back(c);
        } else if(is_alpha(c) && is_alpha(last_c)) {
            buf.push_back(c);
        } else if(c == ':' || is_alpha(c)) {
            if(buf.size() > 0) tokens.push_back(buf);
            buf.clear();
            buf.push_back(c);
        } else if(c == ',' || c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            ; // ignore
        } else {
            CCTKERRORstream sout;
            sout << "Illegal character in input: '" << c << "'" << std::endl;
        }
        last_c = c;
    }
    if(buf.size() > 0) tokens.push_back(buf);
    return tokens;
}

/*
 * Convert the list of tokens to a vector BC data structures.
 */
std::vector<BC> create_bcs(std::vector<std::string>&& vs,bool verbose) {
    size_t i=0;
    std::vector<BC> vb;
    vb.push_back(BC("none"));
    while(i < vs.size()) {
        if(is_alpha(vs.at(i).at(0)) && vs.at(i+1) == ":") {
            BC bc(vs.at(i));
            vb.push_back(bc);
            i += 2;
        } else if(is_alpha(vs.at(i).at(0)) && vs.at(i+1) == "::" && is_alpha(vs.at(i+2).at(0))) {
            GF gf(vs.at(i), vs.at(i+2));
            vb.at(vb.size()-1).gfs.push_back(gf);
            i += 3;
        } else {
            i += 1;
        }
    }
    if(verbose) {
        CCTKINFOstream sout;
        sout << "BCs to be applied by SimpleBCs:" << std::endl;
        for(size_t i=0;i<vb.size();i++)
            sout << "vb[" << i << "]=" << vb[i] << std::endl;
    }
    return vb;
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
        bcs = create_bcs(parse_bc_string(bc_string), verbose);
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
