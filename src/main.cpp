#include <stdio.h>
#include "tokenize.h"
#include "ast.h"

int main(int argc, const char* argv[])
{
	std::vector<Token*> tokens;
	if (!tokenize_file("/code/sample.txt", tokens))
	{
		return -1;
	}

	bool result = ast_tokens(tokens);
}