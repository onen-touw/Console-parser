#pragma once

#include <string>
#include <vector>
#include <map>
#include <type_traits>
#include <ostream>

namespace Console_parser
{
    /// @brief Class for simple access to console line variables like --a=123
    ///
    /// Support program name (first word in line). For enable see Construcors and parse definition 
    ///
    /// Support two modes: strict and no-strict. If strict mode enabled any error stop parser. 
    /// Otherwise, if no-strict mode, it is possible bind wrong variable. You can check state by call good() function.
    ///
    /// For set your variables see bind() function definition.  
    /// 
    ///
    /// Rules:
    ///
    /// 1) Program name (if expected and flag enabled) shoud starts from alphabet symbol otherwise it set bad flag.
    ///
    /// 2) Every options must be used in special format: --key=value or -key=value. No any spaces from first dash 
    /// to last symbol except large string(see 4) otherwise it set bad flag.
    ///
    /// 3) Arrays must be used in format -key or --key=10,23,42,2. No any spaces...
    ///
    /// 4) Large strings should be double quoted by symbol ".
    ///
    /// Example of bad options:
    ///
    ///     ---v=1
    ///
    ///     - 
    ///
    ///     =2 
    ///
    ///     --gp=12.2.5 
    ///
    ///     --str=\"no end --vc=10,,20 -- -key=val"
    class parser
    {
    private:
        using iterator_t = std::string::const_iterator;

    public:

        struct ind_t
        {
            long start;
            long end;

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
        bool _strict = false;
        bool _is_good = true;
        ind_t _pname = {};

    public:

        parser(bool strict = false) : _strict(strict){}
        
        /// @brief constructor if has args do parse console line automticaly.
        /// @param s console line in std::string format.
        /// @param parse_prog_name set if expect that fisrt word if program name.
        /// @param strict flag for enable/disable strict mode. 
        /// If enabled when first error will met than stop parser algorithm and clear all data.
        /// @param separators array of separators by defult: 'space', '\\n', '\t'.
        parser(const std::string& s, bool parse_prog_name = false, bool strict = false, const char* separators = default_line_separators) 
            : _line(s), _strict(strict)
        {
            do_parse(separators, parse_prog_name);
        }

        /// @brief constructor if has args do parse console line automticaly.
        /// @param s&& console line in std::string format.
        /// @param parse_prog_name set if expect that fisrt word if program name.
        /// @param strict flag for enable/disable strict mode. 
        /// If enabled when first error will met than stop parser algorithm and clear all data.
        /// @param separators array of separators by defult: 'space', '\\n', '\t'.
        parser(std::string&& s,  bool parse_prog_name = false, bool strict = false, const char* separators = default_line_separators) 
            : _line(std::move(s)), _strict(strict)
        {
            do_parse(separators, parse_prog_name);
        }

        parser(const parser& oth) : 
            _options(oth._options), _line(oth._line), 
            _strict(oth._strict), _is_good(oth._is_good), 
            _pname(oth._pname) {}

        parser(parser&& oth) : _options(std::move(oth._options)), _line(std::move(oth._line)), 
            _strict(oth._strict), _is_good(oth._is_good), 
            _pname(oth._pname) {}

        /// @brief parse console line. If called more than once reset all data before run parser algorithm.
        /// @param s console line in std::string format.
        /// @param parse_prog_name set if expect that fisrt word if program name.
        /// @param separators - array of separators by defult: 'space', '\\n', '\t'.
        void parse(const std::string& s, bool parse_prog_name = false, const char* separators = default_line_separators)
        {
            _is_good = true;
            _options.clear();
            _line = s;
            do_parse(separators, parse_prog_name);
        }

        /// @brief parse console line. If called more than once reset all data before run parser algorithm.
        /// @param s&& console line in std::string format.
        /// @param parse_prog_name set if expect that fisrt word if program name.
        /// @param separators array of separators by defult: 'space', '\\n', '\t'.
        void parse(std::string &&s, bool parse_prog_name = false, const char *separators = default_line_separators)
        {
            _is_good = true;
            _options.clear();
            _line = std::move(s);
            do_parse(separators, parse_prog_name);
        }

        /// @brief reset all parser data and error flags. After call this function any bind() return false and good() return true.
        void reset()
        {
            _options.clear();
            _line.clear();
            _is_good = true;
        }

        /// @brief return error indicator.
        bool good() const { return _is_good; }

