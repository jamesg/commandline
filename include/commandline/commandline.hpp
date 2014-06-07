#ifndef COMMANDLINE_HPP
#define COMMANDLINE_HPP

/*
 * Commandline - a command line options parsing library.
 * Copyright (C) 2014 James Goode.
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/variant.hpp>

namespace commandline
{
    namespace detail
    {
        class parse_visitor;
        class print_visitor;

        class has_description
        {
        public:
            has_description(const std::string& description_) :
                m_description(description_)
            {
            }

            const std::string& description() const
            {
                return m_description;
            }

        private:
            const std::string m_description;
        };

        class has_name
        {
        public:
            /*
             * name_: option name without dashes.
             */
            has_name(const std::string& name_) :
                m_name(name_)
            {
            }

            const std::string& name() const
            {
                return m_name;
            }

            /*
             * Get the option name including dashes.
             */
            std::string option_name() const
            {
                std::ostringstream oss;
                oss << "--" << m_name;
                return oss.str();
            }

        private:
            const std::string m_name;
        };
    }

    /*
     * A boolean flag.  The presence of this flag in the command line options
     * will set its value to true.
     */
    class flag : public detail::has_description, public detail::has_name
    {
    public:
        flag(
                const std::string& name_,
                bool&              value,
                const std::string& description_=""
                ) :
            detail::has_name(name_),
            detail::has_description(description_),
            m_value(value)
        {
        }
    private:
        bool& m_value;
        friend class detail::parse_visitor;
    };

    /*
     * A single parameter.  The parameter value follows the option name.
     */
    class parameter : public detail::has_description, public detail::has_name
    {
    public:
        parameter(
                const std::string& name_,
                std::string&       value,
                const std::string& description_=""
                ) :
            detail::has_description(description_),
            detail::has_name(name_),
            m_value(value)
        {
        }
    private:
        std::string& m_value;
        friend class detail::parse_visitor;
    };

    /*
     * A list of parameters.  The parameters follow the option name until
     * another option name (identified by two dashes) or then end of the list
     * is encountered.
     */
    class list : public detail::has_description, public detail::has_name
    {
    public:
        list(
                const std::string&        name_,
                std::vector<std::string>& values,
                const std::string&        description_=""
                ) :
            detail::has_description(description_),
            detail::has_name(name_),
            m_values(values)
        {
        }
    private:
        std::vector<std::string>& m_values;
        friend class detail::parse_visitor;
    };

    typedef boost::variant<flag, parameter, list> option;

    namespace detail
    {
        /*
         * Apply a command line option to (argc, argv).  If the option has a
         * value, store it in the option structure.
         */
        class parse_visitor : public boost::static_visitor<void>
        {
        public:
            parse_visitor(int argc, const char **argv) :
                m_argc(argc),
                m_argv(argv)
            {
            }

            void operator()(flag& f) const
            {
                for(const char **arg = m_argv+1; arg < m_argv+m_argc; ++arg)
                    if(f.option_name().compare(*arg) == 0)
                        f.m_value = true;
            }

            void operator()(parameter& p) const
            {
                for(const char **arg = m_argv+1; arg < m_argv+m_argc; ++arg)
                    if(p.option_name().compare(*arg) == 0 && arg+1 < m_argv+m_argc)
                        p.m_value = std::string(*(arg+1));
            }

            void operator()(list& l) const
            {
                for(const char **arg = m_argv+1; arg < m_argv+m_argc; ++arg)
                    if(l.option_name().compare(*arg) == 0)
                        while(
                                arg+1 < m_argv+m_argc &&
                                std::string(*(arg+1), 0, 2).compare("--") != 0
                                )
                        {
                            l.m_values.push_back(*(arg+1));
                            ++arg;
                        }
            }

        private:
            const int m_argc;
            const char **m_argv;
        };
    }

    /*
     * Parse the usual argc and argv parameters, placing the results in
     * options structures.
     *
     * The options structures may contain only one option with each name.
     */
    void parse(
            const int            argc,
            const char           **argv,
            std::vector<option>& options
            )
    {
        for(option& o : options)
            boost::apply_visitor(detail::parse_visitor(argc, argv), o);
    }

    namespace detail
    {
        /*
         * Print details for a command line option to stderr.
         */
        class print_visitor : public boost::static_visitor<void>
        {
        public:
            void operator()(const flag& f) const
            {
                std::cerr << "    " << f.option_name() << std::endl;
                if(f.description().size())
                    std::cerr << "        " << f.description() << std::endl;
            }

            void operator()(const parameter& p) const
            {
                std::cerr << "    " << p.option_name() << " ARG" << std::endl;
                if(p.description().size())
                    std::cerr << "        " << p.description() << std::endl;
            }

            void operator()(const list& l) const
            {
                std::cerr << "    " << l.option_name() << " LIST" << std::endl;
                if(l.description().size())
                    std::cerr << "        " << l.description() << std::endl;
            }
        };
    }

    /*
     * Print a message describing usage of the program.  Options are
     * printed in the order specified in the vector.
     */
    void print(int argc, const char **argv, const std::vector<option>& options)
    {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] " << std::endl;
        std::cerr << "Available options:" << std::endl;
        for(const option& o : options)
            boost::apply_visitor(detail::print_visitor(), o);
    }
}

#endif

