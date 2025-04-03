#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#define NUM_LINES 5
#define FIRST_LINE_Y_OFFSET 10
#include <string.h>
#include <SH1106Ui.h>
#include <Arduino.h>
//#include <SoftwareSerial.h>
//#define TEXTIFY(A) #A
//#define ESCAPEQUOTE(A) TEXTIFY(A)
//#define CONSOLE(format, ...)  _console.printf(format "\n", ##__VA_ARGS__)

class Console  {
public:
    Console();
    ~Console() {};
    
    void scroll();
    void redrawConsole();
    void redrawConsoleFrame(SH1106 *display);
    void addLine(const char* line);
    void addLine(String line);
    void addLine_append(String appendStr);
    //void setOutputSerial(SoftwareSerial* serial);
    
private:
    volatile int logLinePtr;
    String consoleLog[NUM_LINES]; //6 lines with 10pt font, that is
   // SoftwareSerial *output_serial;
};
extern Console _console;
extern Console getConsole();
//#define CONSOLE(format, ...)  _console.addLinef(format, ##__VA_ARGS__)

#endif