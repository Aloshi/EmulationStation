#include "MathExp.h"
#include <iostream>
#include <sstream>

bool MathExp::isOperator(const char c)
{
	if(c == *"+" || c == *"-" || c == *"*" || c == *"/" || c == *"(")
		return true;
	else
		return false;
}

bool MathExp::isRParen(const char c)
{
	if(c == *")")
		return true;
	else
		return false;
}

int MathExp::getPrecedence(const char c)
{
	if(c == *"(")
		return -5;

	if(c == *"+" || c == *"-")
		return 0;

	if(c == *"*" || c == *"/")
		return 1;

	std::cout << "Error - getPrecedence(): unknown character '" << c << "'\n";
	return -1;
}

float MathExp::eval()
{
	unsigned int start = 0;
	for(unsigned int i = 0; i < mExpression.length(); i++)
	{
		if(isOperator(mExpression.at(i)))
		{
			//the string from start to i is an operand, and i is an operator
			if(start != i) //if we actually do have an operand
				mOperands.push(strToVal(mExpression.substr(start, i - start)));
			else
				std::cout << "skipping operand, start == i\n";

			//now we must decide what to do with the operator
			const char op = mExpression.at(i);

			if(op != *"(")
			{
				while(mOperators.size() && getPrecedence(mOperators.top()) >= getPrecedence(op))
				{
					doNextOperation();
				}
			}

			mOperators.push(op);

			start = i + 1;
		}else{
			if(isRParen(mExpression.at(i)))
			{
				while(mOperators.top() != *"(")
				{
					doNextOperation();
				}

				mOperators.pop();
			}
		}
	}

	mOperands.push(strToVal(mExpression.substr(start, mExpression.length() - start)));


	while(mOperators.size() > 0)
		doNextOperation();


	if(mOperands.size() != 1)
	{
		std::cout << "Error - mOperands.size() = " << mOperands.size() << " at the end of evaluation!\n";
		return 0;
	}

	float final = mOperands.top();
	mOperands.pop();

	return final;
}

void MathExp::doNextOperation()
{
	//pop operator off and apply it, then push the value onto the operand stack
	const char top = mOperators.top();
	float val = 0;

	if(top == *"+")
	{
		val = mOperands.top();
		mOperands.pop();
		val += mOperands.top();
		mOperands.pop();
	}
	if(top == *"-")
	{
		val = mOperands.top();
		mOperands.pop();
		val -= mOperands.top();
		mOperands.pop();
	}
	if(top == *"*")
	{
		val = mOperands.top();
		mOperands.pop();
		val *= mOperands.top();
		mOperands.pop();
	}
	if(top == *"/")
	{
		val = mOperands.top();
		mOperands.pop();
		val /= mOperands.top();
		mOperands.pop();
	}

	mOperands.push(val);

	mOperators.pop();
}

void MathExp::setExpression(std::string str)
{
	mExpression = str;
}

void MathExp::setVariable(std::string name, float val)
{
	mVariables[name] = val;
}

float MathExp::getVariable(std::string name)
{
	return mVariables[name];
}

float MathExp::strToVal(std::string str)
{
	if(str[0] == *"$")
		return getVariable(str.substr(1, str.length() - 1)); //it's a variable!

	//it's a value!
	std::stringstream stream;
	stream << str;

	float value;
	stream >> value;

	return value;
}
