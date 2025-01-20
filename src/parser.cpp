#include "parser.h"
#include "ast.h"

bool parse_file(ParserContext* ctx, const char* filepath)
{
	SourceFile* source_file = new SourceFile {.filepath = filepath};

	if (!tokenize_file(filepath, source_file->tokens))
	{
		printf("Failed to tokenize file %s", filepath);
		return false;
	}

	int index = 0;
	FunctionDescriptor func = {};
	if (!parse_function(source_file->tokens, 0, &func, &index))
	{
		puts("Failed to parse function");
		return false;
	}

	print_tree(nullptr, func.node);

	ctx->source_files.push_back(source_file);
	return true;
}

void init_context(ParserContext* ctx)
{
    *ctx = {};
}
