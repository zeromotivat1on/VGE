#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <array>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

#include "Types.h"
#include "Macros.h"
#include "Logging.h"
#include "VulkanGlobals.h"
