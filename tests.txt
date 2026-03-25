#include <string>
#include <vector>
#include <map>
#include <iostream>


struct ind_t
{
    std::string::const_iterator start;
    std::string::const_iterator end;
};

struct opt_ind_t
{
    ind_t cls;
    ind_t args;
};


bool is_separator(char c, const char* seps)
{
    char s = *seps;
    while (s != '\0')
    {
        if (c == s)
        {
            return true;
        }
        
        s = *(++seps);
    }

    return false;
    
}

void split_opts(const std::string& s, const char* separators)
{
    
    auto start = s.begin();
    auto end = start;
    auto iter = start;

    ind_t ind = {start, end};

    size_t dq = 0;

    while (iter != s.end())
    {
        if (is_separator(*iter, separators) && dq % 2 == 0)
        {
            end = iter;
            ind = {start, end};
     
            // avoid array of separators
            if (start != iter)
            {
                auto st = s.substr(start - s.begin(), iter - start);
                std::cout << "split_opts: "<< st << '\n';
            }

            ++iter;
            start = iter;
            continue;
        }
        else if (*iter == '"')
        {
            ++dq;
        }   
        ++iter;
    } 

    // avoid array of separators
    if (start != iter)
    {
        auto st = s.substr(start - s.begin(), iter - start);
        std::cout << "split_opts: "<< st << '\n';
    }
}

opt_ind_t opt_split(const std::string& s, char separator, const std::string::const_iterator begin, const std::string::const_iterator end)
{
    opt_ind_t sind = {};

    auto iter = begin;
    
    while (iter != end)
    {
        if (*iter == separator)
        {
            // class
            auto st = s.substr(begin - s.begin(), iter - begin);
            std::cout << "opt_split: '" << s << "' class: " << st << '\n';
            break;
        }
        ++iter;
    }
    if (iter == end)
    {
        auto st = s.substr(begin - s.begin(), iter - begin);
        std::cout << "opt_split: '" << s << "' class: " << st << '\n';
        return opt_ind_t();
    }
    
    // args
    if (++iter != end)
    {
        auto st = s.substr(iter - s.begin(), end - iter);
        std::cout << "opt_split: '" << s << "' args: " << st << '\n';
    }
    
    return sind;
}

bool is_class(std::string::const_iterator& begin, const std::string::const_iterator end)
{

    if (*begin == '-')
    {
        if (++begin != end)
        {
            if (*begin == '-')
            {
                if (++begin != end)
                {
                    return true;
                }
                return false;
            }
            return true;
        }
    }
    return false;    
}


void test_is_class()
{
    {
        std::string cl = "-vae";
        auto it = cl.cbegin();
        std::cout << "is_class: '" << cl << "' res: " << std::boolalpha << is_class(it, cl.end()) << '\n';
    }

    {
        std::string cl = "--vae";
        auto it = cl.cbegin();
        std::cout << "is_class: '" << cl << "' res: " << std::boolalpha << is_class(it, cl.end()) << '\n';
    }
    {
        std::string cl = "-v";
        auto it = cl.cbegin();
        std::cout << "is_class: '" << cl << "' res: " << std::boolalpha << is_class(it, cl.end()) << '\n';
    }

    {
        std::string cl = "vae";
        auto it = cl.cbegin();
        std::cout << "is_class: '" << cl << "' res: " << std::boolalpha << is_class(it, cl.end()) << '\n';
    }
    
    {
        std::string cl = "v";
        auto it = cl.cbegin();
        std::cout << "is_class: '" << cl << "' res: " << std::boolalpha << is_class(it, cl.end()) << '\n';
    }
    {
        std::string cl = "-";
        auto it = cl.cbegin();
        std::cout << "is_class: '" << cl << "' res: " << std::boolalpha << is_class(it, cl.end()) << '\n';
    }
    {
        std::string cl = "--";
        auto it = cl.cbegin();
        std::cout << "is_class: '" << cl << "' res: " << std::boolalpha << is_class(it, cl.end()) << '\n';
    }
}

void test_opt_split(){
    {
        std::string so = "--v=123,12";
        opt_split(so, '=', so.begin(), so.end());
    }
    {
        std::string so = "--v=\"string string\"";
        opt_split(so, '=', so.begin(), so.end());
    }
    {
        std::string so = "-a";
        opt_split(so, '=', so.begin(), so.end());
    }
    
}

void test_split_opts(){
    {
        std::string s = "-v=1 -a --g=12 --gp=12.2  \n\t --vc=10.0,23,22,10 --str=stringg\n--string-large=\"large string\"";
        std::cout << "tested str: '" << s << "' separators: {" << "' ', '\\n', '\\t'" << "}\n";
        split_opts(s, " \n\t");
    }
    {
        std::string s = "-v";
        std::cout << "tested str: '" << s << "' separators: {" << "' ', '\\n', '\\t'" << "}\n";
        split_opts(s, " \n\t");
    }
}

int main()
{
    test_split_opts();
    std::cout << '\n';

    test_opt_split();
    std::cout << '\n';
    
    test_is_class();
    std::cout << '\n';


    {
        std::string cl = "v";
        auto it = cl.cbegin();
        auto res = is_class(it, cl.end());
        std::cout << "is_class: '" << cl << "' res: " << std::boolalpha << res << '\n';
        std::cout <<"class name: '" << cl << "' res: ";
        
        if (res)
        {
            auto cls = cl.substr(it - cl.begin(), cl.end() - it);
            std::cout << cls << '\n';
        }
        else std::cout << "bad class\n";
    }


    return 0;
}