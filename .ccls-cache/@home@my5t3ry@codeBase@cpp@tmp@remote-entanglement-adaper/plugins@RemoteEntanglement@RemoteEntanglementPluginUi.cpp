#include "RemoteEntanglementPluginUi.hpp"
#include "Config.hpp"
#include "DistrhoUI.hpp"
#include "Fonts/chivo_bold.hpp"
#include "Margin.hpp"
#include "RemoteEntanglementPluginParams.hpp"
#include "Window.hpp"

#include <string>

#if defined(DISTRHO_OS_WINDOWS)
#include "windows.h"
#endif

enum test_enum { Item1 = 0, Item2, Item3 };

bool bvar = true;
int ivar = 12345678;
double dvar = 3.1415926;
float fvar = (float)dvar;
std::string strval = "A string";
test_enum enumval = Item2;
Color colval(0.5f, 0.5f, 0.7f, 1.f);

START_NAMESPACE_DISTRHO
using namespace nanogui;

RemoteEntanglementPluginUi::RemoteEntanglementPluginUi() : UI(611, 662) {
  const uint minWidth = 640;
  const uint minHeight = 480;

  const uint knobsLabelBoxWidth = 66;
  const uint knobsLabelBoxHeight = 21;

  loadSharedResources();

  using namespace WOLF_FONTS;
  NanoVG::FontId chivoBoldId = createFontFromMemory(
      "chivo_bold", (const uchar *)chivo_bold, chivo_bold_size, 0);
  NanoVG::FontId dejaVuSansId = findFont(NANOVG_DEJAVU_SANS_TTF);

  tryRememberSize();
  getParentWindow().saveSizeAtExit(true);

  const float width = getWidth();
  const float height = getHeight();

  const float graphBarHeight = 42;

  positionWidgets(width, height);
}

RemoteEntanglementPluginUi::~RemoteEntanglementPluginUi() {}

void RemoteEntanglementPluginUi::tryRememberSize() {
  int width, height;
  FILE *file;
  std::string tmpFileName = PLUGIN_NAME ".tmp";

#if defined(DISTRHO_OS_WINDOWS)
  CHAR tempPath[MAX_PATH + 1];

  GetTempPath(MAX_PATH + 1, tempPath);
  std::string path = std::string(tempPath) + tmpFileName;
  file = fopen(path.c_str(), "r");
#else
  file = fopen(("/tmp/" + tmpFileName).c_str(), "r");
#endif

  if (file == NULL)
    return;

  const int numberScanned = fscanf(file, "%d %d", &width, &height);

  if (numberScanned == 2 && width && height) {
    setSize(width, height);
  }

  fclose(file);
}

void RemoteEntanglementPluginUi::positionWidgets(uint width, uint height) {
  // TODO: Clean that up
}

void RemoteEntanglementPluginUi::parameterChanged(uint32_t index, float value) {

}

void RemoteEntanglementPluginUi::stateChanged(const char *key,
                                              const char *value) {

  repaint();
}

void RemoteEntanglementPluginUi::onNanoDisplay() {
  nanogui::init();

  /* scoped variables */ {
      bool use_gl_4_1 = false;// Set to true to create an OpenGL 4.1 context.
      Screen *screen = nullptr;

      if (use_gl_4_1) {
          // NanoGUI presents many options for you to utilize at your discretion.
          // See include/nanogui/screen.h for what all of these represent.
          screen = new Screen(Vector2i(500, 700), "NanoGUI test [GL 4.1]",
                              /*resizable*/true, /*fullscreen*/false, /*colorBits*/8,
                              /*alphaBits*/8, /*depthBits*/24, /*stencilBits*/8,
                              /*nSamples*/0, /*glMajor*/4, /*glMinor*/1);
      } else {
          screen = new Screen(Vector2i(500, 700), "NanoGUI test");
      }

      bool enabled = true;
      nanogui::FormHelper *gui = new FormHelper(screen);
      nanogui::Window*  window = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");
      gui->addGroup("Basic types");
      gui->addVariable("bool", bvar);
      gui->addVariable("string", strval);

      gui->addGroup("Validating fields");
      gui->addVariable("int", ivar)->setSpinnable(true);
      gui->addVariable("float", fvar);
      gui->addVariable("double", dvar)->setSpinnable(true);

      gui->addGroup("Complex types");
      gui->addVariable("Enumeration", enumval, enabled)
         ->setItems({"Item 1", "Item 2", "Item 3"});
      // gui->addVariable("Color", colval)
         // ->setFinalCallback([](const Color &c) {
         //       std::cout << "ColorPicker Final Callback: ["
         //                 << c.r() << ", "
         //                 << c.g() << ", "
         //                 << c.b() << ", "
         //                 << c.w() << "]" << std::endl;
         //   });

      gui->addGroup("Other widgets");
      gui->addButton("A button", []() { std::cout << "Button pressed." << std::endl; });

      screen->setVisible(true);
      screen->performLayout();
      window->center();

      nanogui::mainloop();
  }

  nanogui::shutdown();
}

void RemoteEntanglementPluginUi::uiIdle() {}

bool RemoteEntanglementPluginUi::onMouse(const MouseEvent &) { return false; }

void RemoteEntanglementPluginUi::uiReshape(uint width, uint height) {
  // setSize(width, height);
  positionWidgets(width, height);
}

void RemoteEntanglementPluginUi::toggleBottomBarVisibility() {

  positionWidgets(getWidth(), getHeight());
}

bool RemoteEntanglementPluginUi::onKeyboard(const KeyboardEvent &ev) {
  if (ev.press) {
    if (ev.key == 95) // F11
    {
      toggleBottomBarVisibility();
    }
  }

  return true;
}

void RemoteEntanglementPluginUi::nanoSwitchClicked(NanoSwitch *nanoSwitch) {
  const uint switchId = nanoSwitch->getId();
  const int value = nanoSwitch->isDown() ? 1 : 0;

  setParameterValue(switchId, value);
}

void RemoteEntanglementPluginUi::nanoButtonClicked(NanoButton *nanoButton) {}

void RemoteEntanglementPluginUi::nanoWheelValueChanged(NanoWheel *nanoWheel,
                                                       const int value) {
  const uint id = nanoWheel->getId();

  setParameterValue(paramOversample, value);
}

void RemoteEntanglementPluginUi::nanoKnobValueChanged(NanoKnob *nanoKnob,
                                                      const float value) {
  const uint id = nanoKnob->getId();

  setParameterValue(id, value);
}

void RemoteEntanglementPluginUi::resizeHandleMoved(int width, int height) {
  setSize(width, height);
}

UI *createUI() { return new RemoteEntanglementPluginUi(); }

END_NAMESPACE_DISTRHO
