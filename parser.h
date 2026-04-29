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
    /// - Support program name (first word in line). For enable see Construcors and parse definition 
    ///
    /// - Support two modes: strict and no-strict. If strict mode enabled any error stop parser. 
    /// Otherwise, if no-strict mode, it is possible bind wrong variable. You can check state by call good() function.
    ///
    /// - For set your variables see bind() function definition.  
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
    ///    1) ---v=1
    ///
    ///    2) - 
    ///
    ///    3) =2 
    ///
    ///    4) --gp=12.2.5 
    ///
    ///    5) --str=\"no end --vc=10,,20 -- -key=val"
    ///
    class parser
    {
    private:

    public:

        struct ind_t
        {
            long start;
            long end;

            size_t get_dist() const {
                auto ds = end - start;
                if (ds < 0)
                {
                    return 0;
                }
                
                return ds;
            }
        };

        struct opt_ind_t
        {
            ind_t cls;
            ind_t args;
        };
    private:
        using container_t = std::map<std::string, std::string>;

    public:
        static constexpr const char* default_line_separators = " \n\t";
        static constexpr char default_arg_separator = '=';

    private:
        container_t _options;
        std::string _pname;
        bool _strict = false;
        bool _is_good = true;

    public:
        parser(bool strict = false) : _strict(strict){}
        
        /// @brief constructor if has args do parse console line automticaly.
        /// @param s console line in std::string format.
        /// @param parse_prog_name set if expect that fisrt word if program name.
        /// @param strict flag for enable/disable strict mode. 
        /// If enabled when first error will met than stop parser algorithm and clear all data.
        /// @param separators array of separators by defult: 'space', '\\n', '\t'.
        parser(const std::string& s, bool parse_prog_name = false, bool strict = false, const char* separators = default_line_separators) 
            : _strict(strict)
        {
            do_parse(s, separators, parse_prog_name);
        }

        /// @brief constructor if has args do parse console line automticaly.
        /// @param s&& console line in std::string format.
        /// @param parse_prog_name set if expect that fisrt word if program name.
        /// @param strict flag for enable/disable strict mode. 
        /// If enabled when first error will met than stop parser algorithm and clear all data.
        /// @param separators array of separators by defult: 'space', '\\n', '\t'.
        parser(std::string&& s, bool parse_prog_name = false, bool strict = false, const char* separators = default_line_separators) 
            : _strict(strict)
        {
            do_parse(s, separators, parse_prog_name);
        }

        parser(const parser& oth) : 
            _options(oth._options),
            _pname(std::move(oth._pname)),
            _strict(oth._strict), 
            _is_good(oth._is_good){}

        parser(parser&& oth) : 
            _options(std::move(oth._options)), 
            _pname(std::move(oth._pname)),
            _strict(oth._strict), 
            _is_good(oth._is_good) {}

        /// @brief parse console line. If called more than once reset all data before run parser algorithm.
        /// @param s console line in std::string format.
        /// @param parse_prog_name set if expect that fisrt word if program name.
        /// @param separators - array of separators by defult: 'space', '\\n', '\t'.
        void parse(const std::string& s, bool parse_prog_name = false, const char* separators = default_line_separators)
        {
            reset();
            do_parse(s, separators, parse_prog_name);
        }

        /// @brief parse console line. If called more than once reset all data before run parser algorithm.
        /// @param s&& console line in std::string format.
        /// @param parse_prog_name set if expect that fisrt word if program name.
        /// @param separators array of separators by defult: 'space', '\\n', '\t'.
        void parse(std::string &&s, bool parse_prog_name = false, const char *separators = default_line_separators)
        {
            reset();
            do_parse(s, separators, parse_prog_name);
        }

        /// @brief add 
        /// @param s 
        /// @param separators 
        void add_args(std::string& s, const char *separators = default_line_separators)
        {
            parsing(s, separators, s.begin());
        }

        /// @brief reset all parser data and error flags. After call this function any bind() return false and good() return true.
        void reset()
        {
            _options.clear();
            _pname.clear();
            _is_good = true;
        }

        /// @brief return error indicator.
        bool good() const { return _is_good; }

        /// @brief log function.
        /// @param os targer stream e.g. std::cout, std::ofstream ...
        void log(std::ostream& os) const 
        {
            os << "cpplog:\n  status: ";
            
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
            if (!_pname.empty())
                os << _pname << '\n';
            else 
                os << "empty\n";

            os << "key-value pairs:\n";

            if (_options.empty())
            {
                os << "  empty\n";
                if (_strict && !_is_good)
                {
                    os << "  because strict mode catch error in line\n";
                }
                return;
            }
            
            for (const auto &i : _options)
            {
                os << "\t" << i.first << ": " << i.second << '\n';
            }
            os << '\n';
        }

        void log() const 
        {
            printf("clog:\n  status: ");
            
            if (!_is_good)
                printf("error\n");
            else 
                printf("good\n");

            printf("  mode: ");
            if (_strict)
                printf("strict\n");
            else        
                printf("no strict\n");

            printf("program name:\n  ");
            
            if (!_pname.empty())
                printf("%s\n", _pname.c_str());
            else 
                printf("  empty\n");

            printf("key-value pairs:\n");

            if (_options.empty())
            {
                printf("  empty\n");
                if (_strict && !_is_good)
                {
                    printf("  because strict mode catch error in line\n");
                }
                return;
            }

            for (const auto &i : _options)
            {
                printf("\t%s: %s\n", i.first.c_str(), i.second.c_str());
            }
            printf("\n");
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
                        *var = get_arg<Ty>(it->second);
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
                        const auto& args = it->second;      //string of args like: "1,2,3,4"
                        
                        size_t dq = 0;
                        arr->clear();

                        size_t st = 0;
                        size_t sz = 0;
                        size_t cntr = 0;

                        for (const auto &iter : args)
                        {
                            if (iter == ',' && dq % 2 == 0)
                            {
                                sz = cntr - st;
                                auto arg = args.substr(st, sz);

                                arr->emplace_back(get_arg<Ty>(arg));
                                st = ++cntr;
                                continue;
                            }
                            else if (iter == '"')
                            {
                                ++dq;
                            }
                            ++cntr;
                        }

                        sz = cntr - st;
                        auto arg = args.substr(st, sz);
                        arr->emplace_back(get_arg<Ty>(arg));

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

            return _pname;
        }

    private:

        // do not clear _good flag
        void strict_reset()
        {
            _options.clear();
            _pname.clear();
        }

        void do_parse(const std::string& s, const char* separators, bool parse_prog_name) {
            if (s.empty())
            {
                return;
            }

            auto iter = s.begin();
            
            if (parse_prog_name)
            {

                if (!std::isalpha(*iter))
                {
                    _is_good = false;
                    return;
                }

                ++iter;

                while (iter != s.end())
                {
                    if (is_separator(*iter, default_line_separators))
                    {
                        break;
                    }
                    // blank is always word of alphabet simbols
                    if (!isalnum(*iter))
                    {
                        _is_good = false;
                        return;
                    }
                    ++iter;
                }

                _pname = s.substr(0, iter - s.begin());

                // for only blank in line
                if (iter == s.end())
                {
                    return;
                }

            }
            parsing(s, separators, iter);
        } 

        void parsing(const std::string& s, const char *separators, const std::string::const_iterator& beg)
        {
            
            auto start = beg;
            auto iter = start;

            size_t dq = 0;

            while (iter != s.end())
            {
                if (is_separator(*iter, separators) && dq % 2 == 0)
                {
                    // avoid train of separators
                    if (start != iter)
                    {
                        auto sep_cnt = is_class(start, iter);
                        if (sep_cnt)
                        {
                            auto idxs = opt_split(s, _is_good, default_arg_separator, start + sep_cnt, iter);
                            if (!_is_good)
                            {
                                if (_strict)
                                {
                                    strict_reset();
                                    return;
                                }
                            }

                            auto c = s.begin() + idxs.args.start;
                            if (*c == '"')
                            {
                                ++idxs.args.start;
                                --idxs.args.end;
                            }
                            
                            _options.emplace(s.substr(idxs.cls.start, idxs.cls.get_dist()), s.substr(idxs.args.start, idxs.args.get_dist()));
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
                    auto idxs = opt_split(s, _is_good, default_arg_separator, start + sep_cnt, iter);
                    if (!_is_good && _strict)
                    {
                        strict_reset();
                        return;
                    }

                    auto c = s.begin() + idxs.args.start;
                    if (*c == '"')
                    {
                        ++idxs.args.start;
                        --idxs.args.end;
                    }

                    _options.emplace(s.substr(idxs.cls.start, idxs.cls.get_dist()), s.substr(idxs.args.start, idxs.args.get_dist()));
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

        static size_t is_class(std::string::const_iterator begin, const std::string::const_iterator end)
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
            static Ty get_arg(const std::string& arg)
            {
                if (arg.empty())
                {
                    return Ty{};
                }

                Ty f = static_cast<Ty>(std::atof(arg.c_str()));
                return f;
            }

            template <typename Ty,
                std::enable_if_t<std::is_integral_v<Ty>, bool> = true>
            static Ty get_arg(const std::string& arg)
            {
                if (arg.empty())
                {
                    return Ty{};
                }

                Ty i = static_cast<Ty>(std::atoi(arg.c_str()));
                return i;
            }

            template <typename Ty,
                    std::enable_if_t<std::is_same_v<Ty, std::string>, bool> = true>
            static std::string get_arg(const std::string& arg) 
            {
                if (arg.empty())
                {
                    return Ty();
                }

                if (arg.front() == '"')
                {
                    size_t its = 1;
                    size_t ins = arg.size() - 1;

                    if (its != ins) // avoid empty large srtings like: ""
                    {
                        auto s = arg.substr(its, ins - its);
                        return s;                    
                    }
                    return std::string();
                }
                
                return arg;
            }
    };
} // namespace Console_parser
