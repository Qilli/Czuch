#pragma once

namespace Czuch
{
	template <typename T>
	struct UpdatableValue
	{
	public:
		UpdatableValue() : m_Value(T()), m_IsDirty(true) {}
		UpdatableValue(T value) : m_Value(value), m_IsDirty(true) {}
		void UpdateValue(T newValue)
		{
			if (newValue != m_Value)
			{
				m_Value = newValue;
				m_IsDirty = true;
			}
		}

		bool IsDirty(bool clearStateAfterCheck = true)
		{
			if (m_IsDirty)
			{
				if (clearStateAfterCheck)
				{
					m_IsDirty = false;
				}
				return true;
			}
			return false;
		}
		void ClearState() { m_IsDirty = false; }
		T GetValue() { return m_Value; }
		T GetValue() const { return m_Value; }
	private:
		T m_Value;
		bool m_IsDirty;
	};

}