        /// @brief log function.
        /// @param os targer stream e.g. std::cout, std::ofstream ...
        void log(std::ostream& os) const 
        {
            os << "log:\n  status: ";
            
            if (_is_good)
                os << "good\n";
            else 
                os << "error\n";

            os << "  mode: ";
            if (_strict)
                os << "strict\n";
            else        
                os << "no strict\n";

            os << "program name:\n  ";
            auto ss = get_prog_name();
            if (!ss.empty())
                os << ss << '\n';
            else 
                os << "empty\n";

            if (_options.empty())
            {
                os << "empty\n";
                if (_strict && !_is_good)
                {
                    os << "  because strict mode catch error in line\n";
                }
                return;
            }
            
            os << "parser:\n";
            for (const auto &i : _options)
            {
                // os << "\t" 
                // << i.first << " {" << i.second.start  << "; " << i.second.end << "} (" << i.second.get_dist() << ")\n";
                os << "\t\targs: " << _line.substr(i.second.start, i.second.get_dist()) << '\n';
            }
            os << '\n';
        }

        /// @brief Bind your variable <var> that shoud be found by token <token>. 
        /// @tparam Ty type of variable <var> that you want to bind. Support any floating and integral type and std::string.
        /// @param var pointer to variable that you want to bind. If (var == nullptr) skip binding.
        /// @param token string of tokens. You can write multyple words separated by ',' (comma). Example: "-v,--v,--verbose".
        /// @return true if found option in console line and false otherwise. If strict mode enabled and error catched return false.
        template <typename Ty>
        bool bind(Ty* var, const std::string& token) const
        {
            if ((!_is_good && _strict ) || _options.empty())
            {
                return false;
            }
            
            std::vector<std::string> tks = token_split(token, ',');
            
            for (auto& t : tks)
            {
                auto it = _options.find(t);
                if (it != _options.end())
                {
                    if (var)
                    {
                        *var = get_arg<Ty>(_line, it->second);
                    }
                    return true;
                }
            }
            return false;
        }

        /// @brief find token in console line.
        /// @param token string of tokens. You can write multyple words separated by ',' (comma). Example: "-v,--v,--verbose".
        /// @return true if found option in console line and false otherwise. If strict mode enabled and error catched return false.
        bool find(const std::string& token) const 
        {
            if ((!_is_good && _strict) || _options.empty())
            {
                return false;
            }
            
            std::vector<std::string> tks = token_split(token, ',');
            for (auto& t : tks)
            {
                auto it = _options.find(t);
                if (it != _options.end())
                {
                    return true;
                }
            }
            return false;
        }

