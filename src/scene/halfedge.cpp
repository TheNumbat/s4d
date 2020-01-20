
#include "halfedge.h"

#include <map>
#include <set>
#include <sstream>

Halfedge_Mesh::Halfedge_Mesh(const GL::Mesh& mesh) {
	from_mesh(mesh);
}
Halfedge_Mesh::Halfedge_Mesh(const std::vector<std::vector<Index>>& polygons, const std::vector<GL::Mesh::Vert>& verts) {
	from_poly(polygons, verts);
}
Halfedge_Mesh::Halfedge_Mesh(Halfedge_Mesh&& src) {
	halfedges = std::move(src.halfedges);
	vertices = std::move(src.vertices);
	edges = std::move(src.edges);
	faces = std::move(src.faces);
	render_dirty_flag = src.render_dirty_flag;
}
void Halfedge_Mesh::operator=(Halfedge_Mesh&& src) {
	halfedges = std::move(src.halfedges);
	vertices = std::move(src.vertices);
	edges = std::move(src.edges);
	faces = std::move(src.faces);
	render_dirty_flag = src.render_dirty_flag;
}

void Halfedge_Mesh::clear() {
	halfedges.clear();
	vertices.clear();
	edges.clear();
	faces.clear();
	render_dirty_flag = true;
}

Vec3 Halfedge_Mesh::Face::average() const {
	Vec3 c;
	float d = 0.0f;
	HalfedgeCRef h = _halfedge;
	do {
		c += h->vertex()->pos;
		d += 1.0f;
		h = h->next();
	} while (h != _halfedge);
	return c / d;
}

void Halfedge_Mesh::to_mesh(GL::Mesh& mesh, bool face_normals) const {

	std::vector<GL::Mesh::Vert> verts;
	std::vector<GL::Mesh::Index> idxs;

	if(face_normals) {
		GLuint face_id = 1;
		for(FaceCRef f = faces_begin(); f != faces_end(); f++, face_id++) {

			if(f->is_boundary()) continue;

			std::vector<GL::Mesh::Vert> face_verts;
			HalfedgeCRef h = f->halfedge();
			do {
				VertexCRef v = h->vertex();
				face_verts.push_back({v->pos, v->norm, 0});
				h = h->next();
			} while (h != f->halfedge());

			assert(face_verts.size() >= 3);
			for(size_t i = 1; i <= face_verts.size() - 2; i++) {
				Vec3 v0 = face_verts[0].pos;
				Vec3 v1 = face_verts[i].pos;
				Vec3 v2 = face_verts[i+1].pos;
				Vec3 n = cross(v1 - v0, v2 - v0).unit();
				idxs.push_back(verts.size());
				verts.push_back({v0, n, face_id});
				idxs.push_back(verts.size());
				verts.push_back({v1, n, face_id});
				idxs.push_back(verts.size());
				verts.push_back({v2, n, face_id});
			}
		}

	} else {

		// Need to build this map to get vertex's linear index in O(lg n)
		std::map<VertexCRef, Index> vref_to_idx;
		Index i = 0;
		for (VertexCRef f = vertices_begin(); f != vertices_end(); f++, i++) {
			vref_to_idx[f] = i;
			verts.push_back({f->pos, f->norm, 0});
		}

		for(FaceCRef f = faces_begin(); f != faces_end(); f++) {

			if(f->is_boundary()) continue;
			
			std::vector<Index> face_verts;
			HalfedgeCRef h = f->halfedge();
			do {
				face_verts.push_back(vref_to_idx[h->vertex()]);
				h = h->next();
			} while (h != f->halfedge());

			assert(face_verts.size() >= 3);
			for(size_t i = 1; i <= face_verts.size() - 2; i++) {
				idxs.push_back(face_verts[0]);
				idxs.push_back(face_verts[i]);
				idxs.push_back(face_verts[i+1]);
			}
		}
	}

	mesh = GL::Mesh(std::move(verts), std::move(idxs));
}

std::string Halfedge_Mesh::validate() const {
	
	// TODO(max): many more validation checks (run after student op)
	// so we can return an error instead of crashing

	if(!check_finite()) return "A vertex position or normal has a non-finite value.";
	return {};
}

std::string Halfedge_Mesh::from_mesh(const GL::Mesh& mesh) {
	
	std::vector<std::vector<Index>> poly;
	auto idx = mesh.indices();
	for(Index i = 0; i < idx.size(); i += 3) {
		poly.push_back({idx[i], idx[i+1], idx[i+2]});
	}
	
	std::string err = from_poly(poly, mesh.verts());
	if(!err.empty()) return err;
	
	err = validate();
	if(!err.empty()) return err;
	
	return {};
}

