/*
 * John Conway's Game of Life
 * Description: Conway’s Game Of Life is a Cellular Automation Method created by John Conway. This game was created with Biology in mind but has been 
 * applied in various fields such as Graphics, terrain generation, etc.
 * 
 * ADAFRUIT 64 x 64 Color Led Matrix
 * Teensy 4.0
 * ADAFruit Teensy 4.0 Led Matric Adapter
 * 
 * Excerpts from:
 * Initially, there is a grid with some cells which may be alive or dead. Our task is to generate the next generation of cells based on the following rules: 
 
 * 1. Any live cell with fewer than two live neighbors dies as if caused by underpopulation.
 * 2. Any live cell with two or three live neighbors lives on to the next generation.
 * 3. Any live cell with more than three live neighbors dies, as if by overpopulation.
 * 4. Any dead cell with exactly three live neighbors becomes a live cell, as if by reproduction.
 *    Life code was created by adisg25 - Aditya Singh
 * 
 *  SmartMatrix Features Demo - Louis Beaudoin (Pixelmatix)
 *  This example code is released into the public domain

  (New in SmartMatrix Library 4.0) To update a SmartMatrix Library sketch to use Adafruit_GFX compatible layers:

  - Make sure you have the Adafruit_GFX Library installed in Arduino (you can use Arduino Library Manager)
  - add `#define USE_ADAFRUIT_GFX_LAYERS` at top of sketch (this is needed for any sketch to tell SmartMatrix Library that Adafruit_GFX is present, not just this sketch)
    - Add this *before* #include <SmartMatrix.h>
    - Check the documentation Wiki for more details on why you may or may not want to use these layers
*/

#define USE_ADAFRUIT_GFX_LAYERS

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 64;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 64;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_64ROW_MOD32SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);

SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

#ifdef USE_ADAFRUIT_GFX_LAYERS
  // there's not enough allocated memory to hold the long strings used by this sketch by default, this increases the memory, but it may not be large enough
  SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, 6*1024, 1, COLOR_DEPTH, kScrollingLayerOptions);
#else
  SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
#endif

SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

const int defaultBrightness = (100*255)/100;        // full (100%) brightness
//const int defaultBrightness = (15*255)/100;       // dim: 15% brightness
const int defaultScrollOffset = 6;
const rgb24 defaultBackgroundColor = {0x00, 0, 0};

// Teensy 3.0 has the LED on pin 13
const int ledPin = 13;

int row = 64;
int col = 64;
int r;
int c;
int  a[64][64];
int  b[64][64];



// Randomly sets up the grid with life
void initlife(){
  int i, j;
  
    for(i=0; i<row; i++){
        for(j=0;j<col;j++){
            a[i][j] = rand() % 2;
        }
    }
}

// transfer the new genration into the a matrix
void xferlife(){
  int i, j;
  
    for(i=0; i<row; i++){
        for(j=0;j<col;j++){
            a[i][j] = b[i][j];
        }
    }
}

// Calculate the next generation
void nextgen(){
  int i, j;
  int neighbour_live_cell;
      for(i=0; i<row; i++){
        for(j=0;j<col;j++){
            neighbour_live_cell = count_live_neighbour_cell(i,j);
            if(a[i][j]==1 && (neighbour_live_cell==2 || neighbour_live_cell==3)){
                b[i][j]=1;
            }
 
            else if(a[i][j]==0 && neighbour_live_cell==3){
                b[i][j]=1;
            }
 
            else{
                b[i][j]=0;
            }
        }
    }
    xferlife();
}


//returns the count of alive neighbours
int count_live_neighbour_cell(int r, int c){
    int i, j, count=0;
    for(i=r-1; i<=r+1; i++){
        for(j=c-1;j<=c+1;j++){
            if((i==r && j==c) || (i<0 || j<0) || (i>=row || j>=col)){
                continue;
            }
            if(a[i][j]==1){
                count++;
            }
        }
    }
    return count;
}


void setup() {
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);

  matrix.addLayer(&backgroundLayer); 
  matrix.addLayer(&scrollingLayer); 
  matrix.addLayer(&indexedLayer); 
  matrix.begin();

  matrix.setBrightness(defaultBrightness);

  scrollingLayer.setOffsetFromTop(defaultScrollOffset);

  backgroundLayer.enableColorCorrection(true);

  initlife();
}

rgb24 color;

int i;
int count=0;


void loop(){
int x0;
int y0; //, y0, x1, y1, x2, y2, radius, radius2; 
int neighbour_live_cell;
int i, j;

unsigned long currentMillis;

    backgroundLayer.fillScreen(defaultBackgroundColor);
    backgroundLayer.swapBuffers();

    color.red = 0;
    color.green = 255;
    color.blue = 0;

    for(i=0; i<row; i++){
        for(j=0;j<col;j++){
          if(a[i][j]==1)
            backgroundLayer.drawPixel(i, j, color);
        }
    }
    backgroundLayer.swapBuffers();
   nextgen();
   count++;

   if(count>500){
      initlife();
      count = 0;
   }

    char value[] = "000";
    char percent[] = "100%";
    value[0] = '0' + count / 100;
    value[1] = '0' + (count % 100) / 10;
    value[2] = '0' + count % 10;

    backgroundLayer.drawString(1, 1, {0xff, 0, 0}, value);
    backgroundLayer.swapBuffers();

    currentMillis = millis();
    while (millis() < currentMillis + 50); // ms delay between generations
}
