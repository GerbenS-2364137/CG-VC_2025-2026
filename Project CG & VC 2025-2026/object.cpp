#include "object.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "includes/stb_image.h"
#include <iostream>


object::object(const std::string& modelPath, const std::string& texturePath)
    : modelPath(modelPath), texturePath(texturePath) {
}


