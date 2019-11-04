# SamsungTask

This is a solution for a task that was given at the *Samsung Research* stand at the *C++ Russia 2019* conference. Below you'll find a brief task description and the overview of files committed.
## Task legend
Let's imagine we have an old greenhouse building with one of the side walls made of glass pieces.

        C
        ^       
       / \
    B /   \ D
     |  O  |
     |_____|
    A       E
Inside, ABCDE is basically a vitrage made of multiple adjacent triangle pieces, which are all tightly fitted together with no gaps or overhangs.
The vertices of the vitrage pieces are joined together in such a way that no piece's vertex may touch another piece's rib in its middle. In other words, if O is crossing point of AD and BE, then ABCDE may never be split like BCD, ABD, AOE and DOE - because O would be lying inside AD.

Our goal is to disassemble  the vitrage wall without harming it. Since the walls AB and DE are solid, we may only do it by lifting the vitrage pieces straight up one by one, doing it in such an order that each piece we attempt to lift isn't blocked by any other pieces.
## Technical description
We're given the following input:
 - Number of vertices *N* (3 <= *N* <= 10 000 000), which comprise the vitrage pieces
 - Number of pieces *M* (1 <= *M* <= 10 000 000), each piece represented by a triangle
 - Vertex coordinates pairs *(x, y)*, where -1 000 000 <= *x, y* <= 1 000 000
 - Vertex triplets for each piece, each triplet element a 1-based vertex index

The input is a text formatted as:

> N M
> Xvertex1 Yvertex1
> ...
> XvertexN YvertexN
> P1VI1 P1VI2 P1VI3
> ...
> PMVI1 PMVI2 PMVI3

where (Xvertex*k*, Yvertex*k*) are the coordinates of the *k*-th vertex, and (P*i*VI1, P*i*VI2, P*i*VI3) are the vertex indices of the triplet that forms the *i*-th vitrage piece.

Our goal is to produce the output like:
> P1 P2 ... PM

where P*i* is a 1-based index of a vitrage piece (according to its position in the input), and the indices go from the piece we extract the first to the piece we extract the last. 

## Algorithm description
The algorithm is based on the physical model of pressure application. Each vitrage piece applies the pressure of its weight on the pieces below, while receiving pressure from the pieces above.
The algorithm analyzes the way pieces are placed next to each other, and orders them in the pressure application order, efficiently maintaining this order when the topmost pieces are extracted.

The algorithmic complexity of the algorithm is *O(M log(M) + N)* (which may be further simplified to *O(M log(M))* due to the fact that *N* < 3 *M*), with various techniques to reduce the number of allocations and to increase cache-friendliness.

## Files
The repo files bear the following meaning:

 - *puzzle_solver.h*, *puzzle_solver.cpp* - interface and implementation of the actual puzzle solving algorithm
 - *main.cpp* - main file of the program
 - *test.cpp* - main file of the test program
 - *tests/* - a folder with the pre-defined test scenarios which may be supplied to *test* to check the algorithm's correctness
 - *CMakeLists.txt* - a CMake build scenario

## Compiler requirements
The algorithm requires a C++17-compliant 64-bit compiler.