        /// @brief Bind your std::vector that shoud be found by token <token>.
        /// @tparam Ty type of vector's item Support any floating and integral type and std::string.
        /// @param arr pointer to vector that you want to bind. If (arr == nullptr) skip binding.
        /// @param token string of tokens. You can write multyple words separated by ',' (comma). Example: "-v,--v,--verbose".
        /// @return true if found option in console line and false otherwise. If strict mode enabled and error catched return false.
        template <typename Ty>
        bool bind(std::vector<Ty>* arr, const std::string& token) const
        {
            if ((!_is_good && _strict) || _options.empty())
            {
                return false;
            }

            std::vector<std::string> tks = token_split(token, ',');

            for (auto& t : tks)
            {
                auto it = _options.find(t);
                if (it != _options.end())
                {
                    if (arr)
                    {
                        auto ind = it->second;      // indeces of start and end of array
                        
                        auto iter = ind.start + _line.begin();
                        auto start = ind.start + _line.begin();
                        size_t dq = 0;

                        arr->clear();

                        while (iter!= ind.end + _line.begin())
                        {
                            if (*iter == ',' && dq % 2 == 0)
                            {
                                ind_t it_ind = {start - _line.begin(), iter - _line.begin()};
                                arr->push_back(get_arg<Ty>(_line, it_ind));
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

                        ind_t it_ind = {start - _line.begin(), iter - _line.begin()};
                        arr->push_back(get_arg<double>(_line, it_ind));
                        arr->shrink_to_fit();
                    }
                    return true;
                }
            }
            return false;
        }

        /// @brief get program name. 
        /// @return return empty string if error was met in strict mode or flag parse_prog_name was setted false
        std::string get_prog_name() const 
        {
            if (!_is_good && _strict)
            {
                return std::string();
            }
            auto sz = _pname.get_dist();
            if (!sz)
            {
                return std::string();
            }
            
            return _line.substr(_pname.start, sz);
        }

    private:

        void strict_reset()
        {
            _options.clear();
            _line.clear();
        }

        void do_parse(const char* separators, bool parse_prog_name) {
            if (_line.empty())
            {
                return;
            }
            
            if (parse_prog_name)
            {
                auto iter = _line.begin();


                while (iter != _line.end())
                {
                    if (is_separator(*iter, default_line_separators))
                    {
                        break;
                    }
                    // blank is always word of alphabet simbols
                    if (!isalpha(*iter))
                    {
                        _is_good = false;
                        return;
                    }
                    ++iter;
                }
                _pname = {0, iter - _line.begin()};

                // for only blank in line
                if (iter == _line.end())
                {
                    return;
                }

                parsing(separators, iter);
                return;
            }
            parsing(separators, _line.begin());
        } 

        void parsing(const char *separators, const std::string::iterator& beg)
        {
            if (_line.empty() || (!_is_good && _strict))
            {
                return;
            }
            
            auto start = beg;
            auto iter = start;

            ind_t ind = {start - _line.begin(), start - _line.begin()};

            size_t dq = 0;

            while (iter != _line.end())
            {
                if (is_separator(*iter, separators) && dq % 2 == 0)
                {
                    ind = {start - _line.begin(), iter - _line.begin()};

                    // avoid train of separators
                    if (start != iter)
                    {
                        auto sep_cnt = is_class(start, iter);
                        if (sep_cnt)
                        {
                            auto idxs = opt_split(_line, _is_good, default_arg_separator, start + sep_cnt, iter);
                            if (!_is_good)
                            {
                                if (_strict)
                                {
                                    strict_reset();
                                    return;
                                }
                            }
                            _options.emplace(_line.substr(idxs.cls.start, idxs.cls.get_dist()), idxs.args);
                        }
                        else
                        {
                            _is_good = false;
                            if (_strict)
                            {
                                strict_reset();
                                return;
                            }
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
                auto sep_cnt = is_class(start, iter);
                if (sep_cnt)
                {
                    auto idxs = opt_split(_line, _is_good, default_arg_separator, start + sep_cnt, iter);
                    if (!_is_good && _strict)
                    {
                            strict_reset();
                            return;
                    }
                    _options.emplace(_line.substr(idxs.cls.start, idxs.cls.get_dist()), idxs.args);
                }
                else {
                    _is_good = false;
                    if (_strict)
                    {
                        strict_reset();
                        return;
                    }
                }
            }
            if (dq % 2 != 0)
            {
                _is_good = false;
                if (_strict)
                {
                    strict_reset();
                    return;
                }
            }
        }

        static opt_ind_t opt_split(const std::string& s, bool& goodflg, char separator, const std::string::const_iterator begin, const std::string::const_iterator end)
        {
            opt_ind_t sind = {};

            auto iter = begin;
            
            // class
            while (iter != end)
            {
                if (*iter == separator)
                {
                    sind.cls = {begin - s.begin(), iter - s.begin()};
                    break;
                }
                ++iter;
            }
            sind.args = {0, 0}; // for avoid UB

            // no error just oprion without arg
            if (iter == end)
            {
                sind.cls = {begin - s.begin(), iter - s.begin()};
                return sind;
            }

            // args
            if (++iter != end)
            {
                // for avoid --a= and --a== 
                if (is_separator(*iter, default_line_separators) && *iter != '=')
                {
                    goodflg = false;
                    return sind;
                }
                sind.args = {iter - s.begin(), end - s.begin()};
            }
            
            return sind;
        }

        static size_t is_class(std::string::iterator begin, const std::string::const_iterator end)
        {
            if (*begin == '-')
            {
                if (++begin != end)
                {
                    if (*begin == '-')
                    {
                        if (++begin != end)
                        {
                            if (!std::isalpha(*begin))
                            {
                                return 0;
                            }
                            return 2;
                        }
                        return 0;
                    }
                    return 1;
                }
            }
            return 0;
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

                auto s = ln.substr(ind.start, sz);
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

                auto s = ln.substr(ind.start, sz);
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
                if (ln[ind.start] == '"')
                {
                    auto its = ind.start + 1;
                    auto ins = ind.end - 1;

                    if (its != ins) // avoid empty large srtings like: ""
                    {
                        auto s = ln.substr(its, ins - its);
                        return s;                    
                    }
                    return std::string();
                }
                
                auto s = ln.substr(ind.start, sz);
                return s;
            }
    };
} // namespace Console_parser
