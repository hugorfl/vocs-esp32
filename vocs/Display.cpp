#include "Display.h"

RowTextBuffer::RowTextBuffer(const char* inputText, int buffSize)
  : text(inputText),
    buffSize(buffSize),
    cursorPosition(0) { }

const char* RowTextBuffer::getText() const {
  return text;
}

int RowTextBuffer::getBuffSize() const {
  return buffSize;
}

int RowTextBuffer::getCursorPosition() const {
  return cursorPosition;
}

void RowTextBuffer::shiftTextLeft() {
  if (cursorPosition < strlen(text) - LCD_COLS) {
    cursorPosition++;
  } else {
    cursorPosition = 0;
  }
}

ScreenBuffer::ScreenBuffer()
  : rowsBuffer{new RowTextBuffer("", 0), new RowTextBuffer("", 0)},
    _currPosition(0) { }

void ScreenBuffer::push(const char* text, int buffSize) {
  if (_currPosition > 0) {
    rowsBuffer[_currPosition] = rowsBuffer[_currPosition - 1];
  }
  
  rowsBuffer[_currPosition] = new RowTextBuffer(text, buffSize);

  if (_currPosition < 1) {
    _currPosition++;
  }
}

void ScreenBuffer::put(int row, const char* text, int buffSize) {
  if (row < 0 || row >= LCD_ROWS) {
    return;
  }

  rowsBuffer[row] = new RowTextBuffer(text, buffSize);
}

RowTextBuffer* ScreenBuffer::getRow(int row) const {
  if (row < 0 || row >= LCD_ROWS) {
    return NULL;
  }
  
  return rowsBuffer[row];
}

Display::Display(uint8_t addr)
  : lcd(addr, LCD_COLS, LCD_ROWS),
    screenBuffer(new ScreenBuffer()) { }

void Display::init(bool enableBacklight) {
  lcd.init();

  if (enableBacklight) {
    lcd.backlight();
  }
}

void Display::show() {
  lcd.clear();
  
  for (int i = 0; i < LCD_ROWS; i++) {
    RowTextBuffer* row = screenBuffer->getRow(i);
    
    printLCD(i, row);
    if (strlen(row->getText()) > LCD_COLS) {
      row->shiftTextLeft();
    }
  }
}

void Display::push(const char* text, int buffSize) {
  screenBuffer->push(text, buffSize);
}

void Display::put(int row, const char* text, int buffSize) {
  screenBuffer->put(row, text, buffSize);
}

void Display::clear() {
  lcd.clear();
}

void Display::printLCD(int row, RowTextBuffer* rowBuffer) {
  lcd.setCursor(0, row);
  lcd.print(rowBuffer->getText() + rowBuffer->getCursorPosition());
}
