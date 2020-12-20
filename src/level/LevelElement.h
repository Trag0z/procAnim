#pragma once
#include "../Entity.h"
#include "../Collider.h"
#include "../Color.h"
#include "../Texture.h"
#include "Level.h"

class LevelEditor;

// class Wall : public Entity {
//     glm::vec2 half_ext_;
//     BoxCollider* collider;

//   public:
//     glm::vec2 half_ext() const;
//     void set_position(glm::vec2 new_pos);
//     void set_half_ext(glm::vec2 new_half_ext);

//     friend LevelEditor;
// };