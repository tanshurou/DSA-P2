// ----------- test.cpp  (“test main”) -----------
#include "ResultLogger.hpp"

int main(int argc, char* argv[]) {
    // By default, look for "matches.csv" in the current folder.
    // You can override by passing a different path as the first argument.
    std::string csvPath = "matches.csv";
    if (argc > 1) {
        csvPath = argv[1];
    }

    // “10” is the size of the “most recent” circular buffer.
    runResultLogger(csvPath, 10);
    return 0;
}
