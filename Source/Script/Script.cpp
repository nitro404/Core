#include "Script.h"

#include "Utilities/StringUtilities.h"

#include <filesystem>
#include <fstream>

Script::Script() = default;

Script::Script(Script && s) noexcept
	: m_commands(std::move(s.m_commands)) { }

Script::Script(const Script & s) {
	for(std::vector<std::string>::const_iterator i = s.m_commands.begin(); i != s.m_commands.end(); ++i) {
		m_commands.push_back(*i);
	}
}

Script & Script::operator = (Script && s) noexcept {
	if(this != &s) {
		m_commands = std::move(s.m_commands);
	}

	return *this;
}

Script & Script::operator = (const Script & s) {
	m_commands.clear();

	for(std::vector<std::string>::const_iterator i = s.m_commands.begin(); i != s.m_commands.end(); ++i) {
		m_commands.push_back(*i);
	}

	return *this;
}

Script::~Script() = default;

size_t Script::numberOfCommands() const {
	return m_commands.size();
}

const std::string * Script::getCommand(size_t lineNumber) const {
	if(lineNumber >= m_commands.size()) {
		return nullptr;
	}

	return &m_commands[lineNumber];
}

bool Script::addCommand(const std::string & command) {
	if(command.empty()) {
		return false;
	}

	m_commands.push_back(command);

	return true;
}

bool Script::setCommand(size_t lineNumber, const std::string & command) {
	if(lineNumber >= m_commands.size() || command.empty() == 0) {
		return false;
	}

	m_commands[lineNumber] = command;

	return true;
}

bool Script::removeCommand(size_t lineNumber) {
	if(lineNumber >= m_commands.size()) {
		return false;
	}

	m_commands.erase(m_commands.begin() + lineNumber);

	return true;
}

void Script::clear() {
	m_commands.clear();
}

bool Script::readFrom(const std::string & scriptPath) {
	if(scriptPath.empty()) {
		return false;
	}

	if(!std::filesystem::is_regular_file(std::filesystem::path(scriptPath))) {
		return false;
	}

	std::ifstream fileStream(scriptPath);

	if(!fileStream.is_open()) {
		return false;
	}

	m_commands.clear();

	std::string line;

	while(std::getline(fileStream, line)) {
		line = Utilities::trimString(line);

		if(line.empty()) {
			continue;
		}

		m_commands.push_back(line);
	}

	fileStream.close();

	return true;
}

bool Script::operator == (const Script & s) const {
	if(m_commands.size() != s.m_commands.size()) {
		return false;
	}

	for(size_t i = 0; i < m_commands.size(); i++) {
		if(m_commands[i] != s.m_commands[i]) {
			return false;
		}
	}

	return true;
}

bool Script::operator != (const Script & s) const {
	return !operator == (s);
}
