#pragma once
#include "tokenize.h"

enum class NodeType
{
	INVALID,
	INT_LITERAL,
	IDENTIFIER,
	REFERENCE,
	DEREFERECE,
	MULTIPLY,
	ADD,
	SUBTRACT,
	ASSIGN
};

enum class BaseType
{
	INVALID,
	U8,
	U16,
	S16,
	STRUCT
};

struct TypeDescriptor
{
	BaseType base_type = BaseType::INVALID;
	int ptr_count = 0;
};

struct Node
{
	NodeType type = NodeType::INVALID;
	int precedence = 0;
	Node* parent = nullptr;
	Node* left = nullptr;
	Node* right = nullptr;
	Token* token = nullptr;
	bool paren = false;
};

extern int node_precedence(NodeType type);
extern bool ast_tokens(const std::vector<Token*>& tokens);
