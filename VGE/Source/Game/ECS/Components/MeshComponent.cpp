#include "MeshComponent.h"
#include "ECS/Components/MaterialComponent.h"

void vge::ComputeShaderVariant(MeshComponent& mesh)
{
    mesh.ShaderVariant.Clear();

    if (mesh.Material)
    {
        for (const auto& [texName, texture] : mesh.Material->Textures)
        {
            std::string upperTexName = texName;
            std::transform(texName.begin(), texName.end(), upperTexName.begin(), ::toupper);

            mesh.ShaderVariant.AddDefine("HAS_" + upperTexName);
        }
    }

    for (const auto& [attrName, attribute] : mesh.VertexAttributes)
    {
        std::string upperAttribName = attrName;
        std::transform(attrName.begin(), attrName.end(), upperAttribName.begin(), ::toupper);
        
        mesh.ShaderVariant.AddDefine("HAS_" + upperAttribName);
    }
}

bool vge::GetAttribute(MeshComponent& mesh, const std::string& attributeName, VertexAttribute& out)
{
    const auto attribIt = mesh.VertexAttributes.find(attributeName);

    if (attribIt == mesh.VertexAttributes.end())
    {
        return false;
    }

    out = attribIt->second;
    
    return true;
}
