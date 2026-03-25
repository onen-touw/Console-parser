#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "parser.h"

#define NTOS(s) #s

int main()
{
    std::cout << "normal line\n";
    {
        std::string s = "blank -v=1 -a --g=12 --gp=12.2\n -e=-34.08  \n\t --vc=10.25,-23,22,10 --str=stringg\n--string-large=\"large string\"";
        parser prs(s, true, true);
        prs.log(std::cout);

        auto pname = prs.get_prog_name();
        std::cout << "prog name: '" << pname << "'\n";

        std::vector<double> v;
        double dv = 0;
        double ndv = 0;
        int iv = 0;
        std::string str;
        std::string str_large;

        prs.bind(&v, "vc");
        prs.bind(&dv, "gp");
        prs.bind(&ndv, "e");
        prs.bind(&iv, "g");
        prs.bind(&str, "str");
        prs.bind(&str_large, "string-large");

        std::cout << "var value: " << NTOS(dv) " = " << dv << '\n';
        std::cout << "var value: " << NTOS(ndv) " = " << ndv << '\n';
        std::cout << "var value: " << NTOS(iv) " = " << iv << '\n';
        std::cout << "var value: " << NTOS(str) " = " << str << '\n';
        std::cout << "var value: " << NTOS(str_large) " = " << str_large << '\n';
        
        std::cout << "var value: " << NTOS(v) ":\n";
        for (const auto &i : v)
        {
            std::cout << "\titem value: " << i << '\n';
        }
    }


    std::cout << '\n';
    std::cout << "broken line\n";
    {
        std::string s = "-v=1 -g= -- - g";
        parser prs(s, false, true);
        prs.log(std::cout);
    }
    
    std::cout << '\n';
    std::cout << "broken line2\n";
    {
        std::string bad_s = "---v=1 - =2 --gp=12.2.5 --str=\"no end --vc=10,,20 -- -key=val";
        parser prs(bad_s, false, true);
        prs.log(std::cout);
    }
    return 0;
}