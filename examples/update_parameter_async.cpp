#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Image.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace al;

// This example shows how to have a Parameter call its callbacks
// on request rather than synchronously. This is useful when
// certain processes need to be done in a particular context that
// is different to the context where the parameter has changed.
// For example, when loading an image into a texture, the image change might
// be generated in a context that is not the graphics thread, however
// the texture needs to be loaded in the OpenGL thread.
// To solve this, a ParameterString can be created to hold the filename
// but changes to it are not processed immediately but only when calling
// ParameterString::processChanges() because the parameter has been
// configured to process callbacks asynchronously using
// ParameterString::setSynchronousCallbacks(false)

// This is also a useful technique for distributed applications as by exposing
// the ParamterString through the parameterServer(), any registered listeners
// will be notified whenever a load is required.

// TODO this example could use tinc's DiskBufferImage class

struct TincApp : DistributedApp {
  Parameter param{"param", "", 0.5, "", 0.0, 1.0};

  ControlGUI gui;

  ParameterString filename{"filename"};

  Texture graphTex;

  void onInit() override {
    // Generate a new image on disk on every change of param
    param.registerChangeCallback([&](float value) {

      uint8_t intvalue = (uint8_t)(param.get() * 255);

      std::vector<uint8_t> pix;
      pix.resize(4 * 2 * 2); // 2x2 image

      for (auto &component : pix) {
        component = intvalue;
      }
      Image::saveImage("output.png", pix.data(), 2, 2);
      // Setting the value will trigger callbacks (even if the value is the
      // same)
      // But the callbacks will not be called in this context, but later in
      // onAnimate()
      filename.set("output.png");
    });

    // Set up the filename parameter
    // First disable synchronous callbacks. This means we must manually call
    // processChange(). We do that on the onAnimate() callback, that is run
    // once before one or multiple calls to onDraw()
    filename.setSynchronousCallbacks(false);
    // Then we set the callback to load data into the texture.
    // This is only safe because we are processing the changes in teh OpenGL
    // thread
    filename.registerChangeCallback([&](std::string value) {
      al::Image img(value);
      graphTex.resize(img.width(), img.height());
      graphTex.submit(img.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);

      graphTex.filter(Texture::LINEAR);
    });
  }

  void onCreate() override {
    gui.init();
    gui << param;

    graphTex.create2D(2, 2);
  }

  void onAnimate(double dt) override {
    // Process change and callbacks for the filename ParameterString
    // This will load the texture if the parameter has been flagged
    // internally as changed
    filename.processChange();
  }

  void onDraw(Graphics &g) override {
    g.clear(0);

    g.pushMatrix();
    g.translate(0, 0, -4);
    g.quad(graphTex, -1, 1, 2, -1.5);
    g.popMatrix();
    gui.draw(g);
  }
  void onExit() override { gui.cleanup(); }
};

int main() {
  TincApp().start();
  return 0;
}
