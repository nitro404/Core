#ifndef _LIBRARY_INFORMATION_H_
#define _LIBRARY_INFORMATION_H_

#include "Singleton/Singleton.h"

#include <string>
#include <vector>

class LibraryInformation final : public Singleton<LibraryInformation> {
	friend class FactoryRegistry;

public:
	~LibraryInformation() override;

	struct VersionData {
		std::string name;
		std::string version;
		std::string extra;
	};

	size_t numberOfLibraries() const;
	bool hasLibrary(const std::string & name) const;
	size_t indexOfLibrary(const std::string & name) const;
	const VersionData * getLibraryVersionData(size_t index) const;
	const VersionData * getLibraryVersionData(const std::string & name) const;
	std::string getLibraryVersion(size_t index) const;
	std::string getLibraryVersion(const std::string & name) const;
	const std::vector<VersionData> & getLibraryInformation() const;
	std::string getLibraryInformationString() const;
	bool addLibrary(const std::string & name, const std::string & version, const std::string & extra = {});

private:
	LibraryInformation();

	std::vector<VersionData> m_libraryInformation;
};

#endif // _LIBRARY_INFORMATION_H_
