#include "al/graphics/al_Shader.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/io/al_Imgui.hpp"

#include "tinc/vis/AtomRenderer.hpp"

using namespace tinc;

using namespace al;

void InstancingMesh::init(const std::string &vert_str,
                          const std::string &frag_str, GLuint attrib_loc,
                          GLint attrib_num_elems, GLenum attrib_type) {
  shader.compile(vert_str, frag_str);
  buffer.bufferType(GL_ARRAY_BUFFER);
  buffer.usage(GL_DYNAMIC_DRAW); // assumes buffer will change every frame
  // and will be used for drawing
  buffer.create();

  auto &v = mesh.vao();
  v.bind();
  v.enableAttrib(attrib_loc);
  // for normalizing, this code only considers GL_FLOAT AND GL_UNSIGNED_BYTE,
  // (does not normalize floats and normalizes unsigned bytes)
  v.attribPointer(attrib_loc, buffer, attrib_num_elems, attrib_type,
                  (attrib_type == GL_FLOAT) ? GL_FALSE : GL_TRUE, // normalize?
                  0,                                              // stride
                  0);                                             // offset
  glVertexAttribDivisor(attrib_loc, 1); // step attribute once per instance
}

void InstancingMesh::attrib_data(size_t size, const void *data, size_t count) {
  buffer.bind();
  buffer.data(size, data);
  num_instances = count;
}

void InstancingMesh::draw() {
  mesh.vao().bind();
  if (mesh.indices().size()) {
    mesh.indexBuffer().bind();
    glDrawElementsInstanced(mesh.vaoWrapper->GLPrimMode, mesh.indices().size(),
                            GL_UNSIGNED_INT, 0, num_instances);
  } else {
    glDrawArraysInstanced(mesh.vaoWrapper->GLPrimMode, 0,
                          mesh.vertices().size(), num_instances);
  }
}

void AtomRenderer::init() {
  // Define mesh for instance drawing
  addSphere(instancingMesh.mesh, 1, 12, 6);
  instancingMesh.mesh.update();

  std::string funcMarker = "//[[FUNCTION:is_highlighted(vec3 point)]]";

  std::size_t pos = instancing_vert.find(funcMarker);
  if (pos != std::string::npos) {
    instancing_vert.replace(pos, funcMarker.length(), is_highlighted_func());
  }
  instancingMesh.init(instancing_vert, instancing_frag,
                      1,         // location
                      4,         // num elements
                      GL_FLOAT); // type

  instancing_shader.compile(instancing_vert, instancing_frag);

  auto buffer = mBuffer.getWritable();
  buffer->setType(FLOAT);
  mBuffer.doneWriting(buffer);
  //  mMarkerScale = 0.01f;
}

void AtomRenderer::setDataBoundaries(al::BoundingBoxData &b) {
  dataBoundary = b;
}

void AtomRenderer::draw(al::Graphics &g,
                        std::map<std::string, AtomData> &atomData,
                        std::vector<float> &aligned4fData) {
  g.polygonFill();

  updateShader(g);

  {
    g.pushMatrix();
    applyTransformations(g);
    size_t cumulativeCount = 0;
    for (auto data : atomData) {
      assert((int)aligned4fData.size() >=
             (cumulativeCount + data.second.counts) * 4);

      int cumulativeCount = 0;
      //    if (mShowRadius == 1.0f) {

      //      g.shader().uniform("markerScale", data.second.radius *
      //      mAtomMarkerSize);
      //      //                std::cout << data.radius << std::endl;
      //    } else {
      //      g.shader().uniform("markerScale", mAtomMarkerSize);
      //    }
      renderInstances(g, aligned4fData.data() + (cumulativeCount * 4),
                      data.second.counts);

      cumulativeCount += data.second.counts;
    }
  }
}

void AtomRenderer::setPositions(std::vector<Vec3f> &positions,
                                std::map<std::string, AtomData> &atomData) {
  NetCDFData newData;

  newData.setType(FLOAT);
  auto &dataVector = newData.getVector<float>();
  dataVector.resize(positions.size() * 4);
  auto dataIt = dataVector.begin();
  auto atomMapIt = atomData.begin();
  int counter = 0;
  al::Color color{0.5, 0.5, 0};
  float h, s, v;
  //  color = atomMapIt->second.color;
  ImGui::ColorConvertRGBtoHSV(color.r, color.g, color.b, h, s, v);
  counter = 0;
  for (auto &pos : positions) {
    *dataIt = pos.x;
    dataIt++;
    *dataIt = pos.y;
    dataIt++;
    *dataIt = pos.z;
    dataIt++;
    *dataIt = h;
    dataIt++;

    counter++;
    if (counter == atomMapIt->second.counts) {
      color = atomMapIt->second.color;
      ImGui::ColorConvertRGBtoHSV(color.r, color.g, color.b, h, s, v);
      counter = 0;
    }
  }
  mBuffer.setData(newData);
}

void AtomRenderer::setPositions(float *positions, size_t length) {
  NetCDFData newData;
  newData.setType(FLOAT);
  auto &dataVector = newData.getVector<float>();
  dataVector.resize(length);
  memcpy(dataVector.data(), positions, length * sizeof(float));
  mBuffer.setData(newData);
}

void tinc::AtomRenderer::update(double dt) {}

void AtomRenderer::onProcess(Graphics &g) {
  updateShader(g);
  auto buffer = mBuffer.get();
  auto &dataVector = buffer->getVector<float>();
  {
    g.pushMatrix();
    applyTransformations(g);
    renderInstances(g, dataVector.data(), dataVector.size() / 4);
    g.popMatrix();
  }
}

