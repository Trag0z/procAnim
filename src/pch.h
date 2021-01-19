#pragma once

#include <array>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>
#include <map>
#include <codecvt>

#include <shobjidl.h>

// These both raise warnings on /W4
#pragma warning(push, 0)
#include <gl/glew.h>
#include <glm/ext.hpp>
#pragma warning(pop)
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include <sdl/SDL.h>
#include <sdl/SDL_image.h>
#include <sdl/SDL_mixer.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>
