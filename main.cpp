#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "parser.h"


int main()
{
    std::cout << "normal line\n";
    {
        std::string s = "-v=1 -a --g=12 --gp=12.2\n -e=-34.08  \n\t --vc=10.0,-23,22,10 --str=stringg\n--string-large=\"large string\"";
        parser prs(s);
        prs.log();

        std::vector<double> v;
        double dv = 0;
        double ndv = 0;
        int iv = 0;
        std::string str;
        std::string str_large;

        prs.bind_array(&v, "vc");
        prs.bind(&dv, "gp");
        prs.bind(&ndv, "e");
        prs.bind(&iv, "g");
        prs.bind(&str, "str");
        prs.bind(&str_large, "string-large");

    }


    std::cout << '\n';
    std::cout << "broken line\n";
    {
        std::string s = "-v=1 -g= -- - g";
        parser prs(s);

        // parser::parsing(s, " \t\n");
        prs.log();
    }
    
    std::cout << '\n';
    std::cout << "broken line2\n";
    {
        std::string bad_s = "---v=1 - =2 --gp=12.2.5 --str=\"no end --vc=10,,20 -- -key=val";
        parser prs(bad_s);

        // parser::parsing(s, " \t\n");
        prs.log();
    }


    return 0;
}