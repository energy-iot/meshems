#include <console.h>
#include <display.h>
//#include <debug.h>

Console _console;

Console getConsole() {
  return _console;
}

Console::Console() {
    logLinePtr = 0;
    //output_serial = 0;
    for(int i=0;i<NUM_LINES;i++) {
        consoleLog[i] = String();
    }
}

void Console::redrawConsole() {
    if (getUI() == 0) { Serial.println("no UI"); return;}
    SH1106* display = getUI()->getDisplay();
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_10);   
    for(int i=0;i<NUM_LINES;i++) { //if no overla, start at 0
        int y = i+1 * 10;
        display->drawString(0, y, consoleLog[i]); 
        //SerialPins.println(consoleLog[i]);
    }
    getUI()->render();//display->display();
}

void Console::redrawConsoleFrame(SH1106 *display) { 
    for(int i=0;i<NUM_LINES;i++) {
        int y = i * 10;
        String tmp(this->consoleLog[i]);
        display->drawString(0, y+10, tmp); //causes unhandled exception if consoleLog[i] passed to drawString 
        //SerialPins.println(consoleLog[i]);
    }
}

void Console::scroll() {
    for(int i=0;i<NUM_LINES-1;i++) {
        consoleLog[i] = consoleLog[i+1];
    }
}

//void Console::setOutputSerial(SoftwareSerial* ser) {
//    output_serial = ser;
//}
void Console::addLine(const char* line) {
    String l(line);
    return addLine(l);
}
void Console::addLine(String line) {
    if (this->logLinePtr > NUM_LINES-1) { //scroll
        scroll();
        this->logLinePtr = NUM_LINES-1; //zero-based
    }
    this->consoleLog[this->logLinePtr] = line;
    //SerialPins.println(line);
    this->logLinePtr = this->logLinePtr + 1;
    this->redrawConsole();
}

void Console::addLine_append(String appendStr) {
    consoleLog[logLinePtr - 1].concat(appendStr);
    redrawConsole();
}