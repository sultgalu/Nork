module Nork.Utils;

namespace Nork
{
	void Profiler::GenerateTree()
	{
		root = Node{ .scope = "root" };

		for (size_t i = 0; i < entries.size(); i++)
		{
			Node* node = &root;
			for (int j = 0; j < entries[i].scopes.size() - 1; j++)
			{
				auto findNextNode = [&]()
				{
					for (size_t k = 0; k < node->childs.size(); k++)
					{
						if (node->childs[k]->scope == entries[i].scopes[j])
						{
							return node->childs[k];
						}
					}
					return node->childs.emplace_back(new Node{
						.scope = entries[i].scopes[j],
						.entry = nullptr,
						});
				};

				node = findNextNode();
			}

			node->childs.push_back(new Node{
						.scope = entries[i].scopes.back(),
						.entry = &entries[i],
				});
		}
	}
}