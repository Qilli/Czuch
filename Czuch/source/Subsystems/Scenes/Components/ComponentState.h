#pragma once

namespace Czuch
{
	struct ComponentState
	{
	public:
		ComponentState(bool dirty) : m_Dirty(dirty) {}
		~ComponentState() = default;
		const bool IsDirty() const { return m_Dirty; }
		void SetDirty() { m_Dirty = true; }
		void SetAsClean() { m_Dirty = false; }
	private:
		bool m_Dirty;
	};
}