#include "serverProcess.h"
#include "trace.h"
#include <getopt.h>

int main(int argc, char * argv[])
{
    int verbosityLevel = 0;
    bool helpOption = false;
    /*
     * Parse the command line arguments (taken from 'man 3 getopt').
     * Only --help (-h) and --verbose (-v) are processed for the time
     * being.
     */
    int opt, optionIndex;
    struct option longOptions[] = {
      {"help",	  no_argument,	0,  'h'	},
      {"verbose", no_argument,	0,  'v'	},
      {0,	  0,		0,  0	}
    };

    while (true) {
      opt = getopt_long(argc, argv, "hv", longOptions, &optionIndex);
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
      ServerProcess serverProcess;
      TRACE(verbosityLevel, "starting server");
      serverProcess.run();
    }
}
