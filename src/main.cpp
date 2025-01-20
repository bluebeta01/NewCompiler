#include <stdio.h>
#include "tokenize.h"
#include "ast.h"
#include "parser.h"

int main(int argc, const char* argv[])
{
	ParserContext ctx;
	init_context(&ctx);
	if (!parse_file(&ctx, "/code/sample.txt"))
	{
		return -1;
	}
}