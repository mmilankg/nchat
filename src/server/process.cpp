#include "process.h"

void Process::bufferToStrings(char * buffer, int bufferLength, std::vector<std::string> & strings) const
{
    int    remainingLength = bufferLength;
    char * buf             = buffer;
    // Process each string terminated with null-byte until the
    // remainingLength drops to 0.
    while (remainingLength > 0) {
        strings.push_back(buf);
        buf += strings.back().length();
        remainingLength -= strings.back().length();
        // Process the null-byte.
        buf++;
        remainingLength--;
    }
}
