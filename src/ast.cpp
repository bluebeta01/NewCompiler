#include "ast.h"

int node_precedence(NodeType type)
{
	switch (type)
	{
	case NodeType::IDENTIFIER:
	case NodeType::INT_LITERAL:
		return 100;
	case NodeType::REFERENCE:
	case NodeType::DEREFERECE:
		return 90;
	case NodeType::MULTIPLY:
		return 80;
	case NodeType::ADD:
	case NodeType::SUBTRACT:
		return 70;
	case NodeType::ASSIGN:
		return 60;
	}
	return 0;
}

static Node* token_to_node(Token* token)
{
	switch (token->type)
	{
	case TokenType::PLUS:
		return new Node
		{
			.type = NodeType::ADD,
			.token = token
		};
	case TokenType::EQUALS:
		return new Node
		{
			.type = NodeType::ASSIGN,
			.token = token
		};
	case TokenType::MINUS:
		return new Node
		{
			.type = NodeType::SUBTRACT,
			.token = token
		};
	case TokenType::STAR:
		return new Node
		{
			.type = NodeType::MULTIPLY,
			.token = token
		};
	case TokenType::INT_LITERAL:
		return new Node
		{
			.type = NodeType::INT_LITERAL,
			.token = token
		};
	case TokenType::IDENTIFIER:
		return new Node
		{
			.type = NodeType::IDENTIFIER,
			.token = token
		};
	}

	return nullptr;
}

static bool node_is_operator(NodeType type)
{
	switch (type)
	{
	case NodeType::ADD:
	case NodeType::SUBTRACT:
	case NodeType::REFERENCE:
	case NodeType::DEREFERECE:
	case NodeType::MULTIPLY:
	case NodeType::ASSIGN:
		return true;
	}

	return false;
}

//True if the node type can only have a right child
static bool node_is_right_only(NodeType type)
{
	switch (type)
	{
	case NodeType::REFERENCE:
	case NodeType::DEREFERECE:
		return true;
	}

	return false;
}

static Node* parse_tree(const std::vector<Token*>& tokens, int* index)
{
	Node* tree_head = nullptr;
	Node* previous_node = nullptr;
	for (; *index < tokens.size(); (*index)++)
	{
		Token* token = tokens[*index];
		if (token->type == TokenType::CLOSE_PAREN)
		{
			return tree_head;
		}
		Node* node = token_to_node(token);

		if (token->type == TokenType::OPEN_PAREN)
		{
			(*index)++;
			node = parse_tree(tokens, index);

			if (!node)
				return tree_head;

			node->paren = true;

			if (previous_node && node_is_operator(previous_node->type))
			{
				previous_node->left = previous_node->right;
				previous_node->right = node;
				node->parent = previous_node;
				
				previous_node = node;
				continue;
			}

			if (!tree_head)
			{
				tree_head = node;
				continue;
			}

			tree_head->parent = node;
			node->left = tree_head;
			tree_head = node;
			continue;
		}

		//Return early if the token could not converted to a node which can be appended to the tree
		if (node == nullptr)
			return tree_head;

		node->precedence = node_precedence(node->type);

		if (tree_head == nullptr)
		{
			tree_head = node;
			continue;
		}


		//Locate the node we will be appending to or replacing based on the operator precedence
		Node* active_node = tree_head;
		while (active_node)
		{
			if (active_node->type != NodeType::INT_LITERAL &&
				active_node->type != NodeType::IDENTIFIER &&
				active_node->type != NodeType::REFERENCE &&
				active_node->type != NodeType::DEREFERECE &&
				active_node->left == nullptr)
			{
				active_node->left = active_node->right;
				active_node->right = node;
				node->parent = active_node;
				break;
			}

			if (active_node->precedence >= node->precedence || active_node->paren)
			{
				//The active node is the head
				if (!active_node->parent)
				{
					tree_head = node;
					node->right = active_node;
					active_node->parent = node;
					break;
				}

				//The node will be appended to the active node's parent
				if (!active_node->parent->left)
				{
					active_node->parent->left = active_node;
					active_node->parent->right = node;
					node->parent = active_node->parent;
					break;
				}

				//The active node will be replaced in the tree
				active_node->parent->right = node;
				node->parent = active_node->parent;
				node->right = active_node;
				active_node->parent = node;
				break;
			}
			if (!active_node->right)
			{
				active_node->right = node;
				node->parent = active_node;
				break;
			}

			active_node = active_node->right;
		}

		previous_node = node;

		if (!active_node)
		{
			puts("Failed to append token node to tree");
			return tree_head;
		}
	}

	return tree_head;
}

static void print_tree_recurse(FILE* file, Node* tree, int tabs)
{
	if(tree->right)
		print_tree_recurse(file, tree->right, tabs + 1);
	for (int i = 0; i < tabs; i++)
		fwrite("\t", 1, 1, file);
	switch (tree->type)
	{
	case NodeType::INT_LITERAL:
		fprintf(file, "%i", tree->token->parsed_int);
		break;
	case NodeType::IDENTIFIER:
		fprintf(file, "%s", tree->token->name);
		break;
	case NodeType::ASSIGN:
		fprintf(file, "%s", "=");
		break;
	case NodeType::REFERENCE:
		fprintf(file, "%s", "ref");
		break;
	case NodeType::DEREFERECE:
		fprintf(file, "%s", "deref");
		break;
	case NodeType::MULTIPLY:
		fprintf(file, "%s", "*");
		break;
	case NodeType::ADD:
		fprintf(file, "%s", "+");
		break;
	case NodeType::SUBTRACT:
		fprintf(file, "%s", "-");
		break;
	}
	fwrite("\n", 1, 1, file);
	if(tree->left)
		print_tree_recurse(file, tree->left, tabs + 1);
}

static void print_tree(const char* filepath, Node* tree)
{
	if (filepath == nullptr)
		filepath = "/code/ast.txt";
	FILE* file = nullptr;
	file = fopen(filepath, "w");
	if (!file)
	{
		printf("Failed to open file for tree printing %s", filepath);
		return;
	}
	print_tree_recurse(file, tree, 0);
	fclose(file);
}

bool ast_tokens(const std::vector<Token*>& tokens)
{
	int index = 0;
	Node* tree_head = nullptr;

	tree_head = parse_tree(tokens, &index);

	if (tree_head)
	{
		print_tree(nullptr, tree_head);
	}

	return false;
}
