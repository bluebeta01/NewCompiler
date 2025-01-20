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
	case NodeType::VARDECL:
		return 65;
	case NodeType::ASSIGN:
		return 60;
	}
	return 0;
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

static int count_stars(const std::vector<Token*>& tokens, int index)
{
	int count = 0;
	for (; index < tokens.size(); index++)
		if (tokens[index]->type == TokenType::STAR)
			count++;
		else
			return count;
	return count;
}

//Attempts to parse a vardecl node, returs true on sucess and storing result in node
static bool parse_vardecl_node(const std::vector<Token*>& tokens, int* index, Node* node)
{
	int i = *index;
	node->type = NodeType::VARDECL;
	node->type_descriptor.ptr_count = 0;
	Token* token = tokens[i];
	if (token->type != TokenType::IDENTIFIER)
		return false;
	node->token = token;

	i++;
	if (i >= tokens.size())
		return false;
	token = tokens[i];

	if (token->type != TokenType::COLON)
		return false;

	i++;
	if (i >= tokens.size())
		return false;
	token = tokens[i];

	if (token->type == TokenType::IDENTIFIER)
	{
		node->type_descriptor.base_type = BaseType::NOT_EVALUATED;
		node->type_descriptor.type_name = token->name;
	}
	else if (token->type == TokenType::S16)
	{
		node->type_descriptor.type_name = token->name;
		node->type_descriptor.base_type = BaseType::S16;
	}
	else
	{
		return false;
	}

	i++;
	if (i >= tokens.size())
		return false;
	token = tokens[i];

	while (token->type == TokenType::STAR)
	{
		node->type_descriptor.ptr_count++;
		i++;
		if (i >= tokens.size())
			return false;
		token = tokens[i];
	}

	if (token->type == TokenType::SEMICOLON || (token->flags & TOKEN_FLAG_OPERATOR))
	{
		*index = i - 1;
		return true;
	}

	return false;
}

static Node* token_to_node(const std::vector<Token*>& tokens, int* index)
{
	Token* token = tokens[*index];
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
	case TokenType::AMP:
		return new Node
		{
			.type = NodeType::REFERENCE,
			.token = token
		};
	case TokenType::MINUS:
		return new Node
		{
			.type = NodeType::SUBTRACT,
			.token = token
		};
	case TokenType::STAR:
		if (*index == 0 || tokens[*index - 1]->flags & TOKEN_FLAG_OPERATOR)
		{
			return new Node
			{
				.type = NodeType::DEREFERECE,
				.token = token
			};
		}
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
	case TokenType::S16:
	{
		Node n = {};
		if (parse_vardecl_node(tokens, index, &n))
		{
			Node* node = new Node(n);
			return node;
		}
		return nullptr;
	}
	case TokenType::IDENTIFIER:
		Node n = {};
		if (parse_vardecl_node(tokens, index, &n))
		{
			Node* node = new Node(n);
			return node;
		}
		return new Node
		{
			.type = NodeType::IDENTIFIER,
			.token = token
		};
	}

	return nullptr;
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
		if (token->type == TokenType::CLOSE_PAREN || token->type == TokenType::SEMICOLON)
		{
			(*index)++;
			return tree_head;
		}
		Node* node = token_to_node(tokens, index);

		if (token->type == TokenType::OPEN_PAREN)
		{
			(*index)++;
			node = parse_tree(tokens, index);

			if (!node)
				return tree_head;

			//We should now be on the next token. We don't want to increment on the next continue
			(*index)--;

			node->paren = true;

			if (previous_node && node_is_operator(previous_node->type))
			{
				previous_node->left = previous_node->right;
				previous_node->right = node;
				node->parent = previous_node;
				previous_node = node;
				
				continue;
			}

			previous_node = node;

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
				active_node->type != NodeType::VARDECL &&
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
	case NodeType::VARDECL:
		fprintf(file, "%s", "decl");
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
	case NodeType::EXP_SEQUENCE:
		fprintf(file, "%s", "seq");
		break;
	}
	fwrite("\n", 1, 1, file);
	if(tree->left)
		print_tree_recurse(file, tree->left, tabs + 1);
}

void print_tree(const char* filepath, Node* tree)
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

bool parse_expression(const std::vector<Token*>& tokens, int index, Node** node, int* next_index)
{
	Node* tree_head = nullptr;

	tree_head = parse_tree(tokens, &index);
	if (!tree_head)
		return false;

	*next_index = index;
	*node = tree_head;
	return true;
}

bool parse_type(const std::vector<Token*>& tokens, int index, TypeDescriptor* descriptor, int* next_index)
{
	Token* token = tokens[index];
	if (token->type == TokenType::IDENTIFIER)
	{
		descriptor->base_type = BaseType::NOT_EVALUATED;
		descriptor->type_name = token->name;
	}
	else if (token->type == TokenType::S16)
	{
		descriptor->base_type = BaseType::S16;
	}
	else if (token->type == TokenType::VOID)
	{
		descriptor->base_type = BaseType::VOID;
	}
	else
	{
		return false;
	}

	index++;
	if (index >= tokens.size())
		return false;
	token = tokens[index];

	while (token->type == TokenType::STAR)
	{
		descriptor->ptr_count++;
		index++;
		if (index >= tokens.size())
			return false;
		token = tokens[index];
	}

	*next_index = index;
	return true;
}

