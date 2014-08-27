#include "LexicalAnalyzer.h"

using namespace std;

vector<pair<regex, TokenTag>> GeneratePatternMap()
{
	string ws = R"(^\s+)",
		comment = R"(^(//.*?\n|/\*.*?\*/))",
		number = R"(^((-)?\d+(\.\d+)?))",
		boolLiteral = R"(^(true|false))",
		stringLiteral = R"(^("|').*?\1)",
		unaryOp = R"(^(\+\+|--))",
		binaryOp = R"(^(\+|\-|\*|/|%|&&|\|\||!|==|!=|<=|>=|>|<))",
		binaryAssignment = R"(^(\+=|\-=|\*=|/=|&=|\|=|<<=|>>=))",
		assignment = R"(^=)",
		id = R"(^[A-Za-z]\w*)";

	auto patternMap =
	{
		// Keywords
		make_pair(regex("^var"), TokenTag::KeywordVar),
		make_pair(regex("^if"), TokenTag::KeywordIf),
		make_pair(regex("^else if"), TokenTag::KeywordElseIf),
		make_pair(regex("^else"), TokenTag::KeywordElse),
		make_pair(regex("^for"), TokenTag::KeywordFor),
		make_pair(regex("^while"), TokenTag::KeywordWhile),
		make_pair(regex("^continue"), TokenTag::KeywordContinue),
		make_pair(regex("^switch"), TokenTag::KeywordSwitch),
		make_pair(regex("^case"), TokenTag::KeywordCase),
		make_pair(regex("^default"), TokenTag::KeywordDefault),
		make_pair(regex("^break"), TokenTag::KeywordBreak),
		make_pair(regex("^return"), TokenTag::KeywordReturn),
		make_pair(regex("^module"), TokenTag::KeywordModule),
		make_pair(regex("^class"), TokenTag::KeywordClass),
		make_pair(regex("^interface"), TokenTag::KeywordInterface),
		make_pair(regex("^private"), TokenTag::KeywordPrivate),
		make_pair(regex("^public"), TokenTag::KeywordPublic),
		make_pair(regex("^extends"), TokenTag::KeywordExtends),
		make_pair(regex("^implements"), TokenTag::KeywordImplements),

		make_pair(regex(id), TokenTag::Id),

		// Brackets
		make_pair(regex("^\\("), TokenTag::LeftParenthesis),
		make_pair(regex("^\\)"), TokenTag::RightParenthesis),
		make_pair(regex("^\\["), TokenTag::LeftSquareBracket),
		make_pair(regex("^\\]"), TokenTag::RightSquareBracket),
		make_pair(regex("^\\{"), TokenTag::LeftBrace),
		make_pair(regex("^\\}"), TokenTag::RightBrace),

		// Specials
		make_pair(regex("^\\."), TokenTag::Dot),
		make_pair(regex("^,"), TokenTag::Comma),
		make_pair(regex("^:"), TokenTag::Colon),
		make_pair(regex("^;"), TokenTag::Semicolon),

		// Operators
		make_pair(regex(unaryOp), TokenTag::UnaryOp),
		make_pair(regex(binaryAssignment), TokenTag::BinaryAssignment),
		make_pair(regex(binaryOp), TokenTag::BinaryOp),

		// Other constructs
		make_pair(regex(number), TokenTag::Number),
		make_pair(regex(boolLiteral), TokenTag::BoolLiteral),
		make_pair(regex(stringLiteral), TokenTag::StringLiteral),
		make_pair(regex(assignment), TokenTag::Assignment),
		make_pair(regex(comment), TokenTag::Comment),
		make_pair(regex(ws), TokenTag::Whitespace),


	};
	return patternMap;
}

LexicalAnalyzer::LexicalAnalyzer() : patternToTag(GeneratePatternMap())
{ }


Token LexicalAnalyzer::NextMatch(const string& text, int offset) const
{ 
	for (auto pair : this->patternToTag)
	{
		regex& pattern = pair.first;
		TokenTag tag = pair.second;

		smatch match;
		regex_search(text.begin() + offset, text.end(), match, pattern);
		if (!match.empty())
		{
			return Token(match[0], tag);
		}
	}

	return Token();
}

vector<Token> LexicalAnalyzer::Tokenize(const string& text) const
{
	return this->Tokenize(text, false);
}

vector<Token> LexicalAnalyzer::Tokenize(const string& text, bool skipWhitespaceComments) const
{
	vector<Token> tokens;
	Token t;
	int offset = 0;
	do
	{
		t = this->NextMatch(text, offset);
		if (!skipWhitespaceComments || (t.GetTag() != TokenTag::Whitespace && t.GetTag() != TokenTag::Comment))
			tokens.push_back(t);
		offset += t.GetLexeme().length();
	} while (offset != text.size());
	tokens.push_back(Token("", TokenTag::Endmarker));
	return tokens;
}