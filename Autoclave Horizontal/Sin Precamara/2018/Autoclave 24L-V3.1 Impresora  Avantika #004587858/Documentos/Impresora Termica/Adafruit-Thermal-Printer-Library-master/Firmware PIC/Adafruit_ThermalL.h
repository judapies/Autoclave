/*------------------------------------------------------------------------
  An Arduino library for the Adafruit Thermal Printer:

  https://www.adafruit.com/product/597

  These printers use TTL serial to communicate.  One pin (5V or 3.3V) is
  required to issue data to the printer.  A second pin can OPTIONALLY be
  used to poll the paper status, but not all printers support this, and
  the output on this pin is 5V which may be damaging to some MCUs.

  Adafruit invests time and resources providing this open source code.
  Please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries, with
  contributions from the open source community.  Originally based on
  Thermal library from bildr.org
  MIT license, all text above must be included in any redistribution.
  ------------------------------------------------------------------------*/

#include "Adafruit_Thermal.h"

// Though most of these printers are factory configured for 19200 baud
// operation, a few rare specimens instead work at 9600.  If so, change
// this constant.  This will NOT make printing slower!  The physical
// print and feed mechanisms are the bottleneck, not the port speed.
#define BAUDRATE  19200

// ASCII codes used by some of the printer config commands:
#define ASCII_TAB '\t' // Horizontal tab
#define ASCII_LF  '\n' // Line feed
#define ASCII_FF  '\f' // Form feed
#define ASCII_CR  '\r' // Carriage return
#define ASCII_DC2  18  // Device control 2
#define ASCII_ESC  27  // Escape
#define ASCII_FS   28  // Field separator
#define ASCII_GS   29  // Group separator

// Because there's no flow control between the printer and Arduino,
// special care must be taken to avoid overrunning the printer's buffer.
// Serial output is throttled based on serial speed as well as an estimate
// of the device's print and feed rates (relatively slow, being bound to
// moving parts and physical reality).  After an operation is issued to
// the printer (e.g. bitmap print), a timeout is set before which any
// other printer operations will be suspended.  This is generally more
// efficient than using delay() in that it allows the parent code to
// continue with other duties (e.g. receiving or decoding an image)
// while the printer physically completes the task.

// Number of microseconds to issue one byte to the printer.  11 bits
// (not 8) to accommodate idle, start and stop bits.  Idle time might
// be unnecessary, but erring on side of caution here.
#define BYTE_TIME (((11L * 1000000L) + (BAUDRATE / 2)) / BAUDRATE)

// Printer performance may vary based on the power supply voltage,
// thickness of paper, phase of the moon and other seemingly random
// variables.  This method sets the times (in microseconds) for the
// paper to advance one vertical 'dot' when printing and when feeding.
// For example, in the default initialized state, normal-sized text is
// 24 dots tall and the line spacing is 30 dots, so the time for one
// line to be issued is approximately 24 * print time + 6 * feed time.
// The default print and feed times are based on a random test unit,
// but as stated above your reality may be influenced by many factors.
// This lets you tweak the timing to avoid excessive delays and/or
// overrunning the printer buffer.
void setTimes(unsigned long p, unsigned long f) {
  dotPrintTime = p;
  dotFeedTime  = f;
}

// The next four helper methods are used when issuing configuration
// commands, printing bitmaps or barcodes, etc.  Not when printing text.

void writeBytes(int8 a) {
  delay_ms(10);
  putc(a);
  //stream->write(a);
}

void writeBytes(int8 a, int8 b) {
  delay_ms(20);
  putc(a);
  putc(b);
  //stream->write(a);
  //stream->write(b);  
}

void writeBytes(int8 a, int8 b, int8 c) {
  delay_ms(30);
  putc(a);
  putc(b);
  putc(c);
  //stream->write(a);
  //stream->write(b);
  //stream->write(c);  
}

void writeBytes(int8 a, int8 b, int8 c, int8 d) {
  delay_ms(40);
  putc(a);
  putc(b);
  putc(c);
  putc(d);
  //stream->write(a);
  //stream->write(b);
  //stream->write(c);
  //stream->write(d);  
}

// The underlying method for all high-level printing (e.g. println()).
// The inherited Print class handles the rest!
short write(int8 c) {

  if(c != 0x13) { // Strip carriage returns
    delay_ms(100);
    putc(c);
    //stream->write(c);
    unsigned long d = BYTE_TIME;
    if((c == '\n') || (column == maxColumn)) { // If newline or wrap
      d += (prevByte == '\n') ?
        ((charHeight+lineSpacing) * dotFeedTime) :             // Feed line
        ((charHeight*dotPrintTime)+(lineSpacing*dotFeedTime)); // Text line
      column = 0;
      c      = '\n'; // Treat wrap as newline on next pass
    } else {
      column++;
    }
    delay_ms(50);
    prevByte = c;
  }

  return 1;
}


