//
// Created by andys on 5/31/2019.
//

#ifndef MYDREAMLAND_SHADERHELPER_H
#define MYDREAMLAND_SHADERHELPER_H

#include "engine.h"

std::vector<char> readFile(struct engine *engine, const char *fileName);
VkShaderModule createShaderModule(struct engine *engine, const std::vector<char> &code);

#endif //MYDREAMLAND_SHADERHELPER_H
