#include "MeshComponent.h"
#include "ECS/Components/MaterialComponent.h"

void vge::MeshComponent::ComputeShaderVariant()
{
    ShaderVariant.Clear();

    if (Material)
    {
        for (const auto& [texName, texture] : Material->Textures)
        {
            std::string upperTexName = texName;
            std::transform(texName.begin(), texName.end(), upperTexName.begin(), ::toupper);

            ShaderVariant.AddDefine("HAS_" + upperTexName);
        }
    }

    for (const auto& [attrName, attribute] : VertexAttributes)
    {
        std::string upperAttribName = attrName;
        std::transform(attrName.begin(), attrName.end(), upperAttribName.begin(), ::toupper);
        
        ShaderVariant.AddDefine("HAS_" + upperAttribName);
    }
}

bool vge::MeshComponent::GetAttribute(const std::string& attributeName, VertexAttribute& out)
{
    const auto attribIt = VertexAttributes.find(attributeName);

    if (attribIt == VertexAttributes.end())
    {
        return false;
    }

    out = attribIt->second;
    
    return true;
}
