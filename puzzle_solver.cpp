#include "puzzle_solver.h"

#include <vector>
#include <array>
#include <utility>
#include <map>
#include <unordered_map>
#include <cassert>
#include <algorithm>

// ------------------------------------------------------------------------- //
/*
 *  Helper types
 */
// ------------------------------------------------------------------------- //

// basic index type and invalid value
using index_type_t = size_t;
constexpr index_type_t invalidIndex = index_type_t(-1);

// vertex
struct Vertex {
    int x;
    int y;

    Vertex(int xCoord, int yCoord) noexcept
    : x(xCoord)
    , y(yCoord) {}
};

// rib
struct Rib {
    // by design, all ribs are vectors from left to right

    const int deltaX;
    const int deltaY;

    // conditions, clause 3. if a rib is shared, is it between two triangles at most
    // it's more efficient to store pointers to triangle objects,
    // but the task output is triangle indices, so they are more handy 
    std::array<index_type_t, 2> ownerTriangles;

    Rib (const Vertex& l, const Vertex& r) noexcept
    : deltaX(r.x - l.x)
    , deltaY(r.y - l.y) {
        ownerTriangles.fill(invalidIndex);
    }

    void setOwner(index_type_t t) noexcept {
        auto& dest = ownerTriangles[0] == invalidIndex ? ownerTriangles[0] : ownerTriangles[1];
        dest = t;
    }

    index_type_t getOtherOwner(index_type_t t) const noexcept {
        // returns either a different owner or invalidIndex
        // doesn't check whether the owner is valid, that's expected
        return ownerTriangles[ownerTriangles[0] == t ? 1 : 0];
    }
};

// triangle
struct Triangle {
    // for each triangle, we only store the ribs whose alignment is counter-clockwise
    // at most two of such ribs may exist for any triangle
    std::array<index_type_t, 2> ribs;

    Triangle () noexcept {
        ribs.fill(invalidIndex);
    }
};

using triangle_vector_t = std::vector<Triangle>;

// ------------------------------------------------------------------------- //
/*
 *  Concrete implementation subclass
 */
// ------------------------------------------------------------------------- //

class PuzzleSolverImpl : public PuzzleSolver {
    using vertex_vector_t = std::vector<Vertex>;
    using rib_vector_t = std::vector<Rib>;

public:

    PuzzleSolverImpl(std::istream& inputStream, std::ostream& resultStream) noexcept
    : m_is(inputStream)
    , m_os(resultStream) {}

    void solve () {
        initRibsAndTriangles();
        generateExtractionSequence();
    }

private:

    void readVertices(vertex_vector_t& vertices, index_type_t vertexCount) {
        vertices.reserve(vertexCount);

        for (decltype(vertexCount) i = 0; i < vertexCount; ++i) {
            int vertexX, vertexY;

            // conditions, clause 3
            m_is >> vertexX >> vertexY;
            vertices.emplace_back(vertexX, vertexY);
        }

        assert(vertices.size() == vertexCount);
    }

    static bool isRibCCWise(const Rib& mainRib, const Rib& secondRib) noexcept {
        /*
         *  if ABC is a triangle and we wanna check whether the directed rib AB 
         *  follows the counter-clockwise vertex traversal, you calculate a vector product
         *  of AB and AC. Its z-part is positive when AB belongs to the ccwise path
         */

        return mainRib.deltaX*secondRib.deltaY - mainRib.deltaY*secondRib.deltaX > 0;
    }
    
    void readAndInitTriangles(const vertex_vector_t& vertices, index_type_t triangleCount) {
        // helper structs and types
        
        using index_pair_hasher = struct {
            std::size_t operator () (const std::pair<index_type_t, index_type_t> &p) const noexcept {
                auto hasher = std::hash<index_type_t>{};
                auto h1 = hasher(p.first);
                auto h2 = hasher(p.second);

                // Mainly for demonstration purposes, i.e. works but is overly simple
                // In the real world, use sth. like boost.hash_combine
                return h1 << 32 | h2;
                //return h1 ^ h2;
            }
        };        
        using vertex_index_pair_t = std::pair<index_type_t, index_type_t>;
        using vertex_rib_map_t = std::unordered_map<vertex_index_pair_t, index_type_t, index_pair_hasher>;

        struct _vertex_sorter {
            _vertex_sorter(const vertex_vector_t& vertices) : m_vertices(vertices) {}
            
            vertex_index_pair_t operator() (index_type_t vi1, index_type_t vi2) {
                vertex_index_pair_t vp{vi1, vi2};

                if (m_vertices[vi1].x > m_vertices[vi2].x) {
                    std::swap(vp.first, vp.second);
                }

                return vp;
            }

        private:
            const vertex_vector_t& m_vertices;
        } sortVertices{vertices};

        // main method code
        
        vertex_rib_map_t verticesToRibs;

        m_triangles.reserve(triangleCount);

        for (decltype(triangleCount) triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex) {
            m_triangles.emplace_back();
            Triangle& currentTriangle = m_triangles[triangleIndex];

            std::array<index_type_t, 3> vertexIndices;

            std::for_each(vertexIndices.begin(), vertexIndices.end(), [this](index_type_t& el){
                m_is >> el;
                --el; // conditions, input data indices are 1-based
            });

            static constexpr std::array<std::array<index_type_t, 2>, 3> vertexCombinations{{
                {{1, 2}},
                {{0, 2}},
                {{0, 1}}
            }};

            index_type_t ribPos{0};
            
            for (index_type_t combinationNum = 0; combinationNum < vertexCombinations.size(); ++combinationNum) {
                const auto& curCombination = vertexCombinations[combinationNum];                
                const auto& vertexPair = sortVertices(vertexIndices[curCombination[0]], vertexIndices[curCombination[1]]);
                auto vpIt = verticesToRibs.find(vertexPair);
                auto ribIndex = invalidIndex;

                if (vpIt == verticesToRibs.end()) {
                    m_ribs.emplace_back(vertices[vertexPair.first], vertices[vertexPair.second]);
                    ribIndex = m_ribs.size() - 1;
                    verticesToRibs.emplace(vertexPair, ribIndex);
                } else {
                    ribIndex = vpIt->second;
                }

                auto& rib = m_ribs[ribIndex];
                
                rib.setOwner(triangleIndex);

                if (!rib.deltaX) {
                    // a small optimization: vertical ribs don't apply pressure to other triangles,
                    // so we can easily skip them without evaluating their direction

                    continue;
                }

                // 0, 1 -> 2, 0, 2 -> 1, 1, 2 -> 0
                auto thirdVertexIndex = combinationNum;
                const Rib secondRib{vertices[vertexPair.first], vertices[vertexIndices[thirdVertexIndex]]};

                if (isRibCCWise(rib, secondRib)) {
                    // inserting rib to triangle only if it's ccwise-oriented
                    currentTriangle.ribs[ribPos++] = ribIndex;
                }
            }
        }
    }

