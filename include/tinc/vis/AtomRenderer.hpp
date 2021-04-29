#ifndef AL_VASPRENDER
#define AL_VASPRENDER

/*
 * Copyright 2020 AlloSphere Research Group
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 *        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * authors: Andres Cabrera, Kon Hyong Kim, Keehong Youn
 */

#include "tinc/vis/SceneObject.hpp"

#include "al/graphics/al_Graphics.hpp"

//#undef CIEXYZ
#include "al/graphics/al_VAOMesh.hpp"
#include "al/ui/al_BoundingBox.hpp"
#include "al/ui/al_Parameter.hpp"

namespace tinc {

// When setting per instance attribute position,
// remember that in VAOMesh
// position (vertices) uses location 0,
// color 1, texcoord 2, normal 3
//
// Avoid attribute indices that are going to be used.
// ex) if mesh to be instanced will have vertices and color,
//     set instancing attribute index to be 2
//
// Also note that VAOMesh's update method will disable attribute
// if there's no data for that index, so it is better to just set
// instance attribute's index to 4...
struct InstancingMesh {
  al::VAOMesh mesh;
  al::BufferObject buffer;
  al::ShaderProgram shader;
  size_t num_instances = 0;

  // attrib_loc: per instance attribute location, should be set
  //             with "layout (location=#)" in vert shader
  // attrib_num_elems: vec4? vec3?
  // attrib_type: GL_FLOAT? GL_UNSIGNED_BYTE?
  void init(const std::string &vert_str, const std::string &frag_str,
            GLuint attrib_loc, GLint attrib_num_elems, GLenum attrib_type);

  // size: size (in bytes) of whole data. if data is 10 vec4,
  //       it should be 10 * 4 * sizeof(float)
  // count: number of instances to draw with this data
  void attrib_data(size_t size, const void *data, size_t count);

  // must set shader before calling draw
  // g.shader(instancing_mesh.shader);
  // g.shader().uniform("my_uniform", my_uniform_data);
  // g.update();
  // instancing_mesh.draw(instance_count);
  void draw();
};

typedef struct {
  int counts;
  std::string species;
  float radius = 1.0;
  al::Color color;
} AtomData;

class AtomRenderer : public SceneObject {
public:
  AtomRenderer(std::string id, std::string filename = "positions.nc",
               std::string path = "", uint16_t size = 2)
      : SceneObject(id, filename, path, size),
        mDataScale("dataScale", id, {1.0, 1.0, 1.0}),
        mAtomMarkerSize("atomMarkerSize", id, 0.4, 0.0, 5.0f),
        mShowAtoms("showAtoms", id), mAlpha("alpha", id, 1.0f, 0.0f, 1.0f) {
    registerParameter(mAtomMarkerSize);
    registerParameter(mDataScale);
    registerParameter(mShowAtoms);
    registerParameter(mAlpha);
  }

  // PositionedVOice callbacks
  virtual void init() override;
  void update(double dt) override;
  void onProcess(al::Graphics &g) override;

  virtual void draw(al::Graphics &g, std::map<std::string, AtomData> &atomData,
                    std::vector<float> &aligned4fData);

  // AtomRenderer specific functions
  virtual void setDataBoundaries(al::BoundingBoxData &b);
  void setPositions(std::vector<al::Vec3f> &positions,
                    std::map<std::string, AtomData> &atomData);
  /**
   * @brief setPositions
   * @param positions pointer to x,y,z,h
   * @param length total lenght of array. atom count = lenght/4
   *
   * Each atom is represented by 4 elements in the array
   */
  void setPositions(float *positions, size_t length);

  // Parameters
  al::ParameterVec3 mDataScale;
  al::Parameter mAtomMarkerSize;
  al::ParameterChoice mShowAtoms;
  al::Parameter mAlpha;

  // Internal data
  al::BoundingBoxData dataBoundary;
  al::ShaderProgram instancing_shader;
  InstancingMesh instancingMesh;

protected:
  void renderInstances(al::Graphics &g, float *aligned4fData, size_t count);

