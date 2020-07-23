#ifndef GUI_HPP
#define GUI_HPP

#include "tinc/ParameterSpace.hpp"
#include "tinc/ParameterSpaceDimension.hpp"

#include <memory>

namespace tinc {

// This namespace is used for fundtions that require an active ImGui Panel
// This Imgui panel can be created
namespace gui {

void drawControl(std::shared_ptr<ParameterSpaceDimension> dim);

void drawControls(ParameterSpace &ps);

} // namespace gui
} // namespace tinc

#endif // GUI_HPP
