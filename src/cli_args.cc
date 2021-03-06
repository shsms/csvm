#include "cli_args.hh"
#include <CLI/CLI.hpp>
#include <cstdlib>

namespace stdfs = std::filesystem;

cli_args parse_args(int argc, char *argv[]) {
    cli_args args;

    CLI::App app;

    app.add_option("-f", args.in_filename, "input csv filename.  defaults to /dev/stdin");
    app.add_option("-o", args.out_filename, "output filename.  defaults to /dev/stdout");
    app.add_option("script", args.script, "script to execute")->required();
    app.add_option("-t,--temp-dir", args.temp_dir,
                   std::string("dir to create tmp files in.  Default is ") +
                       stdfs::temp_directory_path().string());
    app.add_option("-n", args.thread_count, "number of threads, defaults to 1");
    app.add_option("--chunk_size", args.chunk_size,
                   "size in bytes of each input chunk, defaults to 1e6");
    app.add_flag("--print-engine", args.print_engine, "display how the engine is built and exit");
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        // questions creates imbalance, and answers that fit resolve them.
        // if 42 is the answer,  then the question has to be -42.
        // return -42;
        exit(app.exit(e));
    }

    if (args.chunk_size <= 0) {
        args.chunk_size = 1e6;
    }
    if (args.thread_count <= 0) {
        args.thread_count = 1;
    }
    if (args.temp_dir.empty()) {
        args.temp_dir = stdfs::temp_directory_path().string();
    }
    if (args.in_filename.empty()) {
        args.in_filename = "/dev/stdin";
    }
    if (args.out_filename.empty()) {
        args.out_filename = "/dev/stdout";
    }

    return args;
}
