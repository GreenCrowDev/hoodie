// Copyright 2011-2020 the Polygon Mesh Processing Library developers.
// Distributed under a MIT-style license, see LICENSE.txt for details.

#pragma once

#include "pmp/types.h"
#include "pmp/surface_mesh.h"

namespace pmp {

//! \addtogroup algorithms
//! @{

//! clamp cotangent values as if angles are in [3, 177]
inline Scalar clamp_cot(const Scalar v)
{
    const auto bound = Scalar(19.1); // 3 degrees
    return (v < -bound ? -bound : (v > bound ? bound : v));
}

//! clamp cosine values as if angles are in [3, 177]
inline Scalar clamp_cos(const Scalar v)
{
    const auto bound = Scalar(0.9986); // 3 degrees
    return (v < -bound ? -bound : (v > bound ? bound : v));
}

//! compute angle between two (un-normalized) vectors
inline Scalar angle(const Point& v0, const Point& v1)
{
    return atan2(norm(cross(v0, v1)), dot(v0, v1));
}

//! compute sine of angle between two (un-normalized) vectors
inline Scalar sin(const Point& v0, const Point& v1)
{
    return norm(cross(v0, v1)) / (norm(v0) * norm(v1));
}

//! compute cosine of angle between two (un-normalized) vectors
inline Scalar cos(const Point& v0, const Point& v1)
{
    return dot(v0, v1) / (norm(v0) * norm(v1));
}

//! compute cotangent of angle between two (un-normalized) vectors
inline Scalar cotan(const Point& v0, const Point& v1)
{
    return clamp_cot(dot(v0, v1) / norm(cross(v0, v1)));
}

//! compute area of a triangle given by three points
Scalar triangle_area(const Point& p0, const Point& p1, const Point& p2);

//! Compute area of face \p f.
//! Computes standard area for triangles and norm of vector area for other polygons.
Scalar face_area(const SurfaceMesh& mesh, Face f);

//! Compute the surface area of \p mesh (as sum of face areas).
Scalar surface_area(const SurfaceMesh& mesh);

//! compute (barycentric) Voronoi area of vertex v
Scalar voronoi_area(const SurfaceMesh& mesh, Vertex v);

//! \brief Compute mixed Voronoi area of a vertex
//! \details This version is preferred for irregular triangles with obtuse angles. See \cite meyer_2003_discrete for details.
//! \pre Input mesh needs to be a triangle mesh.
Scalar voronoi_area_mixed(const SurfaceMesh& mesh, Vertex v);

//! compute area assigned to edge e
//! (a face with n edges assigns 1/n of its area to each edge)
Scalar edge_area(const SurfaceMesh& mesh, Edge e);

//! \brief Compute the volume of a mesh
//! \details See \cite zhang_2002_efficient for details.
//! \pre Input mesh needs to be a triangle mesh.
//! \throw InvalidInputException if the input precondition is violated.
Scalar volume(const SurfaceMesh& mesh);

//! compute barycenter/centroid of a face
Point centroid(const SurfaceMesh& mesh, Face f);

//! barycenter/centroid of mesh, computed as area-weighted mean of vertices.
Point centroid(const SurfaceMesh& mesh);

//! \brief Compute dual of a mesh.
//! \warning Changes the mesh in place. All properties are cleared.
void dual(SurfaceMesh& mesh);

//! compute the cotangent weight for edge e
//! \pre Input mesh needs to be a triangle mesh.
double cotan_weight(const SurfaceMesh& mesh, Edge e);

//! compute Laplace vector for vertex v (normalized by Voronoi area)
//! \pre Input mesh needs to be a triangle mesh.
Point laplace(const SurfaceMesh& mesh, Vertex v);

//! @}

} // namespace pmp
