#ifndef _LIBRARY_INFORMATION_H_
#define _LIBRARY_INFORMATION_H_

#include <string>
#include <vector>

namespace LibraryInformation {

	struct VersionData {
		std::string name;
		std::string version;
		std::string extra;
	};

	const std::vector<VersionData> & getLibraryInformation();
	const std::string & getLibraryInformationString();

}

#endif // _LIBRARY_INFORMATION_H_
