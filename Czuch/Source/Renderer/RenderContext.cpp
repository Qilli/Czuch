#include "czpch.h"
#include "RenderContext.h"
#include "GraphicsDevice.h"

namespace Czuch
{
    void RenderObjectInstance::UpdateSceneDataIfRequired(GraphicsDevice* device,BufferHandle buffer,RenderContextFillParams& fillParams)
    {
        if (IsValid())
        {
            MaterialInstance* m = nullptr;
			if (fillParams.forceMaterialForAll)
			{
				m = device->AccessMaterialInstance(fillParams.forcedMaterial);
			}
            else
            {
                if (HANDLE_IS_VALID(overrideMaterial))
                {
                    m = device->AccessMaterialInstance(overrideMaterial);
                }
                else
                {
                    m = device->AccessMaterialInstance(device->AccessMesh(mesh)->materialHandle);
                }
            }

            if (m != nullptr)
            {
                m->params[passIndex].shaderParamsDesc[0].descriptors[0].resource = buffer.handle;
            }
          
        }
    }
}
