#pragma once
#include"Core/AccessContainer.h"
#include"GraphicsDevice.h"
namespace Czuch
{
	template<class T, class TM>
	class GraphicsResourceAccessContainer : public AccessContainer<T>
	{
	public:
		GraphicsResourceAccessContainer(U32 maxSize);
		TM AddElement(T* elem);
	};

	template<class T, class TM>
	inline GraphicsResourceAccessContainer<T, TM>::GraphicsResourceAccessContainer(U32 maxSize):AccessContainer<T>(maxSize)
	{
	}

	template<class T, class TM>
	inline TM GraphicsResourceAccessContainer<T, TM>::AddElement(T* elem)
	{
		I32 index = Add(elem);
		TM h{};
		h.handle = index;
		if (index == -1)
		{
			INVALIDATE_HANDLE(h);
		}
		return h;
	}

}

