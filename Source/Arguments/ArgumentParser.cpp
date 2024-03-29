#include "ArgumentParser.h"

#include "Utilities/StringUtilities.h"

#include <spdlog/spdlog.h>

#include <sstream>

ArgumentParser::ArgumentParser()
	: ArgumentCollection() { }

ArgumentParser::ArgumentParser(int argc, char * argv[])
	: ArgumentCollection() {
	parseArguments(argc, argv);
}

ArgumentParser::ArgumentParser(ArgumentParser && parser) noexcept
	: ArgumentCollection(std::move(parser))
	, m_passthroughArguments(std::move(parser.m_passthroughArguments)) { }

ArgumentParser::ArgumentParser(const ArgumentParser & parser)
	: ArgumentCollection(parser)
	, m_passthroughArguments(parser.m_passthroughArguments) { }

ArgumentParser & ArgumentParser::operator = (ArgumentParser && parser) noexcept {
	if(this != &parser) {
		ArgumentCollection::operator = (std::move(parser));

		m_passthroughArguments = parser.m_passthroughArguments;
	}

	return *this;
}

ArgumentParser & ArgumentParser::operator = (const ArgumentParser & parser) {
	ArgumentCollection::operator = (parser);

	m_passthroughArguments = parser.m_passthroughArguments;

	return *this;
}

ArgumentParser::~ArgumentParser() { }

bool ArgumentParser::hasPassthroughArguments() const {
	return m_passthroughArguments.has_value() &&
		   !m_passthroughArguments->empty();
}

std::optional<std::string> ArgumentParser::getPassthroughArguments() const {
	return m_passthroughArguments;
}

void ArgumentParser::setPassthroughArguments(const std::string & passthroughArguments) {
	m_passthroughArguments = passthroughArguments;
}

void ArgumentParser::clearPassthroughArguments() {
	m_passthroughArguments.reset();
}

bool ArgumentParser::parseArguments(int argc, char * argv[]) {
	if(argc <= 0) {
		return true;
	}

	if(argv == nullptr) {
		return false;
	}

	m_arguments.clear();
	m_passthroughArguments.reset();

	std::string data;
	std::string arg;
	std::string value;
	bool passthroughArgument = false;
	std::stringstream passthroughArguments;

	for(int i = 1; i < argc; i++) {
		data = argv[i];

		if(passthroughArgument) {
			if(passthroughArguments.tellp() != 0) {
				passthroughArguments << " ";
			}

			passthroughArguments << data;

			continue;
		}

		std::optional<std::string> argumentOptional = parseArgument(data);

		if(argumentOptional.has_value()) {
			if(argumentOptional.value().empty()) {
				passthroughArgument = true;

				continue;
			}

			if(!arg.empty()) {
				m_arguments.insert(std::pair<std::string, std::string>(arg, std::string()));
			}

			arg = std::move(argumentOptional.value());
		}
		else {
			if(arg.empty()) {
				spdlog::warn("Value does not have corresponding argument: '{}'.", data);

				continue;
			}

			m_arguments.insert(std::pair<std::string, std::string>(arg, std::move(data)));
			arg.clear();
		}
	}

	if(!arg.empty()) {
		m_arguments.insert(std::pair<std::string, std::string>(arg, std::string()));
	}

	if(passthroughArgument) {
		m_passthroughArguments = passthroughArguments.str();
	}

	return true;
}

std::optional<std::string> ArgumentParser::parseArgument(const std::string & data) {
	if(data.empty() || data[0] != '-') {
		return {};
	}

	size_t offset = 1;

	if(data.size() >= 2 && data[1] == '-') {
		offset = 2;
	}

	return data.substr(offset, data.size() - offset);
}

std::string ArgumentParser::toString() const {
	std::stringstream args;

	args << ArgumentCollection::toString();

	if(m_passthroughArguments.has_value()) {
		args << " -- " << m_passthroughArguments.value();
	}

	return args.str();
}

bool ArgumentParser::operator == (const ArgumentParser & parser) const {
	return ArgumentCollection::operator == (dynamic_cast<const ArgumentCollection &>(parser));
}

bool ArgumentParser::operator != (const ArgumentParser & parser) const {
	return !operator == (parser);
}
