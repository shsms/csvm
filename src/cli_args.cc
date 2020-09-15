#include "cli_args.hh"
#include <CLI/CLI.hpp>
#include <cstdlib>

cli_args parse_args(int argc, char *argv[]) {
    cli_args args;

    CLI::App app;

    app.add_option("-f", args.filename,
                   "csv filename, required until next_stdin_chunk is implemented")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("script", args.script, "script to execute")->required();
    app.add_option("-n", args.thread_count, "number of threads, defaults to 1");
    app.add_option("--in-queue-size", args.in_queue_size, "defaults to number of threads");
    app.add_option("--out-queue-size", args.out_queue_size, "defaults to number of threads");
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
    if (args.in_queue_size <= 0) {
        args.in_queue_size = args.thread_count;
    }
    if (args.out_queue_size <= 0) {
        args.out_queue_size = args.thread_count;
    }

    return args;
}
