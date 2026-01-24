#ifndef _SCRIPT_ARGUMENTS_H_
#define _SCRIPT_ARGUMENTS_H_

#include "Arguments/ArgumentCollection.h"

#include <map>
#include <regex>
#include <string>

class ScriptArguments final : public ArgumentCollection {
public:
	ScriptArguments();
	ScriptArguments(ScriptArguments && arguments) noexcept;
	ScriptArguments(const ScriptArguments & arguments);
	ScriptArguments & operator = (ScriptArguments && arguments) noexcept;
	ScriptArguments & operator = (const ScriptArguments & arguments);
	~ScriptArguments() override;

	std::string applyConditionals(const std::string & command) const;
	std::string applyArguments(const std::string & command) const;

	bool operator == (const ScriptArguments & arguments) const;
	bool operator != (const ScriptArguments & arguments) const;

	static const char COND_OPEN_CHAR;
	static const char COND_CLOSE_CHAR;
	static const char COND_OPTION_CHAR;
	static const char ARG_CHAR;

private:
	std::string applyConditionalsHelper(const std::string & command) const;
};

#endif // _SCRIPT_ARGUMENTS_H_
