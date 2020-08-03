#include "tinc/GUI.hpp"

#include "al/ui/al_ParameterGUI.hpp"

namespace tinc {

namespace gui {

void drawControl(std::shared_ptr<tinc::ParameterSpaceDimension> dim) {
  ImGui::PushID(dim.get());
  if (dim->size() == 1) {
    std::string dimensionText = dim->getName() + ":" + dim->getCurrentId();
    ImGui::Text("%s", dimensionText.c_str());
  } else if (dim->size() > 1) {
    if (dim->type == ParameterSpaceDimension::MAPPED) {
      int v = dim->getCurrentIndex();
      if (ImGui::SliderInt(dim->getName().c_str(), &v, 0, dim->size() - 1)) {
        dim->setCurrentIndex(v);
      }
      ImGui::SameLine();
      ImGui::Text("%s", dim->getCurrentId().c_str());
    } else if (dim->type == ParameterSpaceDimension::INDEX) {
      int v = dim->getCurrentIndex();
      if (ImGui::SliderInt(dim->getName().c_str(), &v, 0, dim->size() - 1)) {
        dim->setCurrentIndex(v);
      }
      ImGui::SameLine();
      ImGui::Text("%s", dim->getCurrentId().c_str());
    } else if (dim->type == ParameterSpaceDimension::INTERNAL) {
      al::ParameterGUI::drawParameter(&dim->parameter());
    }
    ImGui::SameLine();
    if (ImGui::Button("-")) {
      dim->stepDecrease();
    }
    ImGui::SameLine();
    if (ImGui::Button("+")) {
      dim->stepIncrement();
    }
  } else {
    // do nothing
  }
  ImGui::PopID();
}

void drawControls(ParameterSpace &ps) {
  for (auto dim : ps.dimensions) {
    drawControl(dim);
  }
}

} // namespace gui

} // namespace tinc
