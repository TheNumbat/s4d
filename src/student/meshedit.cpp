
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

    if (f->is_boundary()) return f;

    std::vector<VertexRef> verts;
    std::vector<HalfedgeRef> edges;

    { // Gather previous vertices + edges
        HalfedgeRef h = f->halfedge();
        do {
            edges.push_back(h);
            verts.push_back(h->vertex());
            h = h->next();
        } while(h != f->halfedge());
    }

    std::vector<VertexRef> inner_verts;
    std::vector<EdgeRef> inner_edges;
    std::vector<HalfedgeRef> inner_halfedges;
    std::vector<EdgeRef> outer_edges;
    std::vector<FaceRef> outer_faces;
    std::vector<std::vector<HalfedgeRef>> outer_halfedges;

    { // Create new geometry elements
        for(int i = 0; i < verts.size(); i++) {
            inner_verts.push_back(new_vertex());
            inner_verts[i]->pos = verts[i]->pos;
            inner_edges.push_back(new_edge());
            outer_edges.push_back(new_edge());
            outer_faces.push_back(new_face());
            inner_halfedges.push_back(new_halfedge());

            std::vector<HalfedgeRef> outer;
            outer.push_back(edges[i]);
            outer.push_back(new_halfedge());
            outer.push_back(new_halfedge());
            outer.push_back(new_halfedge());
            outer_halfedges.push_back(outer);
        }
    }

    { // Update all connectivity per new vertex
        for(int i = 0; i < verts.size(); i++) {
            int next = i + 1 == verts.size() ? 0 : i + 1;
            int prev = i - 1 == -1 ? verts.size() - 1 : i - 1;

            // Update inner face vertex/edge
            inner_verts[i]->halfedge() = inner_halfedges[i];
            inner_edges[i]->halfedge() = inner_halfedges[i];

            // Update inner face half-edge
            inner_halfedges[i]->face() = f;
            inner_halfedges[i]->edge() = inner_edges[i];
            inner_halfedges[i]->vertex() = inner_verts[i];
            inner_halfedges[i]->twin() = outer_halfedges[i][2];
            inner_halfedges[i]->next() = inner_halfedges[next];

            // Update outer ring edge / face
            outer_edges[i]->halfedge() = outer_halfedges[i][3];
            outer_faces[i]->halfedge() = outer_halfedges[i][0];

            // Update original outer ring half-edge
            outer_halfedges[i][0]->next() = outer_halfedges[i][1];
            outer_halfedges[i][0]->face() = outer_faces[i];

            // Update new 'incoming' half-edge
            outer_halfedges[i][1]->face() = outer_faces[i];
            outer_halfedges[i][1]->edge() = outer_edges[next];
            outer_halfedges[i][1]->vertex() = verts[next];
            outer_halfedges[i][1]->twin() = outer_halfedges[next][3];
            outer_halfedges[i][1]->next() = outer_halfedges[i][2];

            // Update new outer half-edge that is twin to the inner ring
            outer_halfedges[i][2]->face() = outer_faces[i];
            outer_halfedges[i][2]->edge() = inner_edges[i];
            outer_halfedges[i][2]->vertex() = inner_verts[next];
            outer_halfedges[i][2]->twin() = inner_halfedges[i];
            outer_halfedges[i][2]->next() = outer_halfedges[i][3];

            // Update new 'outgoing' half-edge
            outer_halfedges[i][3]->face() = outer_faces[i];
            outer_halfedges[i][3]->edge() = outer_edges[i];
            outer_halfedges[i][3]->vertex() = inner_verts[i];
            outer_halfedges[i][3]->twin() = outer_halfedges[prev][1];
            outer_halfedges[i][3]->next() = outer_halfedges[i][0];
        }
    }

    // Reassign original face
    f->halfedge() = inner_halfedges[0];
    return f;
}

void Halfedge_Mesh::bevel_vertex_position(const std::vector<Vec3>& start_positions, Halfedge_Mesh::VertexRef vertex, 
                                          float tangent_offset) {

}

void Halfedge_Mesh::bevel_edge_position(const std::vector<Vec3>& start_positions, Halfedge_Mesh::EdgeRef edge, 
                                         float tangent_offset) {

}

void Halfedge_Mesh::bevel_face_position(const std::vector<Vec3>& start_positions, Halfedge_Mesh::FaceRef face, 
                                        float tangent_offset, float normal_offset) {

}

/*
    Splits all non-triangular faces into triangles.
*/
void Halfedge_Mesh::triangulate() {

}