// Reset printer to default state.
void reset() {
  writeBytes(ASCII_ESC, '@'); // Init command
  prevByte      = '\n';       // Treat as if prior line is blank
  column        =    0;
  maxColumn     =   32;
  charHeight    =   24;
  lineSpacing   =    6;
  barcodeHeight =   50;

#if PRINTER_FIRMWARE >= 264
  // Configure tab stops on recent printers
  writeBytes(ASCII_ESC, 'D'); // Set tab stops...
  writeBytes( 4,  8, 12, 16); // ...every 4 columns,
  writeBytes(20, 24, 28,  0); // 0 marks end-of-list.
#endif
}

void testPage() {
  writeBytes(ASCII_DC2, 'T');
  delay_us(
    dotPrintTime * 24 * 26 +      // 26 lines w/text (ea. 24 dots high)
    dotFeedTime * (6 * 26 + 30)); // 26 text lines (feed 6 dots) + blank line
}

void setBarcodeHeight(int8 val) { // Default is 50
  if(val < 1) val = 1;
  barcodeHeight = val;
  writeBytes(ASCII_GS, 'h', val);
}

// === Character commands ===

#define INVERSE_MASK       (1 << 1) // Not in 2.6.8 firmware (see inverseOn())
#define UPDOWN_MASK        (1 << 2)
#define BOLD_MASK          (1 << 3)
#define DOUBLE_HEIGHT_MASK (1 << 4)
#define DOUBLE_WIDTH_MASK  (1 << 5)
#define STRIKE_MASK        (1 << 6)

void writePrintMode() {
  writeBytes(ASCII_ESC, '!', printMode);
}

void setPrintMode(int8 mask) {
  printMode |= mask;
  writePrintMode();
  charHeight = (printMode & DOUBLE_HEIGHT_MASK) ? 48 : 24;
  maxColumn  = (printMode & DOUBLE_WIDTH_MASK ) ? 16 : 32;
}

void unsetPrintMode(int8 mask) {
  printMode &= ~mask;
  writePrintMode();
  charHeight = (printMode & DOUBLE_HEIGHT_MASK) ? 48 : 24;
  maxColumn  = (printMode & DOUBLE_WIDTH_MASK ) ? 16 : 32;
}

void normal() {
  printMode = 0;
  writePrintMode();
}

void inverseOn(){
#if PRINTER_FIRMWARE >= 268
  writeBytes(ASCII_GS, 'B', 1);
#else
  setPrintMode(INVERSE_MASK);
#endif
}

void inverseOff(){
#if PRINTER_FIRMWARE >= 268
  writeBytes(ASCII_GS, 'B', 0);
#else
  unsetPrintMode(INVERSE_MASK);
#endif
}

void upsideDownOn(){
  setPrintMode(UPDOWN_MASK);
}

void upsideDownOff(){
  unsetPrintMode(UPDOWN_MASK);
}

void doubleHeightOn(){
  setPrintMode(DOUBLE_HEIGHT_MASK);
}

void doubleHeightOff(){
  unsetPrintMode(DOUBLE_HEIGHT_MASK);
}

void doubleWidthOn(){
  setPrintMode(DOUBLE_WIDTH_MASK);
}

void doubleWidthOff(){
  unsetPrintMode(DOUBLE_WIDTH_MASK);
}

void strikeOn(){
  setPrintMode(STRIKE_MASK);
}

void strikeOff(){
  unsetPrintMode(STRIKE_MASK);
}

void boldOn(){
  setPrintMode(BOLD_MASK);
}

void boldOff(){
  unsetPrintMode(BOLD_MASK);
}

void justify(char value){
  int8 pos = 0;

  switch(toupper(value)) {
    case 'L': pos = 0; break;
    case 'C': pos = 1; break;
    case 'R': pos = 2; break;
  }

  writeBytes(ASCII_ESC, 'a', pos);
}

// Feeds by the specified number of lines
void feed(int8 x) {
#if PRINTER_FIRMWARE >= 264
  writeBytes(ASCII_ESC, 'd', x);
  delay_us(dotFeedTime * charHeight);
  prevByte = '\n';
  column   =    0;
#else
  while(x--) write('\n'); // Feed manually; old firmware feeds excess lines
#endif
}

