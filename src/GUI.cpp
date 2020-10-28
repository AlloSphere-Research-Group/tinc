#include "tinc/GUI.hpp"

#include "al/ui/al_ParameterGUI.hpp"

namespace tinc {

namespace gui {

void drawControl(tinc::ParameterSpaceDimension *dim) {
  ImGui::PushID(dim);
  if (dim->size() == 1) {
    std::string dimensionText = dim->getName() + ":" + dim->getCurrentId();
    ImGui::Text("%s", dimensionText.c_str());
  } else if (dim->size() > 1) {
    if (dim->getSpaceType() == ParameterSpaceDimension::ID) {
      int v = dim->getCurrentIndex();
      if (ImGui::SliderInt(dim->getName().c_str(), &v, 0, dim->size() - 1)) {
        dim->setCurrentIndex(v);
      }
      ImGui::SameLine();
      ImGui::Text("%s", dim->getCurrentId().c_str());
    } else if (dim->getSpaceType() == ParameterSpaceDimension::INDEX) {
      int v = dim->getCurrentIndex();
      if (ImGui::SliderInt(dim->getName().c_str(), &v, 0, dim->size() - 1)) {
        dim->setCurrentIndex(v);
      }
      ImGui::SameLine();
      ImGui::Text("%s", dim->getCurrentId().c_str());
    } else if (dim->getSpaceType() == ParameterSpaceDimension::VALUE) {
      float value = dim->getCurrentValue();
      size_t previousIndex = dim->getCurrentIndex();
      bool changed =
          ImGui::SliderFloat(dim->getName().c_str(), &value,
                             dim->parameter().min(), dim->parameter().max());
      size_t newIndex = dim->getIndexForValue(value);
      if (changed) {
        if (dim->size() > 0) {
          if (previousIndex != newIndex) {
            dim->setCurrentIndex(newIndex);
          }
        } else {
          dim->parameter().set(value);
        }
      }
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
  for (auto dim : ps.getDimensions()) {
    drawControl(dim.get());
  }
}

} // namespace gui

} // namespace tinc
