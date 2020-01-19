/*
	Halfedge_Mesh.h

	Written By Keenan Crane for 15-462 Assignment 2.
*/

/*
	A Halfedge_Mesh is a data structure that makes it easy to iterate over (and
	modify) a polygonal mesh.  The basic idea is that each edge of the mesh
	gets associated with two "halfedges," one on either side, that point in
	opposite directions.  These halfedges essentially serve as the "glue"
	between different mesh elements (vertices, edges, and faces).  A half edge
	mesh has the same basic flavor as a tree or linked list data structure:
	each node has pointers that reference other nodes.  In particular, each
	half edge points to:

	   -its root vertex,
	   -its associated edge,
	   -the face it sits on,
	   -its "twin", i.e., the halfedge on the other side of the edge,
	   -and the next halfedge in cyclic order around the face.

	Vertices, edges, and faces each point to just one of their incident
	halfedges.  For instance, an edge will point arbitrarily to either
	its "left" or "right" halfedge.  Each vertex will point to one of
	many halfedges leaving that vertex.  Each face will point to one of
	many halfedges going around that face.  The fact that these choices
	are arbitrary does not at all affect the practical use of this data
	structure: they merely provide a starting point for iterating over
	the local region (e.g., walking around a face, or visiting the
	neighbors of a vertex).  A practical example of iterating around a
	face might look like:

	   HalfEdgeIter h = myFace->halfedge();
	   do
	   {
		  // do something interesting with h
		  h = h->next();
	   }
	   while( h != myFace->halfEdge() );

	At each iteration we walk to the "next" halfedge, until we return
	to the original starting point.  A slightly more interesting
	example is iterating around a vertex:

	   HalfEdgeIter h = myVertex->halfedge();
	   do
	   {
		  // do something interesting with h
		  h = h->twin()->next();
	   }
	   while( h != myVertex->halfedge() );

	(Can you draw a picture that explains this iteration?)  A very
	different kind of iteration is when we want to iterate over, say,
   all* the edges of a mesh:

	   for( EdgeIter e = mesh.edges.begin(); e != mesh.edges.end(); e++ )
	   {
		  // do something interesting with e
	   }

	A very important consequence of the halfedge representation is that
	---by design---it can only represent manifold, orientable triangle
	meshes.  I.e., every point should have a neighborhood that looks disk-
	like, and you should be able to assign to each polygon a normal
	direction such that all these normals "point the same way" as you walk
	around the surface.

	At a high level, that's all there is to know about the half edge
	data structure.  But it's worth making a few comments about how this
	particular implementation works---especially how things like boundaries
	are handled.  First and foremost, the "pointers" used in this
	implementation are actually STL iterators.  STL stands for the "standard
	template library," and is a basic part of C++ that provides some very
	convenient and powerful data structures and algorithms---if you've never
	looked at STL before, now would be a great time to get familiar!  At
	a high level, STL iterators behave a lot like pointers: they don't store
	data, but rather reference some data that is allocated elsewhere.  And
	the syntax is also very similar; for instance, if p is an iterator, then
	*p yields the value referred to by p.  (As for the rest, Google is a
	terrific resource! :-))

	Rather than accessing raw iterators, the Halfedge_Mesh encapsulates these
	pointers using methods like Halfedge::twin(), Halfedge::next(), etc.  The
	reason for this encapsulation (as in most object-oriented programming)
	is that it allows the user to make changes to the internal representation
	later down the line.  For instance, if you know that the connectivity of
	the mesh is never going to change, you might be able to improve performance
	by (internally) replacing the linked lists with fixed-length arrays,
	without breaking any code that might have been written using the abstract
	interface.  (There are deeper reasons for this kind of encapsulation
	when working with polygon meshes, but that's a story for another time!)

	Finally, some surfaces have "boundary loops," e.g., a pair of pants has
	three boundaries: one at the waist, and two at the ankles.  These boundaries
	are represented by special faces in our halfedge mesh---in fact, rather than
	being stored in the usual list of faces (Halfedge_Mesh::faces), they are
	stored in a separae list of boundary loops (Halfedge_Mesh::boundaries).  Each
	face (boundary or regular) also stored a flag Face::_isBoundary that
	indicates whether or not it is a boundary.  This value can be queried via the
	public method Face::isBoundary() (again: encapsulation!)  So for instance, if
	I wanted to know the area of all polygons that touch a given vertex, I might
	write some code like this:

	   double totalArea = 0.;
	   HalfEdgeIter h = myVertex->halfedge();
	   do
	   {
		  // don't add the area of boundary faces!
		  if( !h->face()->isBoundary() )
		  {
			 totalArea != h->face()->area();
		  }
		  h = h->twin()->next();
	   }
	   while( h != myVertex->halfedge() );

	In other words, whenever I'm processing a face, I should stop and ask: is
	this really a geometric face in my mesh?  Or is it just a "virtual" face
	that represents a boundary loop?  Finally, for convenience, the halfedge
	associated with a boundary vertex is the first halfedge on the boundary.
	In other words, if we want to iterate over, say, all faces touching a
	boundary vertex, we could write

	   HalfEdgeIter h = myBoundaryVertex->halfedge();
	   do
	   {
		  // do something interesting with h
		  h = h->twin()->next();
	   }
	   while( !h->isBoundary() );

	(Notice that this loop will never terminate for an interior vertex!)

	More documentation can be found in the inline comments below.
*/

