#include "tinc/vis/GUI.hpp"

#include "al/ui/al_ParameterGUI.hpp"

namespace tinc {

namespace vis {

void drawControl(tinc::ParameterSpaceDimension *dim) {
  ImGui::PushID(dim);
  if (dim->size() == 0) {
    al::ParameterGUI::draw(dim->getParameterMeta());
  } else if (dim->size() == 1) {
    std::string dimensionText = dim->getName() + ":" + dim->getCurrentId();
    ImGui::Text("%s", dimensionText.c_str());
  } else if (dim->size() > 1) {
    size_t multipleIds = dim->getCurrentIds().size();
    if (multipleIds == 0) {
      multipleIds = 1;
    }
    if (dim->getSpaceRepresentationType() == ParameterSpaceDimension::ID) {
      int v = (int)dim->getCurrentIndex();
      if (ImGui::SliderInt(dim->getName().c_str(), &v, 0,
                           (dim->size() / multipleIds) - 1)) {
        dim->setCurrentIndex(v * multipleIds);
      }
      ImGui::SameLine();
      if (ImGui::Button("-")) {
        dim->stepDecrease();
      }
      ImGui::SameLine();
      if (ImGui::Button("+")) {
        dim->stepIncrement();
      }
      ImGui::SameLine();
      if (multipleIds == 1) {
        ImGui::Text("%s", dim->getCurrentId().c_str());
      } else {
        ImGui::Text("%f", dim->getCurrentValue());
      }
    } else if (dim->getSpaceRepresentationType() ==
               ParameterSpaceDimension::INDEX) {
      int v = dim->getCurrentIndex();
      if (ImGui::SliderInt(dim->getName().c_str(), &v, 0,
                           (dim->size() / multipleIds) - 1)) {
        dim->setCurrentIndex(v * multipleIds);
      }
      ImGui::SameLine();
      if (ImGui::Button("-")) {
        dim->stepDecrease();
      }
      ImGui::SameLine();
      if (ImGui::Button("+")) {
        dim->stepIncrement();
      }
      ImGui::SameLine();
      if (multipleIds == 1) {
        ImGui::Text("%s", dim->getCurrentId().c_str());
      } else {
        ImGui::Text("%f", dim->getCurrentValue());
      }
    } else if (dim->getSpaceRepresentationType() ==
               ParameterSpaceDimension::VALUE) {
      float value = dim->getCurrentValue();
      size_t previousIndex = dim->getCurrentIndex();
      bool changed = false;
      if (dim->getSpaceDataType() == al::VariantType::VARIANT_FLOAT) {
        changed = ImGui::SliderFloat(dim->getName().c_str(), &value,
                                     dim->getParameter<al::Parameter>().min(),
                                     dim->getParameter<al::Parameter>().max());
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_UINT8) {
        int intValue = dim->getParameter<al::ParameterInt>().get();
        changed = ImGui::SliderInt(dim->getName().c_str(), &intValue,
                                   dim->getParameter<al::ParameterInt>().min(),
                                   dim->getParameter<al::ParameterInt>().max());
        value = intValue;
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_INT32) {
        int intValue = dim->getParameter<al::ParameterInt>().get();
        changed = ImGui::SliderInt(dim->getName().c_str(), &intValue,
                                   dim->getParameter<al::ParameterInt>().min(),
                                   dim->getParameter<al::ParameterInt>().max());
        value = intValue;
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_UINT32) {
        int intValue = dim->getParameter<al::ParameterInt>().get();
        changed = ImGui::SliderInt(dim->getName().c_str(), &intValue,
                                   dim->getParameter<al::ParameterInt>().min(),
                                   dim->getParameter<al::ParameterInt>().max());
        value = intValue;
      }
      ImGui::SameLine();
      if (ImGui::Button("-")) {
        dim->stepDecrease();
      }
      ImGui::SameLine();
      if (ImGui::Button("+")) {
        dim->stepIncrement();
      }
      // FIXME don't force change to float, use original type.
      size_t newIndex = dim->getIndexForValue(value);
      if (changed) {
        if (previousIndex != newIndex) {
          dim->setCurrentIndex(newIndex);
        }
      }
    }
  } else {
    // do nothing
  }
  ImGui::PopID();
}

void drawControls(ParameterSpace &ps) {
  ImGui::PushID(("TincParameterSpace_" + ps.getId()).c_str());
  ImGui::Text("Parameter Space: %s", ps.getId().c_str());
  for (auto *dim : ps.getDimensions()) {
    drawControl(dim);
  }
  ImGui::PopID();
}

void drawTincServerInfo(TincServer &tserv, bool debug) {
  ImGui::PushID(&tserv);

  auto serverAddr = tserv.serverAddress();
  std::string status =
      tserv.getStatus() == TincProtocol::STATUS_BUSY ? "BUSY" : "Idle";
  ImGui::Text("Tinc server at %#010lx -- %s:%i Status: %s", (uintptr_t)&tserv,
              serverAddr.first.c_str(), serverAddr.second, status.c_str());

  if (debug) {
    if (ImGui::Button("Print")) {
      tserv.print();
    }
    ImGui::SameLine();
    if (ImGui::Button("Disconnect all")) {
      tserv.disconnectAllClients();
    }
  }
  for (auto conn : tserv.connections()) {
    ImGui::Text("%s:%i", conn.first.c_str(), conn.second);
  }
  if (tserv.getRootPathMap().size() > 0) {
    if (ImGui::CollapsingHeader("RootPath Mapping")) {
      for (auto &mapEntry : tserv.getRootPathMap()) {
        ImGui::Text("Host: '%s'", mapEntry.first.c_str());
        for (auto mapping : mapEntry.second) {
          ImGui::Text("   '%s' -> '%s'", mapping.first.c_str(),
                      mapping.second.c_str());
        }
      }
    }
  }

  ImGui::PopID();
}

} // namespace vis

} // namespace tinc
