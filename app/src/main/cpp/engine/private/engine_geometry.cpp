//
// Created by andys on 6/5/2019.
//
#include <engine.h>
#include <mdlcg.h>

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

void Geometry::initFromFile(Engine *engine, VkCommandBuffer commandBuffer,
                            const char *filename) {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    const char vRandPrefix[] = "/storage/emulated/0/Documents/mydreamland/resources/$Rand";
    if(!strncmp(filename, vRandPrefix, strlen(vRandPrefix))) {
        /* init from 'virtual' file (CG obj) */
        CG::Mesh m(100, 1.f);

        Vertex v;
        v.color[0] = v.color[1] = v.color[2] = 1.f;
        int cnt = 0;
        nVertices = 0;
        for(auto e: m.vertices) {
            v.pos[cnt] = e;
            cnt++;
            if(cnt == 3) {
                switch(nVertices % 4) {
                    case 0:
                        v.texCoord[0] = v.texCoord[1] = 0.f;
                        break;
                    case 1:
                        v.texCoord[0] = 1.f;
                        v.texCoord[1] = 0.f;
                        break;
                    case 2:
                        v.texCoord[0] = v.texCoord[1] = 1.f;
                        break;
                    case 3:
                        v.texCoord[0] = 0.f;
                        v.texCoord[1] = 1.f;
                        break;
                }
                vertices.push_back(v);
                nVertices++;
                cnt = 0;
            }
        }

        for(auto e: m.indices) {
            indices.push_back(e);
        }
        nIndices = static_cast<uint32_t>(indices.size());
    } else {
        std::ifstream f(filename);
        std::string ignore;

        f >> ignore >> nVertices;
        for(int i =0; i < nVertices; i++) {
            Vertex vertex;
            f >> vertex;
            vertices.push_back(vertex);
        }

        f >> ignore >> nIndices;
        for(int i = 0; i < nIndices; i++) {
            uint16_t indice;
            f >> indice;
            indices.push_back(indice);
        }

        f.close();
    }

    /* GPU-side commands */
    VkDeviceSize bufferSize;

    bufferSize = sizeof(Vertex) * vertices.size();
    engine->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer, vertexBufferMemory);
    vkCmdUpdateBuffer(commandBuffer, vertexBuffer, 0, bufferSize, vertices.data());

    bufferSize = sizeof(uint16_t) * indices.size();
    engine->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
    vkCmdUpdateBuffer(commandBuffer, indexBuffer, 0, bufferSize, indices.data());
}

void Geometry::destroy(Engine *engine) {
    vkDestroyBuffer(engine->vkDevice, indexBuffer, nullptr);
    vkFreeMemory(engine->vkDevice, indexBufferMemory, nullptr);
    vkDestroyBuffer(engine->vkDevice, vertexBuffer, nullptr);
    vkFreeMemory(engine->vkDevice, vertexBufferMemory, nullptr);
}