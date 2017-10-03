#ifndef OBJ_FILE_LOADER_H_
#define OBJ_FILE_LOADER_H_

#include "Mesh.h"

namespace FileFormats {
	void LoadObjFile(const std::string& file, Mesh& mesh);
}
#endif
