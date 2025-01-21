#pragma once
#include <vector>
#include "tokenize.h"
#include "ast.h"

struct ModuleDescriptor
{
	const char* name = nullptr;
};

struct SourceFile
{
	const char* filepath = nullptr;
	NodeAllocator* node_allocator = nullptr;
	std::vector<Token*> tokens;
};

struct ParserContext
{
	ModuleDescriptor* current_module;
	SourceFile* current_file;
	std::vector<ModuleDescriptor*> module_descriptors;
	std::vector<SourceFile*> source_files;
};

extern bool parse_file(ParserContext* ctx, const char* filepath);
extern void init_context(ParserContext* ctx);
