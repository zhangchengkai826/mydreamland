//
// Created by andys on 6/20/2019.
//

#ifndef MYDREAMLAND_MDLCG_H
#define MYDREAMLAND_MDLCG_H

#include <vector>

namespace CG {
    class Mesh {
    public:
        /* generate random closed mesh
         * n: num random sample points
         * r: sphere radius
         */
        Mesh(int n, float r);

        std::vector<float> vertices; /* x1, y1, z1, x2, ... */
        std::vector<uint32_t> indices;
    };
}

#endif //MYDREAMLAND_MDLCG_H
