#ifndef _FILE_PATH_VARIABLE_EVALUATOR_H_
#define _FILE_PATH_VARIABLE_EVALUATOR_H_

#include "Singleton/Singleton.h"

#include <optional>
#include <string>

class FilePathVariableEvaluator final : public Singleton<FilePathVariableEvaluator> {
	friend class FactoryRegistry;

public:
	struct NameComparator {
	public:
		bool operator () (const std::string & nameA, const std::string & nameB) const;
	};

	using VariableMap = std::map<std::string, std::string, NameComparator>;

	std::optional<std::string> evaluateFilePath(const std::string & filePath, const VariableMap & customVariables = {}, char variableStartChar = DEFAULT_VARIABLE_START_CHAR, char variableEndChar = DEFAULT_VARIABLE_END_CHAR) const;

	static constexpr char DEFAULT_VARIABLE_START_CHAR = '<';
	static constexpr char DEFAULT_VARIABLE_END_CHAR = '>';

private:
	FilePathVariableEvaluator();

	VariableMap m_defaultVariables;

	FilePathVariableEvaluator(const FilePathVariableEvaluator &) = delete;
	const FilePathVariableEvaluator & operator = (const FilePathVariableEvaluator &) = delete;
};

#endif // _FILE_PATH_VARIABLE_EVALUATOR_H_
