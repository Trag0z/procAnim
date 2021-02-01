#pragma once
#include "pch.h"
#include "Types.h"

struct GameConfig;
class Renderer;

class ConfigManager {
  public:
    void init(GameConfig& game_config, Renderer& renderer);
    void load_config(const char* path = nullptr);
    void save_config();

    bool display_ui_window();

  private:
    typedef std::variant<bool*, float*, s32*, glm::ivec2*, glm::vec2*>
        item_values;

    std::string save_path;

    std::map<std::string, item_values> items;

    struct ParseVisitor {
        std::stringstream& stream;
        void operator()(bool*);
        void operator()(float*);
        void operator()(s32*);
        void operator()(glm::vec2*);
        void operator()(glm::ivec2*);
    };

    struct SaveVisitor {
        static char* write_pos;
        static const char* buf_end;
        const std::string& item_name;

        void operator()(bool*);
        void operator()(float*);
        void operator()(s32*);
        void operator()(glm::vec2*);
        void operator()(glm::ivec2*);
    };

    struct UIVisitor {
        const std::string& item_name;
        void operator()(bool*);
        void operator()(float*);
        void operator()(s32*);
        void operator()(glm::vec2*);
        void operator()(glm::ivec2*);
    };
};
