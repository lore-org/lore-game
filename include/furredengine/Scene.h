#pragma once

#include <furredengine/Node.h>

namespace FurredEngine {

class Scene : public Node {
public:
    virtual bool init() override;

    static std::shared_ptr<Scene> create();
};

}