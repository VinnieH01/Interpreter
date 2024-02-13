#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>
#include <fstream>
#include <sstream>

#include "Lexer.h"
#include "AST.h"
#include "Interpreter.h"

#include "Parser.h"

int main(int argc, char* argv[])
{
	Lexer lexer;
	Parser parser;
	Interpreter interpreter;

	std::string input_code;

	if (argc >= 2)
	{
		std::ifstream input_file(argv[1]);

		if (!input_file.is_open())
		{
			std::cout << "Cannot open file: " << argv[1] << std::endl;
			return -1;
		}

		std::ostringstream code_stream;
		code_stream << input_file.rdbuf();
		input_code = code_stream.str();
	}
	else 
	{
		std::cout << "usage: " << argv[0] << " <input file>" << std::endl;
		return -1;
	}

	auto lexer_res = lexer.tokenize(input_code);
	if (lexer_res.is_error())
	{
		Error err = lexer_res.get_error();
		std::cout << err.message << " at: "<< err.position << std::endl;
		return -1;
	}

	const auto& tokens = *lexer_res;
	auto parser_res = parser.parse(tokens);

	if (parser_res.is_error())
	{
		for (const auto& err : parser_res.get_error())
			std::cout << err.message << " at: " << err.position << std::endl;
		return -1;
	}

	const auto& tree = *parser_res;

	if (tree.size() == 0) return 0;

	for (int i = 0; i < tree.size(); ++i)
	{
		const auto& res = interpreter.interpret(*tree[i]);
		if (res.is_error()) 
			std::cout << res.get_error() << std::endl;
	}

	return 0;
}