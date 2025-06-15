#pragma once
#include"EngineCore.h"

namespace Czuch
{
	template<class T>
	class AccessContainer
	{
	public:
		AccessContainer(U32 maxSize);
		virtual ~AccessContainer();

		void RemoveAll();
		I32 Add(T* elem);
		void Remove(I32 handle);
		bool Get(I32 handle, T** result);

		inline std::vector<T*>::iterator Begin() { return m_Elements.begin(); }
		inline std::vector<T*>::iterator End() { return m_Elements.end(); }
		inline std::vector<T*>::reverse_iterator Rbegin() { return m_Elements.rbegin(); }
		inline std::vector<T*>::reverse_iterator Rend() { return m_Elements.rend(); }

		inline std::vector<T*>::const_iterator Begin_const() const { return m_Elements.begin(); }
		inline std::vector<T*>::const_iterator End_const() const { return m_Elements.end(); }
		inline std::vector<T*>::const_reverse_iterator Rbegin_const() const { return m_Elements.rbegin(); }
		inline std::vector<T*>::const_reverse_iterator Rend_const() const { return m_Elements.rend(); }

		inline U32 GetCount() const { return m_Elements.size(); }

	private:
		std::vector<T*> m_Elements;
		std::list<U32> m_FreeSpots;
		U32 m_MaxCapacity;
	};

	template<class T>
	AccessContainer<T>::AccessContainer(U32 maxSize)
	{
		m_Elements.resize(maxSize);
		for (int i = 0; i < maxSize; i++)
		{
			m_Elements[i] = nullptr;
			m_FreeSpots.push_back(i);
		}
		m_MaxCapacity = maxSize;
	}

	template<class T>
	AccessContainer<T>::~AccessContainer()
	{
		RemoveAll();
	}

	template<class T>
	void AccessContainer<T>::RemoveAll()
	{
		for (U32 i = 0; i < m_MaxCapacity; i++)
		{
			if (m_Elements[i] != nullptr)
			{
				delete m_Elements[i];
				m_Elements[i] = nullptr;
				m_FreeSpots.push_back(i);
			}
		}
	}

	template<class T>
	I32 AccessContainer<T>::Add(T* elem)
	{
		if (m_FreeSpots.empty())
		{
			CZUCH_BE_ASSERT(false, "Failed to add new element to access container, there are not free spots");
			LOG_BE_ERROR("{0} Failed to add new element to access container, there are not free spots", "[AccessContainer]");
			return -1;
		}

		U32 freeSpot = m_FreeSpots.back();
		m_FreeSpots.pop_back();

		m_Elements[freeSpot] = elem;
		return freeSpot;
	}

	template<class T>
	inline void AccessContainer<T>::Remove(I32 index)
	{
		if (index < 0 || index >= m_MaxCapacity)
		{
			return;
		}

		if (m_Elements[index] != nullptr)
		{
			delete m_Elements[index];
			m_Elements[index] = nullptr;
			m_FreeSpots.push_back(index);
		}
	}

	template<class T>
	inline bool AccessContainer<T>::Get(I32 index, T** result)
	{
		if (index < 0 || index >= m_MaxCapacity)
		{	
			return false;
		}

		if (m_Elements[index] != nullptr)
		{
			*result = m_Elements[index];
			return true;
		}
		return false;
	}

}

