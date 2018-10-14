#include "serverProcess.h"
#include "trace.h"
#include <getopt.h>

/*
 * Verbosity level increased for every appearance of --verbose (-v)
 * option in the command line arguments.
 * (Having a global variable doesn't seem like a good practice, but
 * verbosity level will be needed in various classes, and it seems
 * inappropriate to have it as a class member.)
 */
int verbosityLevel = 0;

int main(int argc, char * argv[])
{
    bool helpOption = false;
    /*
     * Parse the command line arguments (taken from 'man 3 getopt').
     * Only --help (-h) and --verbose (-v) are processed for the time
     * being.
     */
    int                 opt, optionIndex;
    const char *        shortOptions  = "hv";
    const struct option longOptions[] = {{"help", no_argument, 0, 'h'}, {"verbose", no_argument, 0, 'v'}, {0, 0, 0, 0}};

    while (true) {
        opt = getopt_long(argc, argv, shortOptions, longOptions, &optionIndex);
        if (opt == -1) break;

        switch (opt) {
        case 'h':
            // Print the help message.
            std::cout << "server side of the nchat program" << std::endl << std::endl;
            std::cout << "options:" << std::endl;
            std::cout << "\t-h, --help: this help text" << std::endl;
            std::cout << "\t-v, --verbose: increase the verbosity level (up to 2)" << std::endl;
            helpOption = true;
            break;
        case 'v':
            // Increase the verbosity level.
            verbosityLevel++;
            break;
        }
    }

    if (!helpOption) {
        TRACE(verbosityLevel, "starting server")
        ServerProcess serverProcess;
        TRACE(verbosityLevel, "running server")
        serverProcess.run();
    }
}
