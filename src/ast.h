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
	ASSIGN,
	VARDECL,
	EXP_SEQUENCE,
	COMMA,
	CALL,
};

enum class BaseType
{
	INVALID,
	U8,
	U16,
	S16,
	VOID,
	STRUCT,
	NOT_EVALUATED,
};

struct TypeDescriptor
{
	BaseType base_type = BaseType::INVALID;
	const char* type_name = nullptr;
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
	TypeDescriptor type_descriptor;
};

struct FunctionParameter
{
	const char* name;
	TypeDescriptor type_descriptor;
};

struct FunctionDescriptor
{
	const char* name;
	std::vector<FunctionParameter> parameters;
	TypeDescriptor return_type;
	TypeDescriptor this_type_descriptor;
	Node* node;
	bool has_this;
};

extern int node_precedence(NodeType type);
extern bool parse_expression(const std::vector<Token*>& tokens, int index, Node** node, int* next_index);
extern bool parse_type(const std::vector<Token*>& tokens, int index, TypeDescriptor* descriptor, int* next_index);
extern bool parse_func_declaration(const std::vector<Token*>& tokens, int index, FunctionDescriptor* descriptor, int* next_index);
extern bool parse_function(std::vector<Token*>& tokens, int index, FunctionDescriptor* function, int* next_index);
extern void print_tree(const char* filepath, Node* tree);