  std::string instancing_vert =
      R"(
#version 330
// Uniforms from allolib
uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;
uniform float markerScale;
uniform vec3 dataScale;
uniform float is_line;
uniform float is_omni;
uniform float eye_sep;
uniform float foc_len;

//uniform float far_clip;
//uniform float near_clip;
uniform float clipped_mult;
uniform float alpha;
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 offset; // 4th component w is used for color
out vec4 color;

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = c.g < c.b ? vec4(c.bg, K.wz) : vec4(c.gb, K.xy);
    vec4 q = c.r < p.x ? vec4(p.xyw, c.r) : vec4(c.r, p.yzx);
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 stereo_displace(vec4 v, float e, float r) {
    vec3 OE = vec3(-v.z, 0.0, v.x); // eye direction, orthogonal to vertex vector
    OE = normalize(OE);             // but preserving +y up-vector
    OE *= e;               // set mag to eye separation
    vec3 EV = v.xyz - OE;  // eye to vertex
    float ev = length(EV); // save length
    EV /= ev;              // normalize
    // coefs for polynomial t^2 + 2bt + c = 0
    // derived from cosine law r^2 = t^2 + e^2 + 2tecos(theta)
    // where theta is angle between OE and EV
    // t is distance to sphere surface from eye
    float b = -dot(OE, EV);         // multiply -1 to dot product because
    // OE needs to be flipped in direction
    float c = e * e - r * r;
    float t = -b + sqrt(b * b - c); // quadratic formula
    v.xyz = OE + t * EV;            // direction from origin to sphere surface
    v.xyz = ev * normalize(v.xyz);  // normalize and set mag to eye-to-v distance
    return v;
}

//[[FUNCTION:is_highlighted(vec3 point)]]

const float PI_2 = 1.57079632679489661923;

void main()
{
    vec4 mesh_center = offset * vec4(dataScale, 1.0);
    mesh_center.w = 1.0;

    float colormult = 1.0;
    if (!is_highlighted(offset.xyz)) {
        if (is_line > 0.5) {
        colormult = clipped_mult * 0.3;
        } else {
        colormult = clipped_mult;
        }
    }

    float local_scale = 1.0;

    float reflectivity = (0.8 + (0.2 - pow(sin(3.14159 *(position.z + 1.0)* 0.5), 0.5)  * 0.2));

    if (is_line > 0.5) {
        local_scale = 1.03;
        color = vec4(hsv2rgb(vec3(offset.w, 1.0, 1.0)), alpha)* colormult * reflectivity;
    } else {
        color = vec4(hsv2rgb(vec3(offset.w, 1.0, 0.85)), alpha)* colormult* reflectivity;
    }

    vec4 p = vec4(local_scale * markerScale * position.xyz, 0.0) + (mesh_center);
    if (is_omni > 0.5) {
        gl_Position = al_ProjectionMatrix * stereo_displace(al_ModelViewMatrix * p, eye_sep, foc_len);
    } else {
        gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * p;
    }
}
)";

  std::string instancing_frag = R"(
#version 330
//uniform vec4 color;
in vec4 color;
layout (location = 0) out vec4 frag_out0;
void main()
{
    //GLfloat[2] range = glGet(GL_DEPTH_RANGE);
    frag_out0 = color;
}
)";

  virtual const std::string is_highlighted_func() {
    return R"(
  bool is_highlighted(vec3 point) {
      // Dummy function.
    return true;
  })";
  }

  virtual void updateShader(al::Graphics &g);

private:
};

class SlicingAtomRenderer : public AtomRenderer {
public:
  al::ParameterVec3 mSlicingPlaneCorner;
  al::Parameter mSlicingPlaneSize;
  al::ParameterVec3 mSlicingPlaneNormal;
  al::Quatf mSlicingPlaneQuat;
  al::Parameter mSlicingPlaneThickness;
  al::Parameter mSliceRotationPitch;
  al::Parameter mSliceRotationYaw;
  al::Parameter mSliceRotationRoll;
  al::Parameter mClippedMultiplier;

  SlicingAtomRenderer(std::string id, std::string filename = "positions.nc",
                      std::string path = "", uint16_t size = 2);

  // Positioned voice callbacks
  virtual void init() override;
  void update(double dt) override;
  void onProcess(al::Graphics &g) override;

  // Atom render callbacks
  virtual void setDataBoundaries(al::BoundingBoxData &b) override;

  virtual void draw(al::Graphics &g, std::map<std::string, AtomData> &atomData,
                    std::vector<float> &aligned4fData) override;

  // SlicingAtomRender specific functions
  void nextLayer();
  void previousLayer();
  void resetSlicing();

protected:
  const std::string is_highlighted_func() override {
    return R"( // Region Plane information
  uniform vec3 plane_normal = vec3(0, 0, -1);
  uniform vec3 plane_vector1;
  uniform vec3 plane_vector2;
  uniform vec3 plane_point = vec3(0.5, 0.5, 0.5);
  uniform float second_plane_distance = 3.0;
  uniform float slice_size = 1.0;

  bool is_highlighted(vec3 point) {

      vec3 difference = point - plane_point;

      float projX = dot(plane_vector1, difference);
      bool withinX = projX >= 0 &&
          projX <= slice_size && slice_size != 0.0;

      float projY = dot(plane_vector2, difference);
      bool withinY = projY >= 0 &&
          projY <= slice_size && slice_size != 0.0;

      float projZ = dot(plane_normal, difference);
      bool withinZ = projZ >= 0 &&
          projZ <= second_plane_distance && second_plane_distance != 0.0;

      return withinX && withinY && withinZ;
  })";
  }

  void updateShader(al::Graphics &g) override;
};

} // namespace tinc

#endif // AL_VASPRENDER