void AtomRenderer::renderInstances(Graphics &g, float *aligned4fData,
                                   size_t count) {

  instancingMesh.attrib_data(count * 4 * sizeof(float), aligned4fData, count);

  g.polygonFill();
  g.shader().uniform("is_line", 0.0f);
  instancingMesh.draw();

  g.shader().uniform("is_line", 1.0f);
  g.polygonLine();
  instancingMesh.draw();
  g.polygonFill();
}

void AtomRenderer::updateShader(Graphics &g) {
  g.shader(instancingMesh.shader);
  g.shader().uniform("layerSeparation", mLayerSeparation);
  g.shader().uniform("is_omni", 1.0f);
  g.shader().uniform("eye_sep", g.lens().eyeSep() * g.eye() / 2.0f);
  // g.shader().uniform("eye_sep", g.lens().eyeSep() * g.eye() / 2.0f);
  g.shader().uniform("foc_len", g.lens().focalLength());
  g.shader().uniform("alpha", mAlpha);
  g.shader().uniform("markerScale", mAtomMarkerSize);

  g.update();
}

void SlicingAtomRenderer::init() {
  AtomRenderer::init();

  mSliceRotationPitch.registerChangeCallback([this](float value) {
    mSlicingPlaneNormal.setNoCalls(Vec3f(sin(mSliceRotationRoll),
                                         cos(mSliceRotationRoll) * sin(value),
                                         cos(value))
                                       .normalize());
  });

  mSliceRotationRoll.registerChangeCallback([this](float value) {
    mSlicingPlaneNormal.setNoCalls(Vec3f(sin(value),
                                         cos(value) * sin(mSliceRotationPitch),
                                         cos(mSliceRotationPitch))
                                       .normalize());
  });

  mSlicingPlaneNormal.registerChangeCallback([this](Vec3f value) {
    value = value.normalized();
    float pitch = std::atan(value.y / value.z);
    float roll = std::atan(value.x / value.z);
    mSliceRotationPitch.setNoCalls(pitch);
    mSliceRotationRoll.setNoCalls(roll);
  });
}

void SlicingAtomRenderer::setDataBoundaries(BoundingBoxData &b) {
  AtomRenderer::setDataBoundaries(b);
  mSlicingPlanePoint.setHint("maxx", b.max.x);
  mSlicingPlanePoint.setHint("minx", b.min.x - (b.max.x));
  mSlicingPlanePoint.setHint("maxy", b.max.y);
  mSlicingPlanePoint.setHint("miny", b.min.y - (b.max.y));
  mSlicingPlanePoint.setHint("maxz", b.max.z);
  mSlicingPlanePoint.setHint("minz", b.min.z - (b.max.z));
  mSlicingPlaneThickness.min(0.0);
  mSlicingPlaneThickness.max(b.max.z - b.min.z);
}

void SlicingAtomRenderer::draw(Graphics &g,
                               std::map<std::string, AtomData> &atomData,
                               std::vector<float> &aligned4fData) {

  //  int cumulativeCount = 0;
  // now draw data with custom shaderg.shader(instancing_mesh0.shader);
  updateShader(g);

  size_t cumulativeCount = 0;
  for (auto data : atomData) {
    assert((int)aligned4fData.size() >=
           (cumulativeCount + data.second.counts) * 4);
    int cumulativeCount = 0;
    renderInstances(g, aligned4fData.data() + (cumulativeCount * 4),
                    data.second.counts);

    cumulativeCount += data.second.counts;
  }
}

void SlicingAtomRenderer::update(double dt) {}

void SlicingAtomRenderer::onProcess(Graphics &g) {
  updateShader(g);
  auto buffer = mBuffer.get();
  auto &dataVector = buffer->getVector<float>();
  {
    g.pushMatrix();
    applyTransformations(g);
    renderInstances(g, dataVector.data(), dataVector.size() / 4);
    g.popMatrix();
  }
}

void SlicingAtomRenderer::nextLayer() {
  mSlicingPlanePoint =
      mSlicingPlanePoint.get() +
      mSlicingPlaneNormal.get().normalized() * mSlicingPlaneThickness;
}

void SlicingAtomRenderer::previousLayer() {
  mSlicingPlanePoint =
      mSlicingPlanePoint.get() -
      mSlicingPlaneNormal.get().normalized() * mSlicingPlaneThickness;
}

void SlicingAtomRenderer::resetSlicing() {
  // Minimum value for hint allows for slice to be completely outside the
  // dataset so:
  auto minz =
      mSlicingPlanePoint.getHint("minz") + mSlicingPlanePoint.getHint("maxz");
  mSlicingPlanePoint.set({0, 0, minz});

  mSlicingPlaneThickness = mSlicingPlaneThickness.max();
  mSliceRotationRoll.set(0);
  mSliceRotationPitch.set(0);
  //      std::cout << mSlicingPlaneThickness.get() <<std::endl;
}

void SlicingAtomRenderer::updateShader(Graphics &g) {
  g.shader(instancingMesh.shader);
  g.shader().uniform("layerSeparation", mLayerSeparation);
  g.shader().uniform("is_omni", 1.0f);
  g.shader().uniform("eye_sep", g.lens().eyeSep() * g.eye() / 2.0f);
  // g.shader().uniform("eye_sep", g.lens().eyeSep() * g.eye() / 2.0f);
  g.shader().uniform("foc_len", g.lens().focalLength());
  g.shader().uniform("alpha", mAlpha);
  g.shader().uniform("markerScale", mAtomMarkerSize);

  g.shader().uniform("plane_point", mSlicingPlanePoint.get());
  g.shader().uniform("plane_normal", mSlicingPlaneNormal.get().normalized());
  g.shader().uniform("second_plane_distance", mSlicingPlaneThickness);
  g.shader().uniform("clipped_mult", mClippedMultiplier);

  g.update();
}
