#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace vge
{
struct NodeComponent;
    
class Scene
{
public:
    Scene() = default;
    Scene(const std::string& name);

public:
    inline void SetName(const std::string& name) { _Name = name; }
    inline const std::string& GetName() const { return _Name; }
    
private:
    std::string _Name;
    NodeComponent* _Root = nullptr;
    std::vector<std::unique_ptr<NodeComponent>> _Nodes;
    std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> components;
};
}   // namespace vge
