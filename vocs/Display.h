#ifndef Display_h
#define Display_h

#define LCD_COLS 16
#define LCD_ROWS 2

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class RowTextBuffer {
  private:
    const char* text;
    int buffSize;
    int cursorPosition;
  public:
    RowTextBuffer(const char* text, int buffSize);
    const char* getText() const;
    int getBuffSize() const;
    int getCursorPosition() const;
    void shiftTextLeft();
};

class ScreenBuffer {
  private:
    RowTextBuffer* rowsBuffer[LCD_ROWS];
    int _currPosition;
  public:
    ScreenBuffer();
    void push(const char* text, int buffSize);
    void put(int row, const char* text, int buffSize);
    RowTextBuffer* getRow(int row) const;
};

class Display {
  private:
    LiquidCrystal_I2C lcd;
    ScreenBuffer* screenBuffer;
  public:
    Display(uint8_t addr);
    void init(bool enableBacklight = true);
    void show();
    void push(const char* text, int buffSize);
    void put(int row, const char* text, int buffSize);
  private:
    void printLCD(int row, RowTextBuffer* rowBuffer);
};

#endif
