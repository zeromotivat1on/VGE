#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <array>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include "Macros.h"
#include "Logging.h"
#include "VulkanGlobals.h"
