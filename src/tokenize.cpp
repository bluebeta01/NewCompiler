#include "tokenize.h"
#include <stdio.h>
#include <string.h>
#include <iostream>

bool tokenize_file(const char* filepath, std::vector<Token*>& tokens)
{
	FILE* file = fopen(filepath, "rb");
	if (!file)
	{
		printf("Failed to open file %s", filepath);
		return false;
	}

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* buffer = new char[length + 1];
	if (fread(buffer, sizeof(char), length, file) != length)
	{
		delete[] buffer;
		printf("Failed to read entire file %s", filepath);
		return false;
	}
	buffer[length] = 0;
	fclose(file);

	char* buffer_begin = buffer;
	char* buffer_end = buffer + length;

	while (buffer_begin < buffer_end)
	{
		if (isspace(*buffer_begin))
		{
			buffer_begin++;
			continue;
		}
		if (!strncmp("void", buffer_begin, 4) && buffer_end - buffer_begin > 4 && !isalnum(*(buffer_begin + 4)))
		{
			Token* t = new Token
			{
				.type = TokenType::VOID
			};
			tokens.push_back(t);
			buffer_begin += 4;
			continue;
		}
		if (!strncmp("s16", buffer_begin, 3) && buffer_end - buffer_begin > 3 && !isalnum(*(buffer_begin + 3)))
		{
			Token* t = new Token
			{
				.type = TokenType::S16
			};
			tokens.push_back(t);
			buffer_begin += 3;
			continue;
		}
		if (!strncmp("->", buffer_begin, 2) && buffer_end - buffer_begin > 2)
		{
			Token* t = new Token
			{
				.type = TokenType::ARROW
			};
			tokens.push_back(t);
			buffer_begin += 2;
			continue;
		}
		if (*buffer_begin == ',')
		{
			Token* t = new Token
			{
				.type = TokenType::COMMA,
				.flags = TOKEN_FLAG_OPERATOR
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == ':')
		{
			Token* t = new Token
			{
				.type = TokenType::COLON,
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == ';')
		{
			Token* t = new Token
			{
				.type = TokenType::SEMICOLON,
				.flags = TOKEN_FLAG_OPERATOR
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == '+')
		{
			Token* t = new Token
			{
				.type = TokenType::PLUS,
				.flags = TOKEN_FLAG_OPERATOR
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == '-')
		{
			Token* t = new Token
			{
				.type = TokenType::MINUS,
				.flags = TOKEN_FLAG_OPERATOR
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == '*')
		{
			Token* t = new Token
			{
				.type = TokenType::STAR,
				.flags = TOKEN_FLAG_OPERATOR
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == '{')
		{
			Token* t = new Token
			{
				.type = TokenType::OPEN_BRACE
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == '}')
		{
			Token* t = new Token
			{
				.type = TokenType::CLOSE_BRACE
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == '(')
		{
			Token* t = new Token
			{
				.type = TokenType::OPEN_PAREN
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == ')')
		{
			Token* t = new Token
			{
				.type = TokenType::CLOSE_PAREN
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == '=')
		{
			Token* t = new Token
			{
				.type = TokenType::EQUALS,
				.flags = TOKEN_FLAG_OPERATOR
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (*buffer_begin == '&')
		{
			Token* t = new Token
			{
				.type = TokenType::AMP,
				.flags = TOKEN_FLAG_OPERATOR
			};
			tokens.push_back(t);
			buffer_begin += 1;
			continue;
		}
		if (isdigit(*buffer_begin))
		{
			char* next = nullptr;
			long value = strtol(buffer_begin, &next, 10);
			buffer_begin = next;

			Token* t = new Token
			{
				.type = TokenType::INT_LITERAL,
				.parsed_int = value
			};
			tokens.push_back(t);
			continue;
		}

		if (isalpha(*buffer_begin))
		{
			const char* old_begin = buffer_begin;
			int identifer_length = 1;
			buffer_begin++;
			while (buffer_begin < buffer_end)
			{
				if (*buffer_begin == '_' || isalnum(*buffer_begin))
				{
					buffer_begin++;
					continue;
				}
				break;
			}
			int length = buffer_begin - old_begin;
			char* name = new char[length + 1];
			memcpy(name, old_begin, length);
			name[length] = 0;

			Token* t = new Token
			{
				.type = TokenType::IDENTIFIER,
				.name = name
			};
			tokens.push_back(t);
			continue;
		}

		puts("Encountered unexpected token");
		delete[] buffer;
		return false;
	}

	delete[] buffer;
	return true;
}