static bool parse_func_return(const std::vector<Token*>& tokens, int index, int* next_index, FunctionDescriptor* func)
{
	Token* token = tokens[index];
	if (token->type != TokenType::ARROW)
		return false;

	index++;
	if (index >= tokens.size())
		return false;
	token = tokens[index];

	int n = index;
	if (!parse_type(tokens, index, &func->return_type, &n))
	{
		return false;
	}

	*next_index = n;
	return true;
}

bool parse_func_declaration(const std::vector<Token*>& tokens, int index, FunctionDescriptor* descriptor, int* next_index)
{
	Token* token = tokens[index];
	int n = 0;

	if (token->type != TokenType::IDENTIFIER)
		return false;
	descriptor->name = token->name;

	index++;
	if (index >= tokens.size())
		return false;
	token = tokens[index];

	if (token->type != TokenType::COLON)
		return false;

	index++;
	if (index >= tokens.size())
		return false;
	token = tokens[index];

	if (token->type != TokenType::OPEN_PAREN)
		return false;

	index++;
	if (index >= tokens.size())
		return false;
	token = tokens[index];

	if (parse_type(tokens, index, &descriptor->this_type_descriptor, &n) &&
		n < tokens.size() &&
		(tokens[n]->type == TokenType::COMMA || tokens[n]->type == TokenType::CLOSE_PAREN))
	{
		descriptor->has_this = true;
		index = n;
		if (index >= tokens.size())
			return false;
		token = tokens[index];
	}

	if (token->type == TokenType::CLOSE_PAREN)
	{
		index++;
		if (index >= tokens.size())
			return false;
		token = tokens[index];

		if (token->type != TokenType::ARROW)
		{
			descriptor->return_type.base_type = BaseType::VOID;
			descriptor->return_type.ptr_count = 0;
			*next_index = index;
			return true;
		}

		if (!parse_func_return(tokens, index, &n, descriptor))
			return false;

		*next_index = n;
		return true;
	}

	if (token->type == TokenType::COMMA)
	{
		index++;
		if (index >= tokens.size())
			return false;
		token = tokens[index];
	}

	while (true)
	{
		FunctionParameter param = {};

		if (token->type != TokenType::IDENTIFIER)
			return false;

		param.name = token->name;

		index++;
		if (index >= tokens.size())
			return false;
		token = tokens[index];

		if (token->type != TokenType::COLON)
			return false;

		index++;
		if (index >= tokens.size())
			return false;
		token = tokens[index];

		if (!parse_type(tokens, index, &param.type_descriptor, &n))
			return false;

		descriptor->parameters.push_back(param);

		index = n;
		if (index >= tokens.size())
			return false;
		token = tokens[index];

		if (token->type == TokenType::COMMA)
		{
			index++;
			if (index >= tokens.size())
				return false;
			token = tokens[index];
			continue;
		}

		if (token->type == TokenType::CLOSE_PAREN)
		{
			index++;
			if (index >= tokens.size())
				return false;
			token = tokens[index];
			break;
		}

		return false;
	}

	if (token->type != TokenType::ARROW)
	{
		descriptor->return_type.base_type = BaseType::VOID;
		descriptor->return_type.ptr_count = 0;
		*next_index = index;
		return true;
	}

	if (!parse_func_return(tokens, index, &n, descriptor))
		return false;

	*next_index = n;
	return true;
}

//Prepends and expression to head. Possibly replaces the head if it is not a EXP_SEQUENCE node or it is full
static void prepend_expression(Node** head, Node* expression)
{
	if (!*head)
	{
		*head = expression;
		return;
	}

	if ((*head)->type != NodeType::EXP_SEQUENCE || ((*head)->left != nullptr && (*head)->right != nullptr))
	{
		Node* node = new Node{ .type = NodeType::EXP_SEQUENCE };
		node->right = expression;
		node->left = *head;
		(*head)->parent = node;
		*head = node;
		return;
	}

	(*head)->right = expression;
}

bool parse_function(std::vector<Token*>& tokens, int index, FunctionDescriptor* function, int* next_index)
{
	int next = 0;
	if (!parse_func_declaration(tokens, index, function, &next))
		return false;

	index = next;
	if (index >= tokens.size())
		return false;
	Token* token = tokens[index];

	if (token->type != TokenType::OPEN_BRACE)
		return false;

	index++;
	if (index >= tokens.size())
		return false;
	token = tokens[index];

	while (true)
	{
		Node* node;
		if (!parse_expression(tokens, index, &node, &next))
			return false;

		prepend_expression(&function->node, node);

		index = next;
		if (index >= tokens.size())
			return false;
		token = tokens[index];

		if (token->type == TokenType::CLOSE_BRACE)
			break;
	}

	index++;
	*next_index = index;
	return true;
}
