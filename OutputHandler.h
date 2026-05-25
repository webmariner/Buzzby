#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

#include <inttypes.h>
#include <stdarg.h>
#include <Arduino.h>

class OutputHandler {
public:
  void setup(Print* printer);
  template <class T, typename... Args> void print(T msg, Args... args) {
    printImpl(false, msg, args...);
  }

  template <class T, typename... Args> void println(T msg, Args... args) {
    printImpl(true, msg, args...);
  }
private:
  Print* _out;
  void fprint(const char *format, va_list args);
  void printFormat(const char format, va_list *args);
  template <class T> void printImpl(bool cr, T msg, ...) {
    va_list args;
    va_start(args, msg);
    fprint(msg, args);
    va_end(args);
    if (cr) {
        _out->print('\n');
    }
  }
};

#endif // OUTPUT_HANDLER_H