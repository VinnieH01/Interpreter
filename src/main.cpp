#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>

#include "lexer.h"
#include "AST.h"
#include "Interpreter.h"

#include "Parser.h"

int main()
{
	Lexer lexer;
	Parser parser;
	Interpreter interpreter;

	while (true) 
	{
		std::string input_code;
		std::getline(std::cin, input_code);

		auto lexer_res = lexer.tokenize(input_code);
		if (lexer_res.is_error())
		{
			std::cout << "Error in Lexer at index: "  << lexer_res.get_error() << std::endl;
			continue;
		}

		const auto& tokens = *lexer_res;

		for (const auto& token : tokens)
		{
			token.print();
			std::cout << std::endl;
		}

		auto parser_res = parser.parse(tokens);

		if (parser_res.is_error())
		{
			std::cout << "Error in Parser:" << std::endl;

			for (const auto& error : parser_res.get_error())
				std::cout << error << std::endl;
			continue;
		}

		const auto& tree = *parser_res;

		if (tree.size() == 0) continue;

		for (int i = 0; i < tree.size(); ++i)
		{
			const auto& res = interpreter.interpret(*tree[i]);
			if (res.is_error()) 
				std::cout << res.get_error() << std::endl;
		}
	}
	return 0;
}