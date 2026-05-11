#ifndef BUZZBY_LOG_H
#define BUZZBY_LOG_H

class Logging {
  public:
    uint8_t debug=0;
    bool nextLogOnNewLine=false;

    Logging() {
      Serial.begin(115200);
    }

    void write(uint8_t level,const char value) {
      if (level>debug) {
        return;
      }
      Serial.write(value);
    }

    void print(uint8_t level,const char* format,...) {
      if (level>debug) {
        return;
      }
      char buffer[64];
      char* tbuffer=buffer;
      va_list args;
      va_list copy;
      va_start(args,format);
      va_copy(copy,args);
      int len=vsnprintf(tbuffer,sizeof(buffer),format,copy);
      va_end(copy);
      if (len<0) {
        va_end(args);
        return;
      }
      if (len>=(int)sizeof(buffer)) {
        tbuffer=(char*)malloc(len+1);
        if (tbuffer==NULL) {
          va_end(args);
          return;
        }
        len=vsnprintf(tbuffer,len+1,format,args);
      }
      va_end(args);

      if (nextLogOnNewLine) {
        nextLogOnNewLine=false;
        Serial.println();
      }
      Serial.write((uint8_t*)tbuffer,len);
      if (tbuffer!=buffer) {
        free(tbuffer);
        }
      }
    };

#endif