    void initRibsAndTriangles () {
        index_type_t vertexCount;
        index_type_t triangleCount;

        // conditions, clause 3 
        m_is >> vertexCount >> triangleCount;

        vertex_vector_t vertices{};

        readVertices(vertices, vertexCount);
        readAndInitTriangles(vertices, triangleCount);
    }

    void outputSingleTriangleIndex (index_type_t ti) {
        // the output indices must be 1-based
        m_os << (ti + 1) << ' ';
    }
    
    void generateExtractionSequence () {
        /*
         *  The main idea is that a triangle lies on its ccwise-oriented ribs. For each triangle,
         *  we'll calculate how many triagles lie on top of it, then we'll sort the triangles
         *  by this number, and will be taking triangles one by one from the front of the sequence
         */

        // first, we wanna build two index structures here:
        //  1) triangle index -> how many other triangles press on it
        //  2) triangle index -> indices of triangles it presses upon
        
        std::vector<int> trianglePressureIndex(m_triangles.size(), 0);
        std::vector<std::array<index_type_t, 2>> trianglePressureReceiversIndex(m_triangles.size(), {{invalidIndex, invalidIndex}});        

        for (index_type_t ti = 0; ti < m_triangles.size(); ++ti) {
            for (const auto& ri : m_triangles[ti].ribs) {
                if (ri == invalidIndex) {
                    // no more ccwise ribs for this triangle
                    break;
                }
                
                auto pressureReceiverTriangle = m_ribs[ri].getOtherOwner(ti);
                
                if (pressureReceiverTriangle == invalidIndex) {
                    // this rib isn't shared - no pressure on other triangles

                    continue;
                }
                
                ++trianglePressureIndex[pressureReceiverTriangle];
                
                auto& pressureReceivers = trianglePressureReceiversIndex[ti];
                pressureReceivers[pressureReceivers[0] == invalidIndex ? 0 : 1] = pressureReceiverTriangle;
            }
        }

        // now, let's build another two index structures:
        //  1) multimap<pressure, triangle index>
        //  2) triangle index -> iterator to its entry in the map

        using pressure_triangle_map_t = std::multimap<int, index_type_t>;

        pressure_triangle_map_t pressureToTriangleMap;
        std::vector<pressure_triangle_map_t::iterator> triangleToMapPosIndex(m_triangles.size(), pressureToTriangleMap.end());

        for (index_type_t ti = 0; ti < m_triangles.size(); ++ti) {
            auto it = pressureToTriangleMap.emplace(trianglePressureIndex[ti], ti);
            triangleToMapPosIndex[ti] = it;
        }

        // finally, let's perform the extraction
        
        while (!pressureToTriangleMap.empty()) {
            const auto& targetIt = pressureToTriangleMap.begin();

            // the puzzle structure guarantees that there must be at least one triangle
            // that's not pressed upon by other triangles
            assert(targetIt->first == 0);

            auto triangleToExtract = targetIt->second;

            // main output action
            outputSingleTriangleIndex(triangleToExtract);

            pressureToTriangleMap.erase(targetIt);

            // now, rearranging the triangles the top triangle was pressing upon
            for (auto ti : trianglePressureReceiversIndex[triangleToExtract]) {
                if (ti == invalidIndex) {
                    break;
                }

                // reducing the pressure by one and modifying the pressure map
                // utilizing nodes allows avoiding reallocation when modifying the map
                auto pressureReceiverNode = pressureToTriangleMap.extract(triangleToMapPosIndex[ti]);
                --pressureReceiverNode.key();
                
                // we expect that the triangles the top triangle was pressing on will be either the new top
                // or very close to it anyway
                triangleToMapPosIndex[ti] = pressureToTriangleMap.insert(pressureToTriangleMap.begin(), std::move(pressureReceiverNode));
            }
        }
    }

private:

    std::istream& m_is;
    std::ostream& m_os;

    rib_vector_t m_ribs;
    triangle_vector_t m_triangles;
};

// ------------------------------------------------------------------------- //
/*
 *  Interface/generator implementation
 */
// ------------------------------------------------------------------------- //

std::unique_ptr<PuzzleSolver> PuzzleSolver::newInstance (std::istream& is, std::ostream& os) {
    return std::make_unique<PuzzleSolverImpl>(is, os);
}
