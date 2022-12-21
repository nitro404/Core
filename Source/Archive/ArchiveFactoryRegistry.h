#ifndef _ARCHIVE_FACTORY_REGISTRY_H_
#define _ARCHIVE_FACTORY_REGISTRY_H_

#include "Archive.h"
#include "Singleton/Singleton.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

class ArchiveFactoryRegistry final : public Singleton<ArchiveFactoryRegistry> {
public:
	ArchiveFactoryRegistry();
	virtual ~ArchiveFactoryRegistry();

	bool hasFactory(const std::string & fileExtension) const;
	bool setFactory(const std::string & fileExtension, std::function<std::unique_ptr<Archive>(std::unique_ptr<ByteBuffer> buffer)> createArchiveFunction, std::function<std::unique_ptr<Archive>(const std::string & filePath)> readArchiveFunction);
	size_t setFactory(const std::vector<std::string> & fileExtensions, std::function<std::unique_ptr<Archive>(std::unique_ptr<ByteBuffer> buffer)> createArchiveFunction, std::function<std::unique_ptr<Archive>(const std::string & filePath)> readArchiveFunction);
	void assignFactories();
	bool removeFactory(const std::string & fileExtension);
	void resetFactories();

	std::unique_ptr<Archive> createArchiveFrom(std::unique_ptr<ByteBuffer> buffer, const std::string & fileExtension);
	std::unique_ptr<Archive> readArchiveFrom(const std::string & filePath);

private:
	struct ArchiveFactoryData {
		std::function<std::unique_ptr<Archive>(std::unique_ptr<ByteBuffer> buffer)> createArchiveFunction;
		std::function<std::unique_ptr<Archive>(const std::string & filePath)> readArchiveFunction;
	};

	typedef std::map<std::string, ArchiveFactoryData> ArchiveFactoryMap;

	void assignStandardFactories();

	static std::string formatFileExtension(const std::string & fileExtension);

	ArchiveFactoryMap m_archiveFactories;

	ArchiveFactoryRegistry(const ArchiveFactoryRegistry &) = delete;
	ArchiveFactoryRegistry(ArchiveFactoryRegistry &&) noexcept = delete;
	const ArchiveFactoryRegistry & operator = (const ArchiveFactoryRegistry &) = delete;
	const ArchiveFactoryRegistry & operator = (ArchiveFactoryRegistry &&) noexcept = delete;
};

#endif // _ARCHIVE_FACTORY_REGISTRY_H_
