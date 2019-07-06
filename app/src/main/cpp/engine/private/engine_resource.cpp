//
// Created by andys on 6/14/2019.
//
#include <engine.h>

void
Object3D::init(Engine *engine, VkCommandBuffer &commandBuffer, const char *geo, const char *tex) {
    this->geo = dynamic_cast<Geometry *>(engine->resourceMgr.findOrLoad(engine, commandBuffer, geo));
    this->tex = dynamic_cast<Texture *>(engine->resourceMgr.findOrLoad(engine, commandBuffer, tex));
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

void
Object2D::init(Engine *engine, VkCommandBuffer &commandBuffer, const char *tex, float posX,
               float posY,
               float anchorX, float anchorY, float width, float height, float screenX, float screenY) {
    this->geo = dynamic_cast<Geometry *>(engine->resourceMgr.findOrLoad(engine, commandBuffer, "$UnitQuad2D.geo"));
    this->tex = dynamic_cast<Texture *>(engine->resourceMgr.findOrLoad(engine, commandBuffer, tex));
    this->posX = posX;
    this->posY = posY;
    this->anchorX = anchorX;
    this->anchorY = anchorY;
    this->width = width;
    this->height = height;
    this->ratioW = 1.f / (screenX / 2.f);
    this->ratioH = 1.f / (screenY / 2.f);
    refreshModelMat();
}

void Object2D::destroy() {

}

void Object2D::refreshModelMat() {
    float ndcW = width * ratioW;
    float ndcH = height * ratioH;
    float ndcX = posX * ratioW;
    float ndcY = posY * ratioH;

    /* NDC is [x/y: -1 ~ 1] but underlying geo is a (0,0)-(1,1) quad, so minus 1 here */
    float dx = -1.f + ndcX - ndcW * anchorX;
    float dy = -1.f + ndcY - ndcH * anchorY;

    glm::mat4 result(1.0f);
    result[0][0] = ndcW;
    result[1][1] = ndcH;  /* no need to flip y axis */
    result[2][0] = dx;  /* [col][row] */
    result[2][1] = dy;
    result[2][2] = 0;
    result[3][3] = 0;
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "2d modelMat: %s", glm::to_string(result).c_str());
    modelMat = result;
}

void Engine::loadResources() {
    VkCommandBuffer commandBuffer = beginOneTimeSubmitCommands();

    Object3D obj3d[2];
    obj3d[0].init(this, commandBuffer, "plane.geo", "statue.jpg");
    obj3d[0].setPostion(0, 0, -0.75f);
    obj3d[0].animController.tMax = 3;
    obj3d[0].animController.rotZ[0] = glm::vec2(-3, -900);
    obj3d[0].animController.rotZ.push_back(glm::vec2(1.5f, -90));
    obj3d[0].animController.rotZ.push_back(glm::vec2(3, 0));
    obj3d[0].animController.rotZ.push_back(glm::vec2(6, -900));
    obj3d[0].refreshModelMat();
    object3ds->emplace("internal/plane.obj3d", obj3d[0]);

    obj3d[1].init(this, commandBuffer, "$Rand1.geo", "earth.png");
    obj3d[1].setPostion(0, 0, 0);
    obj3d[1].refreshModelMat();
    object3ds->emplace("internal/rand1.obj3d", obj3d[1]);

    Object2D obj2d[2];
    float screenX = physicalDeviceSurfaceCapabilities.currentExtent.width;
    float screenY = physicalDeviceSurfaceCapabilities.currentExtent.height;
    float sideMargin = 200.f, bottomMargin = 200.f;
    float ballDimension = 128.f;
    obj2d[0].init(this, commandBuffer, "ball.png", sideMargin, screenY - bottomMargin, 0.5f, 0.5f,
                  ballDimension, ballDimension, screenX, screenY);
    object2ds->emplace("internal/btnLeft.obj2d", obj2d[0]);
    obj2d[1].init(this, commandBuffer, "ball.png", screenX - sideMargin, screenY - bottomMargin, 0.5f, 0.5f,
                  ballDimension, ballDimension, screenX, screenY);
    object2ds->emplace("internal/btnRight.obj2d", obj2d[1]);

    endOneTimeSubmitCommandsSyncWithFence(commandBuffer);
}

void ResourceMgr::destroy(Engine *engine) {
    for(auto it = resources.begin(); it != resources.end(); it++) {
        it->second->destroy(engine);
        delete it->second;
    }
    resources.clear();
}

Resource *ResourceMgr::findOrLoad(Engine *engine, VkCommandBuffer &commandBuffer,
                                  const char *name) {
    std::string k(name);
    auto it = resources.find(k);
    if(it != resources.end()) {
        return it->second;
    }

    const char *ext = strrchr(name, '.') + 1;
    std::string fileAbsDir = "/storage/emulated/0/Documents/mydreamland/resources/" + k;
    if(!strcmp(ext, "geo")) {
        Geometry *geo = new Geometry();
        geo->initFromFile(engine, commandBuffer, fileAbsDir.c_str());
        resources.emplace(k, geo);
        return resources[k];
    } else if(!strcmp(ext, "jpg") || !strcmp(ext, "png")) {
        Texture *tex = new Texture();
        tex->initFromFile(engine, commandBuffer, fileAbsDir.c_str());
        resources.emplace(k, tex);
        return resources[k];
    }
    throw std::runtime_error("ResourceMgr::findOrLoad error!");
}