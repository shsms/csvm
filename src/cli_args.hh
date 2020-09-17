#ifndef CSVM_CLI_ARGS_HH
#define CSVM_CLI_ARGS_HH

#include <string>

struct cli_args {
    std::string filename;
    std::string script;
    int thread_count{1};
    double chunk_size{1e6};
    bool print_engine{false};
};

cli_args parse_args(int argc, char *argv[]);

#endif /* CSVM_CLIARGS_HH */
