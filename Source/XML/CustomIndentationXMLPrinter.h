#ifndef _CUSTOM_INDENTATION_XML_PRINTER_H_
#define _CUSTOM_INDENTATION_XML_PRINTER_H_

#include <tinyxml2.h>

#include <string>

class CustomIndentationXMLPrinter final : public tinyxml2::XMLPrinter {
public:
	CustomIndentationXMLPrinter(FILE * file = nullptr, bool compact = false, int depth = 0, const std::string & indentation = "\t");
	virtual ~CustomIndentationXMLPrinter();

protected:
	virtual void PrintSpace(int depth) override;

private:
	std::string m_indentation;
};

#endif // _CUSTOM_INDENTATION_XML_PRINTER_H_
