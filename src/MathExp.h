#ifndef _MATHEXP_H_
#define _MATHEXP_H_

#include <string>
#include <stack>
#include <map>

class MathExp {
public:

	void setExpression(std::string str);
	void setVariable(std::string name, float val);
	float getVariable(std::string name);

	float eval();

private:
	//float apply(const char operatorChar, std::string operand);
	void doNextOperation();

	bool isOperator(const char c);
	bool isRParen(const char c);
	int getPrecedence(const char c);

	float strToVal(std::string str);

	std::string mExpression;

	std::stack<float> mOperands;
	std::stack<char> mOperators;
	std::map<std::string, float> mVariables;

};

#endif