#pragma once

#include <list>
#include <vector>
#include <string>

#include "../platform/gl.h"

class Halfedge_Mesh {
public:
	/*
		For code clarity, we often want to distinguish between
		an integer that encodes an index (an "ordinal" number)
		from an integer that encodes a size (a "cardinal" number).
	*/
	using Index = size_t;
	using Size = size_t;

	Halfedge_Mesh() {}
	Halfedge_Mesh(const GL::Mesh& mesh);
	Halfedge_Mesh(const std::vector<std::vector<Index>>& polygons, const std::vector<GL::Mesh::Vert>& verts);
	Halfedge_Mesh(const Halfedge_Mesh& src) = delete;
	Halfedge_Mesh(Halfedge_Mesh&& src);
	~Halfedge_Mesh() {}

	void operator=(const Halfedge_Mesh& src) = delete;
	void operator=(Halfedge_Mesh&& src);

	/*
		A Halfedge_Mesh is comprised of four atomic element types:
		vertices, edges, faces, and halfedges.
	*/
	class Vertex;
	class Edge;
	class Face;
	class Halfedge;

	/*
		Rather than using raw pointers to mesh elements, we store references
		as STL::iterators---for convenience, we give shorter names to these
		iterators (e.g., EdgeIter instead of list<Edge>::iterator).
	*/
	using VertexRef = std::list<Vertex>::iterator;
	using EdgeRef = std::list<Edge>::iterator;
	using FaceRef = std::list<Face>::iterator;
	using HalfedgeRef = std::list<Halfedge>::iterator;

	/*
		We also need "const" iterator types, for situations where a method takes
		a constant reference or pointer to a Halfedge_Mesh.  Since these types are
		used so frequently, we will use "CIter" as a shorthand abbreviation for
		"constant iterator."
	*/
	using VertexCRef = std::list<Vertex>::const_iterator;
	using EdgeCRef = std::list<Edge>::const_iterator;
	using FaceCRef = std::list<Face>::const_iterator;
	using HalfedgeCRef = std::list<Halfedge>::const_iterator;

	class Vertex {
	public:
		HalfedgeRef& halfedge() {return _halfedge;}
		HalfedgeCRef halfedge() const {return _halfedge;}
		Vec3 pos, norm;
	private:
		HalfedgeRef _halfedge;
	};
	class Edge {
	public:
		HalfedgeRef& halfedge() {return _halfedge;}
		HalfedgeCRef halfedge() const {return _halfedge;}
	private:
		HalfedgeRef _halfedge;
	};
	class Face {
	public:
		Face(bool is_boundary = false) : boundary(is_boundary) {}
		HalfedgeRef& halfedge() {return _halfedge;}
		HalfedgeCRef halfedge() const {return _halfedge;}
		bool is_boundary() const {return boundary;}
	private:
		HalfedgeRef _halfedge;
		bool boundary = false;
	};
	class Halfedge {
	public:
		HalfedgeRef& twin() {return _twin;}
		HalfedgeCRef twin() const {return _twin;}
		HalfedgeRef& next() {return _next;}
		HalfedgeCRef next() const {return _next;}
		VertexRef& vertex() {return _vertex;}
		VertexCRef vertex() const {return _vertex;}
		EdgeRef& edge() {return _edge;}
		EdgeCRef edge() const {return _edge;}
		FaceRef& face() {return _face;}
		FaceCRef face() const {return _face;}
	private:
		HalfedgeRef _twin, _next;
		VertexRef _vertex;
		EdgeRef _edge;
		FaceRef _face;
	};

	/// Clear mesh of all elements.
	void clear();
	/// Export to renderable vertex-index mesh.
	void to_mesh(GL::Mesh& mesh, bool face_normals) const;
	/// Create mesh from polygon list
	std::string from_poly(const std::vector<std::vector<Index>>& polygons, const std::vector<GL::Mesh::Vert>& verts);
	/// Create mesh from renderable triangle mesh (beware of connectivity, does not de-duplicate vertices)
	std::string from_mesh(const GL::Mesh& mesh);

