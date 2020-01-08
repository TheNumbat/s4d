
#pragma once

#include <climits>
#include <vector>
#include <stack>

#include "../lib/log.h"

template<typename T>
class Linear_FList {
public:
    struct Index {
        size_t val = SIZE_MAX;
    };

    T& get(Index idx) {
        assert(idx.val < data.size());
        assert(data.size() == mask.size());
        assert(mask[idx.val]);
        return data[idx.val];
    }
    void erase(Index idx) {
        assert(idx.val < data.size());
        assert(data.size() == mask.size());
        assert(mask[idx.val]);
        mask[idx.val] = false;
        free.push_back(idx);
    }
    T& make() {
        if(free.size()) {
            Index idx = free.back();
            free.pop_back();
            assert(idx.val < data.size());
            assert(data.size() == mask.size());
            assert(!mask[idx.val]);
            mask[idx.val] = true;
            data[idx.val] = T{};
            return data[idx.val];
        }
        Index idx{data.size()};
        data.push_back({});
        mask.push_back(true);
        return data[idx.val];
    }
    void clear() {
        mask.clear();
        data.clear();
        free.clear();
    }

private:
    std::vector<bool> mask;
    std::vector<T> data;
    std::vector<Index> free;
};

class HEMesh {
public:
    HEMesh();
    ~HEMesh();

    struct Vertex; 
    struct Edge;
    struct Face;
    struct Halfedge;

    using VertexRef = Linear_FList<Vertex>::Index;
    using EdgeRef = Linear_FList<Edge>::Index;
    using FaceRef = Linear_FList<Face>::Index;
    using HalfedgeRef = Linear_FList<Halfedge>::Index;

    struct Vertex {
        HalfedgeRef halfedge;
    };
    struct Edge {
        HalfedgeRef halfedge;
    };
    struct Face {
        HalfedgeRef halfedge;
    };
    struct Halfedge {
        VertexRef vertex;
        EdgeRef edge;
        FaceRef face;
        HalfedgeRef twin, next;
    };

    Vertex& get(VertexRef ref);
    Edge& get(EdgeRef ref);
    Face& get(FaceRef ref);
    Halfedge& get(HalfedgeRef ref);
    
    void erase(VertexRef ref);
    void erase(EdgeRef ref);
    void erase(FaceRef ref);
    void erase(HalfedgeRef ref);

    Vertex& make_vertex();
    Edge& make_edge();
    Face& make_face();
    Halfedge& make_halfedge();

    void clear();

private:
    Linear_FList<Vertex> vertices;
    Linear_FList<Edge> edges;
    Linear_FList<Face> faces;
    Linear_FList<Halfedge> halfedges;
};
