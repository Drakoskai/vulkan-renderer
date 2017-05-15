#pragma once

#define GLFW_INCLUDE_VULKAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <windows.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <math.h>
