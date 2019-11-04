#include "puzzle_solver.h"

#include <iostream>
#include <fstream>

// ------------------------------------------------------------------------- //
/*
 *  Task conditions
 * 
 *  1) vertex count and triangle count < 10 million, i.e. are guaranteed
 *     to fit into int
 *  2) vertex coordinates are between -1000000 and 1000000, i.e. are
 *     guaranteed to fit into int
 *  3) no vertex of any triangle lies on any other triangle's rib
 *  4) input format is:
 *           <vertex count> <triangle count>
 *           <vx1> <vy1>
 *           ...
 *           <vxN> <vyN>
 *           <three vertex indices for the triangle 1>
 *           ...
 *           <three vertex indices for the triangle M>
 */
// ------------------------------------------------------------------------- //

int main (int argc, char* argv[]) {
    // assuming the command-line argument is a path to the input data file

    std::ifstream ifs(argv[1]);

    PuzzleSolver::newInstance(ifs, std::cout)->solve();
}
