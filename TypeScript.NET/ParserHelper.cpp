#include "ParserHelper.h"
#include "Utilities.h"

#include <algorithm>
#include <sstream>

using namespace std;

const string& EPSILON()
{
	static const std::string epsilon = "EPSILON";
	return epsilon;
}

const string& ENDMARKER()
{
	static const std::string endmarker = "ENDMARKER";
	return endmarker;
}


Grammar::Grammar(std::map<std::string, RuleList> rules)
{
	this->rules = rules;
	set<string> symbols;

	for (auto& pair : this->rules)
	{
		this->nonterminals.insert(pair.first);
		symbols.insert(pair.first);

		for (auto& rhs : pair.second)
		{
			for (auto& rhsSymbol : rhs)
			{
				symbols.insert(rhsSymbol);
			}
		}
	}
	set_difference(symbols.begin(), symbols.end(),
		this->nonterminals.begin(), this->nonterminals.end(),
		inserter(this->terminals, terminals.begin()));

	this->ComputeFirst();
}


int Grammar::ComputeFirstStep(string symbol)
{
	RuleList& symbolRules = this->rules[symbol];
	if (this->first.find(symbol) == this->first.end())
	{
		auto equalsEpsilon = [&symbol](const vector<string>& rhs) -> bool {
			return rhs.size() == 1 && rhs[0] == EPSILON();
		};
		this->first[symbol] = set<string>();
		if (any_of(symbolRules.begin(), symbolRules.end(), equalsEpsilon))
		{
			this->first[symbol].insert(EPSILON());
		}
		return this->first[symbol].size();
	}
	int currentSize = this->first[symbol].size();
	for (auto& rhs : symbolRules)
	{
		int index = 0;
		string current = rhs[index];
		this->first[symbol].insert(this->first[current].begin(), first[current].end());

		while (first[current].find(EPSILON()) != first[current].end() && index < rhs.size() - 1)
		{
			string& next = rhs[index++];
			first[symbol].insert(first[next].begin(), first[next].end());
			current = next;
		}
		if (index == rhs.size() - 1 && first[current].find(EPSILON()) != first[current].end())
			first[symbol].insert(first[current].begin(), first[current].end());
		else
		{
			// If we are here, some symbol on the rhs doesn't derive Epsilon, thus remove Epsilon if
			// it has been added by another symbol
			auto iter = first[symbol].find(EPSILON());
			if (iter != first[symbol].end())
				first[symbol].erase(iter);
		}
	}
	return first[symbol].size() - currentSize;
}

void Grammar::ComputeFirst()
{
	for (auto& terminal : this->terminals)
	{
		this->first[terminal] = { terminal };
	}

	int added = 0;
	do
	{
		added = 0;
		for (auto& nonterminal : this->nonterminals)
		{
			added += this->ComputeFirstStep(nonterminal);
		}
	} while (added != 0);
}

ostream& operator<<(ostream& stream, const Grammar& g)
{
	for (auto pair : g.first)
	{
		stream << pair.first << " -> ";
		JoinCollection(pair.second, stream) << endl;
	}
	return stream;
}


set<string> Grammar::ComputeFirstWord(const vector<string>& word)
{
	set<string> wordFirst;
	int index = 0;
	string current = word[index];
	wordFirst.insert(this->first[current].begin(), first[current].end());

	while (this->first[current].find(EPSILON()) != this->first[current].end() && index < word.size() - 1)
	{
		const string& next = word[index++];
		wordFirst.insert(this->first[next].begin(), this->first[next].end());
		current = next;
	}
	if (index == word.size() - 1 && this->first[current].find(EPSILON()) != this->first[current].end())
		wordFirst.insert(this->first[current].begin(), this->first[current].end());
	else
	{
		// If we are here, some symbol on the rhs doesn't derive Epsilon, thus remove Epsilon if
		// it has been added by another symbol
		auto iter = wordFirst.find(EPSILON());
		if (iter != wordFirst.end())
			wordFirst.erase(iter);
	}
	return wordFirst;
}