	/*
		These methods delete a specified mesh element. One should think very, very carefully about
		exactly when and how to delete mesh elements, since other elements will often still point
		to the element that is being deleted, and accessing a deleted element will cause your
		program to crash (or worse!). A good exercise to think about is: suppose you're iterating
		over a linked list, and want to delete some of the elements as you go. How do you do this
		without causing any problems? For instance, if you delete the current
		element, will you be able to iterate to the next element?  Etc.
	*/
	void erase(HalfedgeRef h) { halfedges.erase(h); }
	void erase(VertexRef v) { vertices.erase(v); }
	void erase(EdgeRef e) { edges.erase(e); }
	void erase(FaceRef f) { faces.erase(f); }

	/*
		These methods allocate new mesh elements, returning a pointer (i.e., iterator) to the new element.
		(These methods cannot have const versions, because they modify the mesh!)
	*/
	HalfedgeRef new_halfedge() { return halfedges.insert(halfedges.end(), Halfedge()); }
	VertexRef new_vertex() { return vertices.insert(vertices.end(), Vertex()); }
	EdgeRef new_edge() { return edges.insert(edges.end(), Edge()); }
	FaceRef new_face(bool is_boundary = false) { return faces.insert(faces.end(), Face(is_boundary)); }

	/*
		These methods return iterators to the beginning and end of the lists of
		each type of mesh element.  For instance, to iterate over all vertices
		one can write

		    for( VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++ )
		    {
		       // do something interesting with v
		    }

		Note that we have both const and non-const versions of these functions;when
		a mesh is passed as a constant reference, we would instead write

		    for( VertexCIter v = ... )

		rather than VertexIter.
	*/
	HalfedgeRef halfedges_begin() { return halfedges.begin(); }
	HalfedgeCRef halfedges_begin() const { return halfedges.begin(); }
	HalfedgeRef halfedges_end() { return halfedges.end(); }
	HalfedgeCRef halfedges_end() const { return halfedges.end(); }
	VertexRef vertices_begin() { return vertices.begin(); }
	VertexCRef vertices_begin() const { return vertices.begin(); }
	VertexRef vertices_end() { return vertices.end(); }
	VertexCRef vertices_end() const { return vertices.end(); }
	EdgeRef edges_begin() { return edges.begin(); }
	EdgeCRef edges_begin() const { return edges.begin(); }
	EdgeRef edges_end() { return edges.end(); }
	EdgeCRef edges_end() const { return edges.end(); }
	FaceRef faces_begin() { return faces.begin(); }
	FaceCRef faces_begin() const { return faces.begin(); }
	FaceRef faces_end() { return faces.end(); }
	FaceCRef faces_end() const { return faces.end(); }

	/// Check if half-edge mesh is valid
	std::string validate() const;

	/// For rendering
	mutable bool render_dirty_flag = false;

private:
	std::list<Vertex> vertices;
	std::list<Edge> edges;
	std::list<Face> faces;
	std::list<Halfedge> halfedges;

	bool check_finite() const;
};

/*
	Some algorithms need to know how to compare two iterators (which comes first?)
	Here we just say that one iterator comes before another if the address of the
	object it points to is smaller. (You should not have to worry about this!)
*/
inline bool operator<(const Halfedge_Mesh::HalfedgeRef& i, const Halfedge_Mesh::HalfedgeRef& j) {
  return &*i < &*j;
}
inline bool operator<(const Halfedge_Mesh::VertexRef& i, const Halfedge_Mesh::VertexRef& j) {
  return &*i < &*j;
}
inline bool operator<(const Halfedge_Mesh::EdgeRef& i, const Halfedge_Mesh::EdgeRef& j) {
  return &*i < &*j;
}
inline bool operator<(const Halfedge_Mesh::FaceRef& i, const Halfedge_Mesh::FaceRef& j) {
  return &*i < &*j;
}
inline bool operator<(const Halfedge_Mesh::HalfedgeCRef& i, const Halfedge_Mesh::HalfedgeCRef& j) {
  return &*i < &*j;
}
inline bool operator<(const Halfedge_Mesh::VertexCRef& i, const Halfedge_Mesh::VertexCRef& j) {
  return &*i < &*j;
}
inline bool operator<(const Halfedge_Mesh::EdgeCRef& i, const Halfedge_Mesh::EdgeCRef& j) {
  return &*i < &*j;
}
inline bool operator<(const Halfedge_Mesh::FaceCRef& i, const Halfedge_Mesh::FaceCRef& j) {
  return &*i < &*j;
}
