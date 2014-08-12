#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <ostream>

#include "Utilities.h"

struct dllspec Item
{
	int RuleIndex,
		DotIndex;
	std::string ProductionHead;
	std::string Lookahead;
};

dllspec bool operator<(const Item& first, const Item& second);

dllspec const std::string EPSILON();
dllspec const std::string ENDMARKER();
dllspec const std::string AUGMENTED_START();

class Grammar
{
private:
	std::set<std::string> terminals;
	std::set<std::string> nonterminals;
	std::string startSymbol;


	int ComputeFirstStep(std::string symbol);
	void ComputeFirst();
	int ComputeFollowStep(std::string symbol);
	void ComputeFollow();
	std::set<Item> GoTo(const std::set<Item>& setOfItems, const std::string& terminal);
	void ComputeItems();
public:

	typedef std::vector<std::string> RuleBody;
	typedef std::vector<RuleBody> RuleList;

	std::map<std::string, RuleList> rules;
	std::map<std::string, std::set<std::string>> first;
	std::map<std::string, std::set<std::string>> follow;

	dllspec Grammar(std::string start, std::map<std::string, RuleList> RuleList, bool shouldAugment);
	dllspec std::set<std::string> ComputeFirstWord(const std::vector<std::string>& word);
	dllspec friend std::ostream& operator<<(std::ostream& stream, const Grammar& grammar);
	dllspec std::set<Item>& Closure(std::set<Item>& setOfItems);
};