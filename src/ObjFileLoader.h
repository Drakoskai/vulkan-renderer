#ifndef OBJ_FILE_LOADER_H__
#define OBJ_FILE_LOADER_H__

#include "Mesh.h"

namespace FileFormats {
	void LoadObjFile(const std::string& file, Mesh& mesh);
}
#endif
