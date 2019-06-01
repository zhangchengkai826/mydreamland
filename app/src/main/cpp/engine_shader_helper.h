//
// Created by andys on 5/31/2019.
//

#ifndef MYDREAMLAND_ENGINE_SHADER_HELPER_H
#define MYDREAMLAND_ENGINE_SHADER_HELPER_H

#include "engine.h"

std::vector<char> readFile(struct engine *engine, const char *fileName);
VkShaderModule createShaderModule(struct engine *engine, const std::vector<char> &code);

#endif //MYDREAMLAND_ENGINE_SHADER_HELPER_H
