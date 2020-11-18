#include "tinc/GUI.hpp"

#include "al/ui/al_ParameterGUI.hpp"

namespace tinc {

namespace gui {

void drawControl(tinc::ParameterSpaceDimension *dim) {
  ImGui::PushID(dim);
  if (dim->size() == 0) {
    al::ParameterGUI::draw(dim->parameterMeta());
  } else if (dim->size() == 1) {
    std::string dimensionText = dim->getName() + ":" + dim->getCurrentId();
    ImGui::Text("%s", dimensionText.c_str());
  } else if (dim->size() > 1) {
    if (dim->getSpaceRepresentationType() == ParameterSpaceDimension::ID) {
      int v = dim->getCurrentIndex();
      if (ImGui::SliderInt(dim->getName().c_str(), &v, 0, dim->size() - 1)) {
        dim->setCurrentIndex(v);
      }
      ImGui::SameLine();
      ImGui::Text("%s", dim->getCurrentId().c_str());
    } else if (dim->getSpaceRepresentationType() ==
               ParameterSpaceDimension::INDEX) {
      int v = dim->getCurrentIndex();
      if (ImGui::SliderInt(dim->getName().c_str(), &v, 0, dim->size() - 1)) {
        dim->setCurrentIndex(v);
      }
      ImGui::SameLine();
      ImGui::Text("%s", dim->getCurrentId().c_str());
    } else if (dim->getSpaceRepresentationType() ==
               ParameterSpaceDimension::VALUE) {
      float value = dim->getCurrentValue();
      size_t previousIndex = dim->getCurrentIndex();
      bool changed = false;
      if (dim->getSpaceDataType() == al::DiscreteParameterValues::FLOAT) {
        changed = ImGui::SliderFloat(dim->getName().c_str(), &value,
                                     dim->parameter<al::Parameter>().min(),
                                     dim->parameter<al::Parameter>().max());
      } else if (dim->getSpaceDataType() ==
                 al::DiscreteParameterValues::UINT8) {
        int intValue = dim->parameter<al::ParameterInt>().get();
        changed = ImGui::SliderInt(dim->getName().c_str(), &intValue,
                                   dim->parameter<al::ParameterInt>().min(),
                                   dim->parameter<al::ParameterInt>().max());
        value = intValue;
      } else if (dim->getSpaceDataType() ==
                 al::DiscreteParameterValues::INT32) {
        int intValue = dim->parameter<al::ParameterInt>().get();
        changed = ImGui::SliderInt(dim->getName().c_str(), &intValue,
                                   dim->parameter<al::ParameterInt>().min(),
                                   dim->parameter<al::ParameterInt>().max());
        value = intValue;
      } else if (dim->getSpaceDataType() ==
                 al::DiscreteParameterValues::UINT32) {
        int intValue = dim->parameter<al::ParameterInt>().get();
        changed = ImGui::SliderInt(dim->getName().c_str(), &intValue,
                                   dim->parameter<al::ParameterInt>().min(),
                                   dim->parameter<al::ParameterInt>().max());
        value = intValue;
      }
      // FIXME don't force change to float, use original type.
      size_t newIndex = dim->getIndexForValue(value);
      if (changed) {
        if (previousIndex != newIndex) {
          dim->setCurrentIndex(newIndex);
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
