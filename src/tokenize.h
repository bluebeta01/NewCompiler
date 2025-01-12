#pragma once
#include <vector>
#include <optional>
enum class TokenType
{
	PLUS,
	MINUS,
	CLOSE_PAREN,
	OPEN_PAREN,
	INT_LITERAL,
	IDENTIFIER,
	EQUALS,
	S16,
	STAR,
};

struct Token
{
	TokenType type;
	const char* name;
	int row;
	int column;
	long parsed_int;
};

extern bool tokenize_file(const char* filepath, std::vector<Token*>& tokens);
