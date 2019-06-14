//
// Created by andys on 6/14/2019.
//
#include <engine.h>

void Object3D::init(Geometry *geo, Texture *tex) {
    this->geo = geo;
    this->tex = tex;
    this->animController.t = 0;
    this->animController.tMax = 0;

    for(int i = 0; i < 2; i++) {
        this->animController.posX.push_back(glm::vec2(0, 0));
        this->animController.posY.push_back(glm::vec2(0, 0));
        this->animController.posZ.push_back(glm::vec2(0, 0));
        this->animController.rotX.push_back(glm::vec2(0, 0));
        this->animController.rotY.push_back(glm::vec2(0, 0));
        this->animController.rotZ.push_back(glm::vec2(0, 0));
        this->animController.scaleX.push_back(glm::vec2(0, 1));
        this->animController.scaleY.push_back(glm::vec2(0, 1));
        this->animController.scaleZ.push_back(glm::vec2(0, 1));
    }
}

void Object3D::destroy() {

}

void Object3D::setPostion(float x, float y, float z) {
    animController.posX[1].y = x;
    animController.posY[1].y = y;
    animController.posZ[1].y = z;
}

void Object3D::refreshModelMat() {
    modelMat = animController.advance(0);
}

void Engine::loadResources() {
    VkCommandBuffer commandBuffer = beginOneTimeSubmitCommands();

    std::queue<std::string> resDirQueue;
    std::string rootDir = "/storage/emulated/0/Documents/mydreamland/resources/";
    resDirQueue.push("");

    while(!resDirQueue.empty()) {
        std::string dirRelPath = resDirQueue.front();
        resDirQueue.pop();
        std::string dirAbsPath = rootDir + dirRelPath;
        struct DIR *dir = opendir(dirAbsPath.c_str());

        struct dirent *dirEntry;
        while((dirEntry = readdir(dir)) != nullptr) {
            if(strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0) {
                continue;
            }

            std::string fileRelPath = dirRelPath + dirEntry->d_name;
            std::string fileAbsPath = dirAbsPath + dirEntry->d_name;
            struct stat fileStat;
            lstat(fileAbsPath.c_str(), &fileStat);
            if(S_ISREG(fileStat.st_mode)) {
                char *ext = strrchr(dirEntry->d_name, '.') + 1;
                if(strcmp(ext, "geo") == 0) {
                    Geometry geo;
                    geo.initFromFile(this, commandBuffer, fileAbsPath.c_str());
                    geometries->emplace(fileRelPath, geo);
                } else if(strcmp(ext, "jpg") == 0 || strcmp(ext, "png") == 0) {
                    Texture tex;
                    tex.initFromFile(this, commandBuffer, fileAbsPath.c_str());
                    textures->emplace(fileRelPath, tex);
                }
            } else if(S_ISDIR(fileStat.st_mode)) {
                resDirQueue.push(fileRelPath);
            }
        }

        closedir(dir);
    }

    Object3D obj3d;
    obj3d.init(&(*geometries)["plane.geo"], &(*textures)["statue.jpg"]);
    obj3d.setPostion(0, 0, -0.5f);
    obj3d.animController.tMax = 3;
    obj3d.animController.rotZ[0] = glm::vec2(-3, -900);
    obj3d.animController.rotZ.push_back(glm::vec2(1.5f, -90));
    obj3d.animController.rotZ.push_back(glm::vec2(3, 0));
    obj3d.animController.rotZ.push_back(glm::vec2(6, -900));
    obj3d.refreshModelMat();
    object3ds->emplace("internal/plane.obj3d", obj3d);

    obj3d.setPostion(0, 0, 0);
    obj3d.refreshModelMat();
    object3ds->emplace("internal/plane2.obj3d", obj3d);

    endOneTimeSubmitCommandsSyncWithFence(commandBuffer);
}

void Engine::destroyResources() {
    for(auto it = object3ds->begin(); it != object3ds->end(); it++) {
        it->second.destroy();
    }
    for(auto it = textures->begin(); it != textures->end(); it++) {
        it->second.destroy(this);
    }
    for(auto it = geometries->begin(); it != geometries->end(); it++) {
        it->second.destroy(this);
    }
}