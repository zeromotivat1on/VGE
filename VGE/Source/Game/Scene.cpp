#include "Scene.h"
#include "ECS/Coordinator.h"

vge::Scene::Scene(const std::string& name)
    : _Name(name)
{
}

vge::NodeComponent* vge::Scene::FindNode(const std::string& nodeName)
{
    for (auto rootNode : _Root->Children)
    {
        std::queue<NodeComponent*> traverseNodes = {};
        traverseNodes.push(rootNode);

        while (!traverseNodes.empty())
        {
            auto node = traverseNodes.front();
            traverseNodes.pop();

            if (node->Name == nodeName)
            {
                return node;
            }

            for (auto childNode : node->Children)
            {
                traverseNodes.push(childNode);
            }
        }
    }

    return nullptr;
}
