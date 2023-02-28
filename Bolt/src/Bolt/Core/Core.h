#pragma once
#include "Bolt_Types.h"

//GLM
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

//STL
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <set>

#ifdef BOLT_PLATFORM_WINDOWS
	
#else
	#error Bolt only supports Windows!
#endif
