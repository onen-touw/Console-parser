#pragma once

#include <string>
#include <vector>
#include <map>
#include <type_traits>
#include <iostream>

class parser
{
private:
    using iterator_t = std::string::const_iterator;

public:

    struct ind_t
    {
        iterator_t start;
        iterator_t end;

        size_t get_dist() const {return end - start;}
    };

    struct opt_ind_t
    {
        ind_t cls;
        ind_t args;
    };
private:
    using container_t = std::map<std::string, ind_t>;

public:
    static constexpr const char* default_line_separators = " \n\t";
    static constexpr char default_arg_separator = '=';

private:
    container_t _options;
    std::string _line;
    bool _is_good = true;
public:

    parser() {}
    
    parser(const std::string& s, const char* separators = default_line_separators) 
        : _line(s) 
    {
        parsing(separators);
    }

    parser(std::string&& s, const char* separators) 
        : _line(std::move(s)) 
    {
        parsing(separators);
    }

    ~parser() {}


    void log() const 
    {
        std::cout << "log:\n";
        for (const auto &i : _options)
        {
            std::cout << "\t" 
            << i.first << " {" << i.second.start - _line.begin() << "; " << i.second.end - _line.begin() << "} (" << i.second.get_dist() << ")\n";
            std::cout << "\t\targs: " << _line.substr(i.second.start - _line.begin(), i.second.get_dist()) << '\n';
        }
        std::cout << '\n';
    }

    template <typename Ty>
    bool bind(Ty* var, const std::string& token) const
    {
        std::vector<std::string> tks = token_split(token, ',');

        for (auto& t : tks)
        {
            auto it = _options.find(t);
            if (it != _options.end())
            {
                if (var)
                {
                    auto st = _line.substr(it->second.start - _line.begin(), it->second.get_dist());
                    std::cout << "bind arg: "<< st << '\n';
                    *var = get_arg<Ty>(_line, it->second);

                    std::cout << "\tval: " << *var << '\n';
                }
                return true;
            }
        }
        return false;
    }


