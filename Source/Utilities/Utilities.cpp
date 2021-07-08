#include "Utilities.h"

#include "StringUtilities.h"

namespace Utilities {

#if _WIN32
	const char newLine[] = "\r\n";
#else
	const char newLine[] = "\n";
#endif // _WIN32

	bool initialRandomize = true;

	const char * monthStrings[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

	void randomizeSeed() {
		srand(static_cast<unsigned int>(time(NULL)));
	}

	void randomSeed(unsigned int seed) {
		srand(seed);
	}

	int randomInteger(int min, int max, bool randomize) {
		if(max <= min) { return min; }

		if(randomize || initialRandomize) {
			Utilities::randomizeSeed();
			initialRandomize = false;
		}

		return (rand() % (max - min + 1)) + min;
	}

	float randomFloat(float min, float max, bool randomize) {
		if(max <= min) { return min; }

		if(randomize || initialRandomize) {
			Utilities::randomizeSeed();
			initialRandomize = false;
		}

		return ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min)) + min;
	}

	std::string generateFullPath(const char * path, const char * fileName) {
		return Utilities::generateFullPath(std::string(path), std::string(fileName));
	}

	std::string generateFullPath(const std::string & path, const std::string & fileName) {
		std::string trimmedPath = Utilities::trimString(path);
		std::string trimmedFileName = trimString(fileName);
		std::string fullPath("");
		
		if(trimmedPath.empty())	 { return trimmedFileName; }
		if(trimmedFileName.empty()) { return trimmedPath; }
		
		if(compareStrings(trimmedPath, ".") != 0) {
			fullPath.append(trimmedPath);
			
			if(trimmedPath[trimmedPath.length() - 1] != '/' && trimmedPath[trimmedPath.length() - 1] != '\\') {
				fullPath.append("/");
			}
		}
		
		fullPath.append(trimmedFileName);
		
		return fullPath;
	}

	char * getFileNameNoExtension(const char * fileName) {
		if(fileName == NULL) { return NULL; }

		size_t index = std::string(fileName).find_last_of('.');

		if(index == std::string::npos) {
			return copyString(fileName);
		}

		return substring(fileName, 0, index);
	}

	char * getFileExtension(const char * fileName) {
		if(fileName == NULL) { return NULL; }

		size_t index = std::string(fileName).find_last_of('.');

		if(index == std::string::npos) {
			return nullptr;
		}

		return substring(fileName, index + 1, stringLength(fileName));
	}

	bool fileHasExtension(const char * fileName, const char * fileExtension) {
		if(fileName == NULL || fileExtension == NULL) { return false; }

		char * actualFileExtension = Utilities::getFileExtension(fileName);
		bool fileExtensionMatches = actualFileExtension != NULL && compareStringsIgnoreCase(actualFileExtension, fileExtension) == 0;
		delete [] actualFileExtension;
		return fileExtensionMatches;
	}

}
