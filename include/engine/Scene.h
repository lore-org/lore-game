#pragma once

#include <engine/Node.h>

class Scene : public Node {
public:
    virtual bool init() override;

    static std::shared_ptr<Scene> create();
};