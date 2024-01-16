#pragma once

#include <GLFW/glfw3.h>

#define DEFINE_THREE_STATE_KEY(key)                            \
  inline bool key##Down = false, key = false, key##Up = false; \
  inline bool _##key##Down = false, _##key = false, _##key##Up = false

#define RESET_THREE_STATE_KEY(pre, key) \
  pre::key##Down = false;               \
  pre::key##Up = false

#define RECORD_THREE_STATE_KEY(pre, key) \
  pre::key##Down = pre::_##key##Down;    \
  pre::key = pre::_##key;                \
  pre::key##Up = pre::_##key##Up;        \
  RESET_THREE_STATE_KEY(pre, _##key)

#define THREE_STATE_KEY_CALLBACK(glName, key) \
  if (button == glName) {                     \
    if (option == GLFW_PRESS) {               \
      _##key##Down = true;                    \
      _##key = true;                          \
    }                                         \
    if (option == GLFW_RELEASE) {             \
      _##key##Up = true;                      \
      _##key = false;                         \
    }                                         \
  }

namespace Input {
namespace Mouse {
inline double posX = 0, posY = 0;

DEFINE_THREE_STATE_KEY(left);
DEFINE_THREE_STATE_KEY(right);
DEFINE_THREE_STATE_KEY(middle);

inline void PositionCallback(GLFWwindow* window, double x, double y) {
  posX = x;
  posY = y;
}
inline void ButtonCallback(GLFWwindow* window, int button, int option,
                           int mods) {
  THREE_STATE_KEY_CALLBACK(GLFW_MOUSE_BUTTON_LEFT, left);
  THREE_STATE_KEY_CALLBACK(GLFW_MOUSE_BUTTON_RIGHT, right);
  THREE_STATE_KEY_CALLBACK(GLFW_MOUSE_BUTTON_MIDDLE, middle);
}
}  // namespace Mouse
namespace Key {
DEFINE_THREE_STATE_KEY(a);
DEFINE_THREE_STATE_KEY(b);
DEFINE_THREE_STATE_KEY(c);
DEFINE_THREE_STATE_KEY(d);
DEFINE_THREE_STATE_KEY(e);
DEFINE_THREE_STATE_KEY(f);
DEFINE_THREE_STATE_KEY(g);
DEFINE_THREE_STATE_KEY(h);
DEFINE_THREE_STATE_KEY(i);
DEFINE_THREE_STATE_KEY(j);
DEFINE_THREE_STATE_KEY(k);
DEFINE_THREE_STATE_KEY(l);
DEFINE_THREE_STATE_KEY(m);
DEFINE_THREE_STATE_KEY(n);
DEFINE_THREE_STATE_KEY(o);
DEFINE_THREE_STATE_KEY(p);
DEFINE_THREE_STATE_KEY(q);
DEFINE_THREE_STATE_KEY(r);
DEFINE_THREE_STATE_KEY(s);
DEFINE_THREE_STATE_KEY(t);
DEFINE_THREE_STATE_KEY(u);
DEFINE_THREE_STATE_KEY(v);
DEFINE_THREE_STATE_KEY(w);
DEFINE_THREE_STATE_KEY(x);
DEFINE_THREE_STATE_KEY(y);
DEFINE_THREE_STATE_KEY(z);

inline void ButtonCallback(GLFWwindow* window, int button, int scancode,
                           int option, int mods) {
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_A, a);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_B, b);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_C, c);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_D, d);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_E, e);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_F, f);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_G, g);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_H, h);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_I, i);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_J, j);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_K, k);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_L, l);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_M, m);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_N, n);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_O, o);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_P, p);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_Q, q);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_R, r);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_S, s);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_T, t);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_U, u);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_V, v);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_W, w);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_X, x);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_Y, y);
  THREE_STATE_KEY_CALLBACK(GLFW_KEY_Z, z);
}
}  // namespace Key
inline void RecordDownUpFlags() {
  RECORD_THREE_STATE_KEY(Mouse, left);
  RECORD_THREE_STATE_KEY(Mouse, right);
  RECORD_THREE_STATE_KEY(Mouse, middle);

  RECORD_THREE_STATE_KEY(Key, a);
  RECORD_THREE_STATE_KEY(Key, b);
  RECORD_THREE_STATE_KEY(Key, c);
  RECORD_THREE_STATE_KEY(Key, d);
  RECORD_THREE_STATE_KEY(Key, e);
  RECORD_THREE_STATE_KEY(Key, f);
  RECORD_THREE_STATE_KEY(Key, g);
  RECORD_THREE_STATE_KEY(Key, h);
  RECORD_THREE_STATE_KEY(Key, i);
  RECORD_THREE_STATE_KEY(Key, j);
  RECORD_THREE_STATE_KEY(Key, k);
  RECORD_THREE_STATE_KEY(Key, l);
  RECORD_THREE_STATE_KEY(Key, m);
  RECORD_THREE_STATE_KEY(Key, n);
  RECORD_THREE_STATE_KEY(Key, o);
  RECORD_THREE_STATE_KEY(Key, p);
  RECORD_THREE_STATE_KEY(Key, q);
  RECORD_THREE_STATE_KEY(Key, r);
  RECORD_THREE_STATE_KEY(Key, s);
  RECORD_THREE_STATE_KEY(Key, t);
  RECORD_THREE_STATE_KEY(Key, u);
  RECORD_THREE_STATE_KEY(Key, v);
  RECORD_THREE_STATE_KEY(Key, w);
  RECORD_THREE_STATE_KEY(Key, x);
  RECORD_THREE_STATE_KEY(Key, y);
  RECORD_THREE_STATE_KEY(Key, z);
}
inline void ResetDownUpFlags() {
  RESET_THREE_STATE_KEY(Mouse, left);
  RESET_THREE_STATE_KEY(Mouse, right);
  RESET_THREE_STATE_KEY(Mouse, middle);

  RESET_THREE_STATE_KEY(Key, a);
  RESET_THREE_STATE_KEY(Key, b);
  RESET_THREE_STATE_KEY(Key, c);
  RESET_THREE_STATE_KEY(Key, d);
  RESET_THREE_STATE_KEY(Key, e);
  RESET_THREE_STATE_KEY(Key, f);
  RESET_THREE_STATE_KEY(Key, g);
  RESET_THREE_STATE_KEY(Key, h);
  RESET_THREE_STATE_KEY(Key, i);
  RESET_THREE_STATE_KEY(Key, j);
  RESET_THREE_STATE_KEY(Key, k);
  RESET_THREE_STATE_KEY(Key, l);
  RESET_THREE_STATE_KEY(Key, m);
  RESET_THREE_STATE_KEY(Key, n);
  RESET_THREE_STATE_KEY(Key, o);
  RESET_THREE_STATE_KEY(Key, p);
  RESET_THREE_STATE_KEY(Key, q);
  RESET_THREE_STATE_KEY(Key, r);
  RESET_THREE_STATE_KEY(Key, s);
  RESET_THREE_STATE_KEY(Key, t);
  RESET_THREE_STATE_KEY(Key, u);
  RESET_THREE_STATE_KEY(Key, v);
  RESET_THREE_STATE_KEY(Key, w);
  RESET_THREE_STATE_KEY(Key, x);
  RESET_THREE_STATE_KEY(Key, y);
  RESET_THREE_STATE_KEY(Key, z);
}
}  // namespace Input

#undef DEFINE_THREE_STATE_KEY
#undef RESET_THREE_STATE_KEY
#undef RECORD_THREE_STATE_KEY
#undef THREE_STATE_KEY_CALLBACK