//
// Created by andys on 6/20/2019.
//
#include <mdlcg.h>

#include <vector>

#include <android/log.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/convex_hull_3_to_face_graph.h>

void mdlcgtmp() {
    CGAL::Random_points_in_sphere_3<CGAL::Simple_cartesian<float>::Point_3> gen(100.f);
    std::vector<CGAL::Simple_cartesian<float>::Point_3> points;

    CGAL::cpp11::copy_n(gen, 250, std::back_inserter(points));
    CGAL::Delaunay_triangulation_3<CGAL::Simple_cartesian<float>> T;
    T.insert(points.begin(), points.end());

    CGAL::Surface_mesh<CGAL::Simple_cartesian<float>::Point_3> chull;
    CGAL::convex_hull_3_to_face_graph(T, chull);

    __android_log_print(ANDROID_LOG_INFO, "mdlog", "%d", CGAL::num_vertices(chull));
}
