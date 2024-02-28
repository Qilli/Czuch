#include "czpch.h"
#include "RenderContext.h"
#include "GraphicsDevice.h"

namespace Czuch
{
    void RenderObjectInstance::UpdateSceneDataIfRequired(GraphicsDevice* device,BufferHandle buffer)
    {
        if (IsValid())
        {
            Material* m = nullptr;
            if (HANDLE_IS_VALID(overrideMaterial))
            {
                m = device->AccessMaterial(overrideMaterial);
            }
            else
            {
                m = device->AccessMaterial(device->AccessMesh(mesh)->materialHandle);
            }

            if (m != nullptr)
            {
                m->desc.descriptorsDesc[0].Reset();
                m->desc.descriptorsDesc[0].AddBuffer(buffer, 0);
            }
          
        }
    }
}
