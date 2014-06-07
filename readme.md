Command Line Options
====================

A tiny (one header) library for parsing command line options.

To keep the library as simple as possible, only three argument formats are
supported:

* Boolean flags - the presence of the argument indicates true (e.g.
  '--flag').
* Single arguments - an option name followed by a single value (e.g. '--arg
  value')
* Lists - an option name followed by zero or more values (e.g. '--list a b
  c').

All option names are preceded by two dashes.
This fact is used to differentiate between option names and values (so values
can't contain any dashes, sorry).

Code Example
------------

Access the library using just one header.  The library lives in namespace
commandline.

    #include "commandline/commandline.hpp"

Declare some variables to store the parsed command line options.

    bool show_help = false;
    std::string data_file;
    std::vector<std::string> regions;

Define the command line options.  The short description text is optional.

    std::vector<commandline::option> options{
        commandline::parameter("data", data_file, "Data file path"),
        commandline::flag("help", show_help, "Show this help message"),
        commandline::list("regions", regions, "Regions of interest")
    };

    commandline::parse(argc, argv, options);

Print the name of the program and a short description for each option.

    if(show_help)
    {
        commandline::print(argc, argv, options);
        return 0;
    }

