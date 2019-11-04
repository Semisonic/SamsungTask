#pragma once

#include <iostream>
#include <memory>

// ------------------------------------------------------------------------- //
/*
 *  Puzzle solver interface
 */
// ------------------------------------------------------------------------- //

class PuzzleSolver {
public:

    virtual ~PuzzleSolver() = default;
    
    virtual void solve () = 0;

    static std::unique_ptr<PuzzleSolver> newInstance (std::istream& is, std::ostream& os);
};