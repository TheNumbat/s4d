
#include "../scene/halfedge.h"

/*
    Merge all faces incident on a given vertex, returning a
    pointer to the merged face.
 */
Halfedge_Mesh::FaceRef Halfedge_Mesh::erase_vertex(Halfedge_Mesh::VertexRef v) {
    
    return v->halfedge()->face();
}

/*
    Merge the two faces on either side of an edge, returning a
    pointer to the merged face.
 */
Halfedge_Mesh::FaceRef Halfedge_Mesh::erase_edge(Halfedge_Mesh::EdgeRef e) {

    return e->halfedge()->face();
}

/*
    Collapse an edge, returning a pointer to the collapsed vertex
*/
Halfedge_Mesh::VertexRef Halfedge_Mesh::collapse_edge(Halfedge_Mesh::EdgeRef e) {

    return e->halfedge()->vertex();
}

/*
    Collapse a face, returning a pointer to the collapsed vertex
*/
Halfedge_Mesh::VertexRef Halfedge_Mesh::collapse_face(Halfedge_Mesh::FaceRef f) {

    return f->halfedge()->vertex();
}

/*
    Flip an edge, returning a pointer to the flipped edge
*/
Halfedge_Mesh::EdgeRef Halfedge_Mesh::flip_edge(Halfedge_Mesh::EdgeRef e) {

    return e;
}

/*
    Split an edge, returning a pointer to the inserted midpoint vertex; the
    halfedge of this vertex should refer to one of the edges in the original
    mesh
*/
Halfedge_Mesh::VertexRef Halfedge_Mesh::split_edge(Halfedge_Mesh::EdgeRef e) {
    
    return e->halfedge()->vertex();
}

/*
    Creates a face in place of the vertex, returning a pointer to the new face
*/
Halfedge_Mesh::FaceRef Halfedge_Mesh::bevel_vertex(Halfedge_Mesh::VertexRef v) {

    return v->halfedge()->face();
}

/*
    Creates a face in place of the edge, returning a pointer to the new face
*/
Halfedge_Mesh::FaceRef Halfedge_Mesh::bevel_edge(Halfedge_Mesh::EdgeRef e) {

    return e->halfedge()->face();
}

/*
    Insets a face into the given face, returning a pointer to the new center face
*/
Halfedge_Mesh::FaceRef Halfedge_Mesh::bevel_face(Halfedge_Mesh::FaceRef f) {

    return f;
}

/*
    Splits all non-triangular faces into triangles.
*/
void Halfedge_Mesh::triangulate() {

}


