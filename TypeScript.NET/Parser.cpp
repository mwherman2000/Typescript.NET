#include "Parser.h"
#include <stack>

using namespace std;

void SyntaxTree::ReverseChildren()
{
	reverse(this->Children.begin(), this->Children.end());
	for (auto& node : this->Children)
		node->ReverseChildren();
}

void SyntaxTree::PrintTree(ostream& o, int indentation) const
{
	for (int i = 0; i < indentation; i++)
		o << '|' << string(3, ' ');
	o << this->Node.GetLexeme() << endl;
	for (auto& node : this->Children)
		node->PrintTree(o, indentation + 1);
}

ostream& operator<<(ostream& o, const SyntaxTree& tree)
{
	tree.PrintTree(o, 0);
	return o;
}


void Parser::ComputeActionTable()
{
	// see page 288 in the pdf (265 in the book)
	set<string> symbols = this->grammar.GetSymbols();

	for (int i = 0; i < this->grammar.items.size(); i++)
	{
		for (const string& symbol : symbols)
		{
			auto key = make_pair(i, symbol);
			ParsingAction action = this->ComputeAction(i, symbol);
			if (action.Type != ParsingActionType::Error)
				this->actionTable[key] = action;
		}
	}
}

ParsingAction Parser::ComputeAction(int i, const string& symbol)
{
	ParsingAction action;
	for (const Item& item : this->grammar.items[i])
	{
		vector<string>& rule = this->grammar.rules[item.ProductionHead][item.RuleIndex];
		// Is this starting state?
		if (item.ProductionHead == AUGMENTED_START() && item.Lookahead == ENDMARKER() &&
			item.RuleIndex == 0 && item.DotIndex == 0)
			this->startState = i;

		// Reduce / accept
		if (item.DotIndex == rule.size())
		{
			if (item.Lookahead == symbol)
			{
				if (item.ProductionHead == AUGMENTED_START() && symbol == ENDMARKER())
				{
					if (action.Type != ParsingActionType::Error) throw std::invalid_argument("Grammar is not LR(1)");
					action.Type = ParsingActionType::Accept;
					continue;
				}
				// Prefer shift to reduce
				if (action.Type != ParsingActionType::Shift)
				{
					action.Type = ParsingActionType::Reduce;
					action.ProductionHead = item.ProductionHead;
					action.RuleIndex = item.RuleIndex;
				}
			}
			continue;
		}

		string& dotSymbol = rule[item.DotIndex];
		// Shift
		if (dotSymbol == symbol && this->grammar.terminals.find(dotSymbol) != this->grammar.terminals.end())
		{
			//if (action.Type == ParsingActionType::Error ) throw std::invalid_argument("Grammar is not LALR(1)");
			auto key = make_pair(i, symbol);
			action.Type = ParsingActionType::Shift;
			action.StateIndex = this->grammar.gotoTable[key];
		}
	}
	return action;
}

shared_ptr<SyntaxTree> Parser::Parse(vector<Token>& text) const
{
	stack<pair<int, shared_ptr<SyntaxTree>>> states;
	states.push(make_pair(this->startState, make_shared<SyntaxTree>()));
	shared_ptr<SyntaxTree> parseTree;

	int tokenIndex = 0;
	while (true)
	{
		const Token& token = text[tokenIndex];
		auto key = make_pair(states.top().first, token.GetTerminal());

		ParsingAction action = this->actionTable.at(key);

		if (action.Type == ParsingActionType::Shift)
		{
			states.push(make_pair(action.StateIndex, make_shared<SyntaxTree>(token)));
			++tokenIndex;
		}
		else if (action.Type == ParsingActionType::Reduce)
		{
			vector<string> ruleBody = this->grammar.rules.at(action.ProductionHead)[action.RuleIndex];
			auto treeNode = make_shared<SyntaxTree>();
			treeNode->Node = action.ProductionHead;
			for (int i = 0; i < ruleBody.size(); i++)
			{
				auto subtree = states.top().second;
				treeNode->Children.push_back(subtree);
				states.pop();
			}

			auto gotoKey = make_pair(states.top().first, action.ProductionHead);
			states.push(make_pair(this->grammar.gotoTable.at(gotoKey), treeNode));
		}
		else if (action.Type == ParsingActionType::Accept)
		{
			parseTree = states.top().second;
			break;
		}
	}
	parseTree->ReverseChildren();
	return parseTree;
}


ostream& operator<<(ostream& o, const ParsingAction& action)
{
	switch (action.Type)
	{
	case ParsingActionType::Shift:
		o << "s" << action.StateIndex;
		break;
	case ParsingActionType::Reduce:
			o << "r" << action.ProductionHead << ", " << action.RuleIndex;
			break;
	case ParsingActionType::Accept:
			o << "Accept";
			break;
	default:
		break;
	}

	return o;
}

ostream& operator<<(ostream& o, const Parser& parser)
{
	for (auto& pair : parser.actionTable)
	{
		o << "<" << pair.first.first << ", " << pair.first.second << ">: " << pair.second << endl;
	}

	return o;
}