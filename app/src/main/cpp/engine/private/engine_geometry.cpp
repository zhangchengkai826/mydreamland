//
// Created by andys on 6/5/2019.
//
#include <engine.h>

VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

std::ifstream& operator>>(std::ifstream &f, glm::vec2 &v) {
    f >> v[0] >> v[1];
    return f;
}

std::ifstream& operator>>(std::ifstream &f, glm::vec3 &v) {
    f >> v[0] >> v[1] >> v[2];
    return f;
}

std::ifstream& operator>>(std::ifstream &f, Vertex &v) {
    f >> v.pos >> v.color >> v.texCoord;
    return f;
}

void Geometry::loadFromFile(const char *filename) {
    std::ifstream f(filename);
    std::string ignore;
    int n;

    f >> ignore >> n;
    for(int i =0; i < n; i++) {
        Vertex vertex;
        f >> vertex;
        vertices.push_back(vertex);
    }

    f >> ignore >> n;
    for(int i = 0; i < n; i++) {
        uint16_t indice;
        f >> indice;
        indices.push_back(indice);
    }

    f.close();
}
