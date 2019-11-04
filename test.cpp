#include "puzzle_solver.h"

#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    /*
     *  Each program argument is a test name. Running the program like this:
     *      test.exe test-1 test-2
     *  will mean that the program will try looking up the file pairs
     *      test-1.in, test-1.out
     *      test-2.in, test-2.out
     *  and then supply the .in file to the puzzle solver instance, while checking
     *  whether the puzzle solver's output matches the .out file contents
     */

    for (auto i = 1; i < argc; ++i) {
        std::string testPath{argv[i]};

        std::ifstream testInput{testPath + ".in"};
        std::ifstream testOutputExpected{testPath + ".out"};
        
        if (!testInput || !testOutputExpected) {
            std::cout << "Test " << testPath << " failed to load, skipping..." << std::endl;
            continue;
        }
        
        std::ostringstream testOutputActual{};

        PuzzleSolver::newInstance(testInput, testOutputActual)->solve();

        if (testOutputActual.str()
            == static_cast<const std::ostringstream&>(std::ostringstream() << testOutputExpected.rdbuf()).str()
        ) {
            std::cout << "Test " << testPath << " passed" << std::endl;
        } else {
            std::cout << "Test " << testPath << " FAILED" << std::endl;
        }
    }
}