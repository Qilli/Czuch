#pragma once

namespace Czuch
{
	template <typename T>
	class TreeNode
	{
	public:
		TreeNode() = default;
		TreeNode(const TreeNode& other) = default;
		TreeNode(TreeNode&& other) noexcept = default;
		TreeNode& operator=(const TreeNode& other) = default;
		TreeNode& operator=(TreeNode&& other) noexcept = default;
		virtual ~TreeNode() = default;

		TreeNode(const T& data) :data(data) {}
		TreeNode(T&& data) noexcept :data(std::move(data)) {}

		T& GetData() { return data; }
		const T& GetData() const { return data; }

		void SetData(const T& data) { this->data = data; }
		void SetData(T&& data) { this->data = std::move(data); }

		TreeNode* GetParent() { return parent; }
		const TreeNode* GetParent() const { return parent; }

		void SetParent(TreeNode* parent) { this->parent = parent; }
		TreeNode* GetChildAt(int index) { return &children[index]; }
		Array<TreeNode>& GetChildren() { return children; }

		TreeNode* AddChild(const T&& data)
		{
			TreeNode child(data);
			child.SetParent(this);
			children.push_back(std::move(child));
			return &children.back();
		}

		void Clear()
		{
			for (auto& child : children)
			{
				child.Clear();
			}
			children.clear();
			parent = nullptr;
		}
	private:
		TreeNode* parent = nullptr;
		Array<TreeNode> children;
		T data;
	};

}

