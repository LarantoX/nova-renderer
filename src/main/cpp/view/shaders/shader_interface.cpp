/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#include "shader_interface.h"
#include <algorithm>

namespace nova {
    namespace view {

        shader_tree shader_interface::gbuffers_shaders = shader_tree("gbuffers_basic", {
                shader_tree("gbuffers_skybasic"),
                shader_tree("gbuffers_textured", {
                        shader_tree("gbuffers_spidereyes"),
                        shader_tree("gbuffers_armor_glint"),
                        shader_tree("gbuffers_clouds"),
                        shader_tree("gbuffers_skytextured"),
                        shader_tree("gbuffers_textured_lit", {
                                shader_tree("gbuffers_entities"),
                                shader_tree("gbuffers_hand"),
                                shader_tree("gbuffers_weather"),
                                shader_tree("gbuffers_terrain", {
                                        shader_tree("gbuffers_damaged_block"),
                                        shader_tree("gbuffers_water"),
                                        shader_tree("gbuffers_block")
                                })
                        })
                })
        });

        void shader_interface::build_filters() {
            // First, define one filter per possible shader
            static std::unordered_map<std::string, std::function<bool(model::render_object)>> orig_filters = {
                    {"gui",                     [](const auto& geom) {return geom.type == model::geometry_type::gui;}},

                    {"shadow",                  [](const auto& geom) {return geom.type != model::geometry_type::fullscreen_quad;}},

                    {"gbuffers_basic",          [](const auto& geom) {return geom.type == model::geometry_type::selection_box;}},
                    {"gbuffers_textured",       [](const auto& geom) {return geom.type == model::geometry_type::particle;}},
                    {"gbuffers_textured_lit",   [](const auto& geom) {return geom.type == model::geometry_type::lit_particle || geom.name == "world_border";}},
                    {"gbuffers_skybasic",       [](const auto& geom) {return geom.name == "sky" || geom.name == "horizon" || geom.name == "stars" || geom.name == "void";}},
                    {"gbuffers_skytextured",    [](const auto& geom) {return geom.name == "sun" || geom.name == "moon";}},
                    {"gbuffers_clouds",         [](const auto& geom) {return geom.type == model::geometry_type::cloud;}},
                    {"gbuffers_terrain",        [](const auto& geom) {return geom.type == model::geometry_type::block && !geom.is_transparent;}},
                    {"gbuffers_damagedblock",   [](const auto& geom) {return geom.type == model::geometry_type::block && geom.damage_level > 0;}},
                    {"gbuffers_water",          [](const auto& geom) {return geom.type == model::geometry_type::block && geom.is_transparent;}},
                    {"gbuffers_block",          [](const auto& geom) {return geom.type == model::geometry_type::falling_block;}},
                    {"gbuffers_entities",       [](const auto& geom) {return geom.type == model::geometry_type::entity;}},
                    {"gbuffers_armor_glint",    [](const auto& geom) {return geom.type == model::geometry_type::glint;}},
                    {"gbuffers_spidereyes",     [](const auto& geom) {return geom.type == model::geometry_type::eyes;}},
                    {"gbuffres_hand",           [](const auto& geom) {return geom.type == model::geometry_type::hand;}},
                    {"gbuffers_weather",        [](const auto& geom) {return geom.type == model::geometry_type::weather;}},

                    {"composite",               [](const auto& geom) {return geom.type == model::geometry_type::fullscreen_quad;}},
                    {"composite1",              [](const auto& geom) {return geom.type == model::geometry_type::fullscreen_quad;}},
                    {"composite2",              [](const auto& geom) {return geom.type == model::geometry_type::fullscreen_quad;}},
                    {"composite3",              [](const auto& geom) {return geom.type == model::geometry_type::fullscreen_quad;}},
                    {"composite4",              [](const auto& geom) {return geom.type == model::geometry_type::fullscreen_quad;}},
                    {"composite5",              [](const auto& geom) {return geom.type == model::geometry_type::fullscreen_quad;}},
                    {"composite6",              [](const auto& geom) {return geom.type == model::geometry_type::fullscreen_quad;}},
                    {"composite7",              [](const auto& geom) {return geom.type == model::geometry_type::fullscreen_quad;}},
                    {"final",                   [](const auto& geom) {return geom.type == model::geometry_type::fullscreen_quad;}},
            };

            // Next, look at which shaders are loaded and figure out the actual filters to use
            gbuffers_shaders.map([&](auto& node) {
                if(loaded_shaders.find(node.shader_name) != loaded_shaders.end()) {
                    // If we've loaded this shader, let's set its filtering function as the filtering function for the shader
                    filters[node.shader_name] = node.get_filter_function();
                }
            });
        }

        shader_tree::shader_tree(std::string name) : shader_name(name) {}

        shader_tree::shader_tree(std::string name, std::initializer_list<shader_tree> children) : shader_name(name), children(children) { }

        void shader_tree::calculate_filters(
                std::unordered_map<std::string, std::function<bool(model::render_object)>> orig_filters,
                std::vector<std::string> loaded_shaders
        ) {
            filters.push_back(orig_filters[shader_name]);   // The filters for this shader always include the original filter for this shader

            for(auto& child : children) {
                child.calculate_filters(orig_filters, loaded_shaders);

                if(std::find(loaded_shaders.begin(), loaded_shaders.end(), child.shader_name) == loaded_shaders.end()) {
                    // Only add the child's filters to our own if the child's shader isn't loaded
                    filters.insert(child.filters.begin(), child.filters.end(), filters.end());
                }
            }
        }

        std::function<bool(model::render_object)> shader_tree::get_filter_function() {
            return [&](const auto& geom) {
                for(auto& filter : filters) {
                    if(filter(geom)) {
                        return true;
                    }
                }

                return false;
            };
        }
    }
}