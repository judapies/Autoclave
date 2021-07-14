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

#ifndef ADAFRUIT_THERMAL_H
#define ADAFRUIT_THERMAL_H

// *** EDIT THIS NUMBER ***  Printer firmware version is shown on test
// page (hold feed button when connecting power).  Number used here is
// integerized, e.g. 268 = 2.68 firmware.
#define PRINTER_FIRMWARE 268


// Barcode types and charsets
#if PRINTER_FIRMWARE >= 264
 #define UPC_A   65
 #define UPC_E   66
 #define EAN13   67
 #define EAN8    68
 #define CODE39  69
 #define ITF     70
 #define CODABAR 71
 #define CODE93  72
 #define CODE128 73

 #define CHARSET_USA           0
 #define CHARSET_FRANCE        1
 #define CHARSET_GERMANY       2
 #define CHARSET_UK            3
 #define CHARSET_DENMARK1      4
 #define CHARSET_SWEDEN        5
 #define CHARSET_ITALY         6
 #define CHARSET_SPAIN1        7
 #define CHARSET_JAPAN         8
 #define CHARSET_NORWAY        9
 #define CHARSET_DENMARK2     10
 #define CHARSET_SPAIN2       11
 #define CHARSET_LATINAMERICA 12
 #define CHARSET_KOREA        13
 #define CHARSET_SLOVENIA     14
 #define CHARSET_CROATIA      14
 #define CHARSET_CHINA        15

 #define CODEPAGE_CP437        0 // USA, Standard Europe
 #define CODEPAGE_KATAKANA     1
 #define CODEPAGE_CP850        2 // Multilingual
 #define CODEPAGE_CP860        3 // Portuguese
 #define CODEPAGE_CP863        4 // Canadian-French
 #define CODEPAGE_CP865        5 // Nordic
 #define CODEPAGE_WCP1251      6 // Cyrillic
 #define CODEPAGE_CP866        7 // Cyrillic #2
 #define CODEPAGE_MIK          8 // Cyrillic/Bulgarian
 #define CODEPAGE_CP755        9 // East Europe, Latvian 2
 #define CODEPAGE_IRAN        10
 #define CODEPAGE_CP862       15 // Hebrew
 #define CODEPAGE_WCP1252     16 // Latin 1
 #define CODEPAGE_WCP1253     17 // Greek
 #define CODEPAGE_CP852       18 // Latin 2
 #define CODEPAGE_CP858       19 // Multilingual Latin 1 + Euro
 #define CODEPAGE_IRAN2       20
 #define CODEPAGE_LATVIAN     21
 #define CODEPAGE_CP864       22 // Arabic
 #define CODEPAGE_ISO_8859_1  23 // West Europe
 #define CODEPAGE_CP737       24 // Greek
 #define CODEPAGE_WCP1257     25 // Baltic
 #define CODEPAGE_THAI        26
 #define CODEPAGE_CP720       27 // Arabic
 #define CODEPAGE_CP855       28
 #define CODEPAGE_CP857       29 // Turkish
 #define CODEPAGE_WCP1250     30 // Central Europe
 #define CODEPAGE_CP775       31
 #define CODEPAGE_WCP1254     32 // Turkish
 #define CODEPAGE_WCP1255     33 // Hebrew
 #define CODEPAGE_WCP1256     34 // Arabic
 #define CODEPAGE_WCP1258     35 // Vietnam
 #define CODEPAGE_ISO_8859_2  36 // Latin 2
 #define CODEPAGE_ISO_8859_3  37 // Latin 3
 #define CODEPAGE_ISO_8859_4  38 // Baltic
 #define CODEPAGE_ISO_8859_5  39 // Cyrillic
 #define CODEPAGE_ISO_8859_6  40 // Arabic
 #define CODEPAGE_ISO_8859_7  41 // Greek
 #define CODEPAGE_ISO_8859_8  42 // Hebrew
 #define CODEPAGE_ISO_8859_9  43 // Turkish
 #define CODEPAGE_ISO_8859_15 44 // Latin 3
 #define CODEPAGE_THAI2       45
 #define CODEPAGE_CP856       46
 #define CODEPAGE_CP874       47
#else
 #define UPC_A    0
 #define UPC_E    1
 #define EAN13    2
 #define EAN8     3
 #define CODE39   4
 #define I25      5
 #define CODEBAR  6
 #define CODE93   7
 #define CODE128  8
 #define CODE11   9
 #define MSI     10
#endif

//Stream *stream;
int8 printMode,prevByte,column,maxColumn,charHeight,lineSpacing,barcodeHeight,maxChunkHeight,dtrPin;        
boolean dtrEnabled;    // True if DTR pin set & printer initialized
unsigned long resumeTime,dotPrintTime,dotFeedTime;   


#endif // ADAFRUIT_THERMAL_H
