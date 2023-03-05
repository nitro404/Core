#include "CustomIndentationXMLPrinter.h"

CustomIndentationXMLPrinter::CustomIndentationXMLPrinter(FILE * file, bool compact, int depth, const std::string & indentation)
	: tinyxml2::XMLPrinter(file, compact, depth)
	, m_indentation(indentation) { }

CustomIndentationXMLPrinter::~CustomIndentationXMLPrinter() { }

void CustomIndentationXMLPrinter::PrintSpace(int depth) {
	for(int i = 0; i < depth; i++) {
		Write(m_indentation.c_str(), m_indentation.length());
	}
}