// Feeds by the specified number of individual pixel rows
void feedRows(int8 rows) {
  writeBytes(ASCII_ESC, 'J', rows);
  delay_us(rows * dotFeedTime);
  prevByte = '\n';
  column   =    0;
}

void flush() {
  writeBytes(ASCII_FF);
}

void setSize(char value){
  int8 size;

  switch(toupper(value)) {
   default:  // Small: standard width and height
    size       = 0x00;
    charHeight = 24;
    maxColumn  = 32;
    break;
   case 'M': // Medium: double height
    size       = 0x01;
    charHeight = 48;
    maxColumn  = 32;
    break;
   case 'L': // Large: double width and height
    size       = 0x11;
    charHeight = 48;
    maxColumn  = 16;
    break;
  }

  writeBytes(ASCII_GS, '!', size);
  prevByte = '\n'; // Setting the size adds a linefeed
}

// Underlines of different weights can be produced:
// 0 - no underline
// 1 - normal underline
// 2 - thick underline
void underlineOn(int8 weight) {
  if(weight > 2) weight = 2;
  writeBytes(ASCII_ESC, '-', weight);
}

void underlineOff() {
  writeBytes(ASCII_ESC, '-', 0);
}

// Take the printer offline. Print commands sent after this will be
// ignored until 'online' is called.
void offline(){
  writeBytes(ASCII_ESC, '=', 0);
}

// Take the printer back online. Subsequent print commands will be obeyed.
void online(){
  writeBytes(ASCII_ESC, '=', 1);
}


// Wake the printer from a low-energy state.
void wake() {
  //timeoutSet(0);   // Reset timeout counter
  writeBytes(255); // Wake
#if PRINTER_FIRMWARE >= 264
  delay_us(50);
  writeBytes(ASCII_ESC, '8', 0, 0); // Sleep off (important!)
#else
  // Datasheet recommends a 50 mS delay before issuing further commands,
  // but in practice this alone isn't sufficient (e.g. text size/style
  // commands may still be misinterpreted on wake).  A slightly longer
  // delay, interspersed with NUL chars (no-ops) seems to help.
  for(uint8_t i=0; i<10; i++) {
    writeBytes(0);
    delay_us(10000);
  }
#endif
}


void begin(int8 heatTime) {

  // The printer can't start receiving data immediately upon power up --
  // it needs a moment to cold boot and initialize.  Allow at least 1/2
  // sec of uptime before printer can receive data.
  //timeoutSet(500000L);
  delay_us(500000); 
   
  wake();
  reset();

  // ESC 7 n1 n2 n3 Setting Control Parameter Command
  // n1 = "max heating dots" 0-255 -- max number of thermal print head
  //      elements that will fire simultaneously.  Units = 8 dots (minus 1).
  //      Printer default is 7 (64 dots, or 1/6 of 384-dot width), this code
  //      sets it to 11 (96 dots, or 1/4 of width).
  // n2 = "heating time" 3-255 -- duration that heating dots are fired.
  //      Units = 10 us.  Printer default is 80 (800 us), this code sets it
  //      to value passed (default 120, or 1.2 ms -- a little longer than
  //      the default because we've increased the max heating dots).
  // n3 = "heating interval" 0-255 -- recovery time between groups of
  //      heating dots on line; possibly a function of power supply.
  //      Units = 10 us.  Printer default is 2 (20 us), this code sets it
  //      to 40 (throttled back due to 2A supply).
  // More heating dots = more peak current, but faster printing speed.
  // More heating time = darker print, but slower printing speed and
  // possibly paper 'stiction'.  More heating interval = clearer print,
  // but slower printing speed.

  writeBytes(ASCII_ESC, '7');   // Esc 7 (print settings)
  writeBytes(11, heatTime, 40); // Heating dots, heat time, heat interval

  // Print density description from manual:
  // DC2 # n Set printing density
  // D4..D0 of n is used to set the printing density.  Density is
  // 50% + 5% * n(D4-D0) printing density.
  // D7..D5 of n is used to set the printing break time.  Break time
  // is n(D7-D5)*250us.
  // (Unsure of the default value for either -- not documented)

#define printDensity   10 // 100% (? can go higher, text is darker but fuzzy)
#define printBreakTime  2 // 500 uS

  writeBytes(ASCII_DC2, '#', (printBreakTime << 5) | printDensity);

  dotPrintTime   = 30000; // See comments near top of file for
  dotFeedTime    =  2100; // an explanation of these values.
  maxChunkHeight =   255;
}

// Reset text formatting parameters.
void setDefault(){
  online();
  justify('L');
  inverseOff();
  doubleHeightOff();
  boldOff();
  underlineOff();
  setBarcodeHeight(50);
  setSize('s');
}
