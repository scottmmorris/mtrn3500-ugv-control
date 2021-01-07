
#ifndef MTRN3500_KEYMANAGER_H
#define MTRN3500_KEYMANAGER_H


class KeyManager {
  
private:
  static KeyManager * instance;

  bool ascii_keys[256];
  bool special_keys[1024];

protected:
  KeyManager();

public:

  static KeyManager * get();

  void asciiKeyPressed(unsigned char key);
  void asciiKeyReleased(unsigned char key);
  
  void specialKeyPressed(int keycode);
  void specialKeyReleased(int keycode);
  
  bool isAsciiKeyPressed(unsigned char key);
  bool isSpecialKeyPressed(int keycode);

  
};

#endif // for MTRN3500_KEYMANAGER_H