bool Halfedge_Mesh::check_finite() const {

	for (VertexCRef v = vertices_begin(); v != vertices_end(); v++) {
		Vec3 p = v->pos, n = v->norm;
		bool finite = std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z);
		finite = finite && std::isfinite(n.x) && std::isfinite(n.y) && std::isfinite(n.z);
		if(!finite) return false;
	}
	return true;
}

std::string Halfedge_Mesh::from_poly(const std::vector<std::vector<Index>>& polygons, const std::vector<GL::Mesh::Vert>& verts) {
	
	// This method initializes the halfedge data structure from a raw list of
	// polygons, where each input polygon is specified as a list of vertex indices.
	// The input must describe a manifold, oriented surface, where the orientation
	// of a polygon is determined by the order of vertices in the list. Polygons
	// must have at least three vertices.  Note that there are no special conditions
	// on the vertex indices, i.e., they do not have to start at 0 or 1, nor does
	// the collection of indices have to be contiguous.  Overall, this initializer
	// is designed to be robust but perhaps not incredibly fast (though of course
	// this does not affect the performance of the resulting data structure).  One
	// could also implement faster initializers that handle important special cases
	// (e.g., all triangles, or data that is known to be manifold). Since there are
	// no strong conditions on the indices of polygons, we assume that the list of
	// vertex positions is given in lexicographic order (i.e., that the lowest index
	// appearing in any polygon corresponds to the first entry of the list of
	// positions and so on).

	// define some types, to improve readability
	typedef std::vector<Index> IndexList;
	typedef IndexList::const_iterator IndexListCIter;
	typedef std::vector<IndexList> PolygonList;
	typedef PolygonList::const_iterator PolygonListCIter;
	typedef std::pair<Index, Index> IndexPair;  // ordered pair of vertex indices,
											// corresponding to an edge of an
											// oriented polygon

	// Clear any existing elements.
	clear();

	// Since the vertices in our halfedge mesh are stored in a linked list,
	// we will temporarily need to keep track of the correspondence between
	// indices of vertices in our input and pointers to vertices in the new
	// mesh (which otherwise can't be accessed by index).  Note that since
	// we're using a general-purpose map (rather than, say, a vector), we can
	// be a bit more flexible about the indexing scheme: input vertex indices
	// aren't required to be 0-based or 1-based; in fact, the set of indices
	// doesn't even have to be contiguous.  Taking advantage of this fact makes
	// our conversion a bit more robust to different types of input, including
	// data that comes from a subset of a full mesh.

	// maps a vertex index to the corresponding vertex
	std::map<Index, VertexRef> indexToVertex;

	// Also store the vertex degree, i.e., the number of polygons that use each
	// vertex; this information will be used to check that the mesh is manifold.
	std::map<VertexRef, Size> vertexDegree;

	// First, we do some basic sanity checks on the input.
	for (PolygonListCIter p = polygons.begin(); p != polygons.end(); p++) {
		if (p->size() < 3) {
			// Refuse to build the mesh if any of the polygons have fewer than three
			// vertices.(Note that if we omit this check the code will still
			// constructsomething fairlymeaningful for 1- and 2-point polygons, but
			// enforcing this stricterrequirementon the input will help simplify code
			// further downstream, since it can be certain it doesn't have to check for
			// these rather degenerate cases.)
			return "Each polygon must have at least three vertices.";
		}

		// We want to count the number of distinct vertex indices in this
		// polygon, to make sure it's the same as the number of vertices
		// in the polygon---if they disagree, then the polygon is not valid
		// (or at least, for simplicity we don't handle polygons of this type!).
		std::set<Index> polygonIndices;

		// loop over polygon vertices
		for (IndexListCIter i = p->begin(); i != p->end(); i++) {
			polygonIndices.insert(*i);

			// allocate one vertex for each new index we encounter
			if (indexToVertex.find(*i) == indexToVertex.end()) {
				VertexRef v = new_vertex();
				v->halfedge() =
					halfedges.end();  // this vertex doesn't yet point to any halfedge
				indexToVertex[*i] = v;
				vertexDegree[v] = 1;  // we've now seen this vertex only once
			} else {
				// keep track of the number of times we've seen this vertex
				vertexDegree[indexToVertex[*i]]++;
			}
		}  // end loop over polygon vertices

		// check that all vertices of the current polygon are distinct
		Size degree = p->size();  // number of vertices in this polygon
		if (polygonIndices.size() < degree) {
			std::stringstream stream;
			stream << "One of the input polygons does not have distinct vertices!"
				<< std::endl;
			stream << "(vertex indices:";
			for (IndexListCIter i = p->begin(); i != p->end(); i++) {
				stream << " " << *i;
			}
			stream << ")" << std::endl;
			return stream.str();
		}  // end check that polygon vertices are distinct

	}  // end basic sanity checks on input

	// The number of faces is just the number of polygons in the input.
	Size nFaces = polygons.size();
	faces.resize(nFaces);  // allocate storage for faces in our new mesh

	// We will store a map from ordered pairs of vertex indices to
	// the corresponding halfedge object in our new (halfedge) mesh;
	// this map gets constructed during the next loop over polygons.
	std::map<IndexPair, HalfedgeRef> pairToHalfedge;

	// Next, we actually build the halfedge connectivity by again looping over
	// polygons
	PolygonListCIter p;
	FaceRef f;
	for (p = polygons.begin(), f = faces.begin(); p != polygons.end(); p++, f++) {
		
		std::vector<HalfedgeRef> faceHalfedges; // cyclically ordered list of the half
												// edges of this face
		Size degree = p->size();             	// number of vertices in this polygon

		// loop over the halfedges of this face (equivalently, the ordered pairs of
		// consecutive vertices)
		for (Index i = 0; i < degree; i++) {

			Index a = (*p)[i];                 // current index
			Index b = (*p)[(i + 1) % degree];  // next index, in cyclic order
			IndexPair ab(a, b);
			HalfedgeRef hab;

			// check if this halfedge already exists; if so, we have a problem!
			if (pairToHalfedge.find(ab) != pairToHalfedge.end()) {
				std::stringstream stream;
				stream << "Found multiple oriented edges with indices ("
					<< a << ", " << b << ")." << std::endl;
				stream << "This means that either (i) more than two faces contain this "
						"edge (hence the surface is nonmanifold), or"
					<< std::endl;
				stream << "(ii) there are exactly two faces containing this edge, but "
						"they have the same orientation (hence the surface is"
					<< std::endl;
				stream << "not consistently oriented." << std::endl;
				return stream.str();
			} else  // otherwise, the halfedge hasn't been allocated yet
			{
				// so, we point this vertex pair to a new halfedge
				hab = new_halfedge();
				pairToHalfedge[ab] = hab;

				// link the new halfedge to its face
				hab->face() = f;
				hab->face()->halfedge() = hab;

				// also link it to its starting vertex
				hab->vertex() = indexToVertex[a];
				hab->vertex()->halfedge() = hab;

				// keep a list of halfedges in this face, so that we can later
				// link them together in a loop (via their "next" pointers)
				faceHalfedges.push_back(hab);
			}

			// Also, check if the twin of this halfedge has already been constructed
			// (during construction of a different face).  If so, link the twins
			// together and allocate their shared halfedge.  By the end of this pass
			// over polygons, the only halfedges that will not have a twin will hence
			// be those that sit along the domain boundary.
			IndexPair ba(b, a);
			std::map<IndexPair, HalfedgeRef>::iterator iba = pairToHalfedge.find(ba);
			if (iba != pairToHalfedge.end()) {
				HalfedgeRef hba = iba->second;

				// link the twins
				hab->twin() = hba;
				hba->twin() = hab;

				// allocate and link their edge
				EdgeRef e = new_edge();
				hab->edge() = e;
				hba->edge() = e;
				e->halfedge() = hab;
			} else {  // If we didn't find a twin...
				// ...mark this halfedge as being twinless by pointing
				// it to the end of the list of halfedges. If it remains
				// twinless by the end of the current loop over polygons,
				// it will be linked to a boundary face in the next pass.
				hab->twin() = halfedges.end();
			}
		}  // end loop over the current polygon's halfedges

		// Now that all the halfedges of this face have been allocated,
		// we can link them together via their "next" pointers.
		for (Index i = 0; i < degree; i++) {
			Index j =
				(i + 1) % degree;  // index of the next halfedge, in cyclic order
			faceHalfedges[i]->next() = faceHalfedges[j];
		}

	}  // done building basic halfedge connectivity

	// For each vertex on the boundary, advance its halfedge pointer to one that
	// is also on the boundary.
	for (VertexRef v = vertices_begin(); v != vertices_end(); v++) {
		// loop over halfedges around vertex
		HalfedgeRef h = v->halfedge();
		do {
		if (h->twin() == halfedges.end()) {
			v->halfedge() = h;
			break;
		}

		h = h->twin()->next();
		} while (h != v->halfedge());  // end loop over halfedges around vertex

	}  // done advancing halfedge pointers for boundary vertices

	// Next we construct new faces for each boundary component.
	for (HalfedgeRef h = halfedges_begin(); h != halfedges_end();
		h++)  // loop over all halfedges
	{
		// Any halfedge that does not yet have a twin is on the boundary of the
		// domain. If we follow the boundary around long enough we will of course
		// eventually make a closed loop; we can represent this boundary loop by a
		// new face. To make clear the distinction between faces and boundary loops,
		// the boundary face will (i) have a flag indicating that it is a boundary
		// loop, and (ii) be stored in a list of boundaries, rather than the usual
		// list of faces.  The reason we need the both the flag *and* the separate
		// list is that faces are often accessed in two fundamentally different
		// ways: either by (i) local traversal of the neighborhood of some mesh
		// element using the halfedge structure, or (ii) global traversal of all
		// faces (or boundary loops).
		if (h->twin() == halfedges.end()) {
			FaceRef b = new_face(true);
			// keep a list of halfedges along the boundary, so we can link them together
			std::vector<HalfedgeRef> boundaryHalfedges;  

			// We now need to walk around the boundary, creating new
			// halfedges and edges along the boundary loop as we go.
			HalfedgeRef i = h;
			do {
				// create a twin, which becomes a halfedge of the boundary loop
				HalfedgeRef t = new_halfedge();
				// keep a list of all boundary halfedges, in cyclic order
				boundaryHalfedges.push_back(t);
				i->twin() = t;
				t->twin() = i;
				t->face() = b;
				t->vertex() = i->next()->vertex();

				// create the shared edge
				EdgeRef e = new_edge();
				e->halfedge() = i;
				i->edge() = e;
				t->edge() = e;

				// Advance i to the next halfedge along the current boundary loop
				// by walking around its target vertex and stopping as soon as we
				// find a halfedge that does not yet have a twin defined.
				i = i->next();
				while (i != h &&  // we're done if we end up back at the beginning of
								// the loop
					i->twin() != halfedges.end())  // otherwise, we're looking for
													// the next twinless halfedge
													// along the loop
				{
					i = i->twin();
					i = i->next();
				}
			} while (i != h);

			b->halfedge() = boundaryHalfedges.front();

			// The only pointers that still need to be set are the "next" pointers of
			// the twins; these we can set from the list of boundary halfedges, but we
			// must use the opposite order from the order in the list, since the
			// orientation of the boundary loop is opposite the orientation of the
			// halfedges "inside" the domain boundary.
			Size degree = boundaryHalfedges.size();
			for (Index p = 0; p < degree; p++) {
				Index q = (p - 1 + degree) % degree;
				boundaryHalfedges[p]->next() = boundaryHalfedges[q];
			}
		}  // end construction of one of the boundary loops

		// Note that even though we are looping over all halfedges, we will still
		// construct the appropriate number of boundary loops (and not, say, one
		// loop per boundary halfedge).  The reason is that as we continue to
		// iterate through halfedges, we check whether their twin has been assigned,
		// and since new twins may have been assigned earlier in this loop, we will
		// end up skipping many subsequent halfedges.

	}  // done adding "virtual" faces corresponding to boundary loops

	// To make later traversal of the mesh easier, we will now advance the
	// halfedge
	// associated with each vertex such that it refers to the *first* non-boundary
	// halfedge, rather than the last one.
	for (VertexRef v = vertices_begin(); v != vertices_end(); v++) {
		v->halfedge() = v->halfedge()->twin()->next();
	}

	// Finally, we check that all vertices are manifold.
	for (VertexRef v = vertices.begin(); v != vertices.end(); v++) {
		// First check that this vertex is not a "floating" vertex;
		// if it is then we do not have a valid 2-manifold surface.
		if (v->halfedge() == halfedges.end()) {
			return "Some vertices are not referenced by any polygon.";
		}

		// Next, check that the number of halfedges emanating from this vertex in
		// our half edge data structure equals the number of polygons containing
		// this vertex, which we counted during our first pass over the mesh.  If
		// not, then our vertex is not a "fan" of polygons, but instead has some
		// other (nonmanifold) structure.
		Size count = 0;
		HalfedgeRef h = v->halfedge();
		do {
			if (!h->face()->is_boundary()) {
				count++;
			}
			h = h->twin()->next();
		} while (h != v->halfedge());

		if (count != vertexDegree[v]) {
			return "At least one of the vertices is nonmanifold.";
		}
	}  // end loop over vertices

	// Now that we have the connectivity, we copy the list of vertex
	// positions into member variables of the individual vertices.
	if (verts.size() < vertices.size()) {
		std::stringstream stream;
		stream << "The number of vertex positions is different from the number of distinct vertices!"
			<< std::endl;
		stream << "(number of positions in input: " << verts.size() << ")"
			<< std::endl;
		stream << "(  number of vertices in mesh: " << vertices.size() << ")" << std::endl;
		return stream.str();
	}
	
	// Since an STL map internally sorts its keys, we can iterate over the map
	// from vertex indices to vertex iterators to visit our (input) vertices in
	// lexicographic order
	int i = 0;
	for (std::map<Index, VertexRef>::const_iterator e = indexToVertex.begin();
		e != indexToVertex.end(); e++) {
		// grab a pointer to the vertex associated with the current key (i.e., the
		// current index)
		VertexRef v = e->second;

		// set the att of this vertex to the corresponding
		// position in the input
		v->pos = verts[i].pos;
		v->norm = verts[i].norm;
		i++;
	}
	return {};
}
