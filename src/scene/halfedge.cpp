
#include "halfedge.h"

HEMesh::HEMesh() {

}
HEMesh::~HEMesh() {

}

HEMesh::Vertex& HEMesh::get(HEMesh::VertexRef ref) {
    return vertices.get(ref);
}
HEMesh::Edge& HEMesh::get(HEMesh::EdgeRef ref) {
    return edges.get(ref);
}
HEMesh::Face& HEMesh::get(HEMesh::FaceRef ref) {
    return faces.get(ref);
}
HEMesh::Halfedge& HEMesh::get(HEMesh::HalfedgeRef ref) {
    return halfedges.get(ref);
}

void HEMesh::erase(HEMesh::VertexRef ref) {
    vertices.erase(ref);
}
void HEMesh::erase(HEMesh::EdgeRef ref) {
    edges.erase(ref);
}
void HEMesh::erase(HEMesh::FaceRef ref) {
    faces.erase(ref);
}
void HEMesh::erase(HEMesh::HalfedgeRef ref) {
    halfedges.erase(ref);
}

HEMesh::Vertex& HEMesh::make_vertex() {
    return vertices.make();
}
HEMesh::Edge& HEMesh::make_edge() {
    return edges.make();
}
HEMesh::Face& HEMesh::make_face() {
    return faces.make();
}
HEMesh::Halfedge& HEMesh::make_halfedge() {
    return halfedges.make();
}

void HEMesh::clear() {
    vertices.clear();
    edges.clear();
    faces.clear();
    halfedges.clear();
}

