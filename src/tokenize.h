#pragma once
#include <vector>
#include <optional>

#define TOKEN_FLAG_OPERATOR 1
typedef uint32_t TokenFlags;

enum class TokenType
{
	PLUS,
	MINUS,
	CLOSE_PAREN,
	OPEN_PAREN,
	CLOSE_BRACE,
	OPEN_BRACE,
	INT_LITERAL,
	IDENTIFIER,
	EQUALS,
	S16,
	VOID,
	STAR,
	AMP,
	SEMICOLON,
	COLON,
	COMMA,
	ARROW,
	IF,
	ELSE,
};

struct Token
{
	TokenType type;
	const char* name;
	int row;
	int column;
	long parsed_int;
	TokenFlags flags = 0;
};

extern bool tokenize_file(const char* filepath, std::vector<Token*>& tokens);
