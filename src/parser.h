#pragma once
#include <vector>
#include "tokenize.h"

struct ModuleDescriptor
{
	const char* name;
};

struct SourceFile
{
	const char* filepath;
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
