
#include "KeyManager.hpp"

#include <stddef.h>

KeyManager * KeyManager::instance = NULL;

KeyManager::KeyManager() {
  
  for (int i = 0; i < 256; i++) {
    ascii_keys[i] = false;
  }

  for (int i = 0; i < 1024; i++) {
    special_keys[i] = false;
  }
  
};


KeyManager * KeyManager::get() {
  
  if (instance == NULL) {
    instance = new KeyManager();
  }
  
  return instance;
};

void KeyManager::asciiKeyPressed(unsigned char key) {
  ascii_keys[key] = true;
};

void KeyManager::asciiKeyReleased(unsigned char key) {
  ascii_keys[key] = false;
};

void KeyManager::specialKeyPressed(int keycode) {
  special_keys[keycode] = true;
};

void KeyManager::specialKeyReleased(int keycode) {
  special_keys[keycode] = false;
};

bool KeyManager::isAsciiKeyPressed(unsigned char key) {
  return ascii_keys[key];
};

bool KeyManager::isSpecialKeyPressed(int keycode) {
  return special_keys[keycode];
};
