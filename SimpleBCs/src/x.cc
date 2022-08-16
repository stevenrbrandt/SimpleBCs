#include <vector>
#include <string>
#include <iostream>
#include <assert.h>

struct GF {
    std::string thorn, name;
    GF(const std::string& t,const std::string n) : thorn(t), name(n) {}
    ~GF() {}
};

inline std::ostream& operator<<(std::ostream& o, const GF& gf) {
    return o << gf.thorn << "::" << gf.name;
}

struct BC {
    std::string name;
    std::vector<GF> gfs;
    BC(const std::string& n) : name(n) {}
    ~BC() {}
};

inline std::ostream& operator<<(std::ostream& o,const BC& bc) {
    o << "BC(name=" << bc.name;
    for(int i=0;i<bc.gfs.size();i++)
        o << "," << bc.gfs[i];
    return o << ")";
}

inline bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c=='_';
}

std::vector<std::string> parse_bc_string(std::string s) {
    char last_c=0;
    std::string buf;
    std::vector<std::string> tokens;
    for(int i=0;i<s.size();i++) {
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
            std::cerr << "Illegal character in input: '" << c << "'" << std::endl;
            abort();
        }
        last_c = c;
    }
    if(buf.size() > 0) tokens.push_back(buf);
    return tokens;
}

std::vector<BC> create_bcs(std::vector<std::string>&& vs) {
    int i=0;
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
    std::cout << "BCs:" << std::endl;
    for(int i=0;i<vb.size();i++)
        std::cout << "vb[" << i << "]=" << vb[i] << std::endl;
    return vb;
}

int main() {
    create_bcs(parse_bc_string("none: , a::b"));
    create_bcs(parse_bc_string("flat: aa::bb c::dd"));
}