    // template <typename Ty>
    bool bind_array(std::vector<double>* arr, const std::string& token) const
    {
        std::vector<std::string> tks = token_split(token, ',');

        for (auto& t : tks)
        {
            auto it = _options.find(t);
            if (it != _options.end())
            {
                if (arr)
                {
                    auto ind = it->second;      // indeces of start and end of array
                    
                    auto iter = ind.start;
                    auto start = ind.start;
                    size_t dq = 0;

                    while (iter!= ind.end)
                    {
                        if (*iter == ',' && dq % 2 == 0)
                        {
                            auto st = _line.substr(start - _line.begin(), iter - start);
                            std::cout << "array item: "<< st << '\n';

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

                    auto st = _line.substr(start - _line.begin(), iter - start);
                    std::cout << "array item: "<< st << '\n';

                    // arr->reserve(sz);
                    // for (size_t i = 0; i < sz; i++)
                    // {
                    //     arr->push_back(it->second.get_arg<Ty>(i));
                    // }
                }
                return true;
            }
        }
        return false;
    }



private:
    void parsing(const char *separators)
    {

        auto start = _line.begin();
        auto iter = start;

        ind_t ind = {start, start};

        size_t dq = 0;

        while (iter != _line.end())
        {
            if (is_separator(*iter, separators) && dq % 2 == 0)
            {
                ind = {start, iter};

                // avoid array of separators
                if (start != iter)
                {
                    auto st = _line.substr(start - _line.begin(), iter - start);
                    std::cout << "split_opts: "<< st << '\n';

                    auto isres = is_class(start, iter);
                    std::cout << "\tis_class: " << std::boolalpha << isres << '\n';

                    if (isres)
                    {
                        auto idxs = opt_split(_line, default_arg_separator, start, iter);


                        auto cl = _line.substr(idxs.cls.start - _line.begin(), idxs.cls.get_dist());
                        std::cout << "\tclass: " << cl << '\n';

                        _options.emplace(cl, idxs.args);

                        auto sz = idxs.args.get_dist();
                        std::cout << "\targs-sz: " << sz << '\n';
                        if (sz)
                        {
                            auto args = _line.substr(idxs.args.start - _line.begin(), sz);
                            std::cout << "\targs: " << args << '\n';
                        }

                        std::cout << "\n";
                    }
                    else
                    {
                        _is_good = false;
                    }
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

        // avoid train of separators
        if (start != iter)
        {
            auto st = _line.substr(start - _line.begin(), iter - start);
            std::cout << "split_opts: "<< st << '\n';
            
            auto isres = is_class(start, iter);
            std::cout << "\tis_class: " << std::boolalpha << isres << '\n';
            if (isres)
            {
                auto idxs = opt_split(_line, default_arg_separator, start, iter);

                auto cl = _line.substr(idxs.cls.start - _line.begin(), idxs.cls.get_dist());
                std::cout << "\tclass: " << cl << '\n';
                _options.emplace(cl, idxs.args);

                auto sz = idxs.args.get_dist();
                std::cout << "\targs-sz: " << sz << '\n';
                if (sz)
                {
                    auto args = _line.substr(idxs.args.start - _line.begin(), sz);
                    std::cout << "\targs: " << args << '\n';
                }

                std::cout << "\n";
            }
            else {_is_good = false;}
        }
    }

    static opt_ind_t opt_split(const std::string& s, char separator, const std::string::const_iterator begin, const std::string::const_iterator end)
    {
        opt_ind_t sind = {};

        auto iter = begin;
        
        while (iter != end)
        {
            if (*iter == separator)
            {
                sind.cls = {begin, iter};
                // class
                auto st = s.substr(begin - s.begin(), iter - begin);
                std::cout << "opt_split: class: " << st << '\n';
                break;
            }
            ++iter;
        }
        sind.args = {s.begin(), s.begin()}; // for avoid UB
        
        if (iter == end)
        {
            sind.cls = {begin, iter};
            auto st = s.substr(begin - s.begin(), iter - begin);
            std::cout << "opt_split: class: " << st << '\n';
            return sind;
        }

        // args
        if (++iter != end)
        {
            sind.args = {iter, end};
            auto st = s.substr(iter - s.begin(), end - iter);
            std::cout << "opt_split: args: " << st << '\n';
        }
        
        return sind;
    }

    static bool is_class(std::string::iterator &begin, const std::string::const_iterator end)
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

    static bool is_separator(char c, const char* seps)
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
    
	static std::vector<std::string> token_split(const std::string& s, char separator)
	{
		std::vector<std::string> conteiner;

		size_t pos = 0;
		size_t iter = 0;

		while (iter < s.size())
		{
			if (s[iter] == separator)
			{
				conteiner.emplace_back(s.substr(pos, iter - pos));
				pos = ++iter;
			}
			++iter;
		}
		conteiner.emplace_back(s.substr(pos, iter - pos));
		return conteiner;
	}

        template <typename Ty,
            std::enable_if_t<std::is_floating_point_v<Ty>, bool> = true>
        static Ty get_arg(const std::string&ln, const ind_t& ind)
        {
            auto sz = ind.get_dist();
            
            if (!sz)
            {
                return Ty{};
            }

            auto s = ln.substr(ind.start - ln.begin(), sz);
            Ty f = static_cast<Ty>(std::atof(s.c_str()));
            return f;
        }

        template <typename Ty,
            std::enable_if_t<std::is_integral_v<Ty>, bool> = true>
        static Ty get_arg(const std::string&ln, const ind_t& ind)
        {
            auto sz = ind.get_dist();
            
            if (!sz)
            {
                return Ty{};
            }

            auto s = ln.substr(ind.start - ln.begin(), sz);
            Ty i = static_cast<Ty>(std::atoi(s.c_str()));
            return i;
        }

        template <typename Ty,
                  std::enable_if_t<std::is_same_v<Ty, std::string>, bool> = true>
        static std::string get_arg(const std::string &ln, const ind_t &ind) 
        {
            auto sz = ind.get_dist();
            
            if (!sz)
            {
                return Ty();
            }
            if (*ind.start == '"')
            {
                auto its = ind.start + 1;
                auto ins = ind.end - 1;

                if (its != ins) // avoid empty large srtings like: ""
                {
                    auto s = ln.substr(its - ln.begin(), ins - its);
                    return s;                    
                }
                return std::string();
            }
            
            auto s = ln.substr(ind.start - ln.begin(), sz);
            return s;
        }




};