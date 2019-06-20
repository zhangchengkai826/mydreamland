//
// Created by andys on 6/20/2019.
//
#include <mdlcg.h>

#include <android/log.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/convex_hull_3_to_face_graph.h>

CG::Mesh::Mesh(int n, float r) {
    CGAL::Random_points_in_sphere_3<CGAL::Simple_cartesian<float>::Point_3> gen(r);
    std::vector<CGAL::Simple_cartesian<float>::Point_3> points;
    CGAL::cpp11::copy_n(gen, 250, std::back_inserter(points));

    CGAL::Delaunay_triangulation_3<CGAL::Simple_cartesian<float>> T;
    T.insert(points.begin(), points.end());

    CGAL::Surface_mesh<CGAL::Simple_cartesian<float>::Point_3> chull;
    CGAL::convex_hull_3_to_face_graph(T, chull);

    for(auto p: chull.points()) {
        vertices.push_back(p.x());
        vertices.push_back(p.y());
        vertices.push_back(p.z());
    }

    for(auto f: chull.faces()) {
        for(auto v: chull.vertices_around_face(chull.halfedge(f))) {
            indices.push_back(v);
        }
    }
}
