// The Neo Pixel library
#include <Adafruit_NeoPixel.h>
// pin of the LED
#define PIN 6
// pin for button to control the hour
#define hourPin  13
// pin for button to control the minute
#define minutePin 12
// pin for button to control the second
#define secondPin 11
// LED length 13 x 13
#define N_LEDS 169
// the Led strip object
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);
//Matrices-------------------------------------------------
// size 
const uint8_t rowSize = 13;
const uint8_t columnSize = 13;
const uint8_t layerSize = 7;
// Matrices for each hand + a layer so they rotate in a line. 
int secondMatrix[rowSize][columnSize];
double secondLayer[layerSize];
int minuteMatrix[rowSize][columnSize];
double minuteLayer[layerSize];
int hourMatrix[rowSize][columnSize];
double hourLayer[layerSize];
//---------------------------------------------------------
// color constants -------------------------
uint32_t red= strip.Color(50, 0, 0);
uint32_t green= strip.Color(0, 50, 0);
uint32_t blue= strip.Color(0, 0, 50);
uint32_t blank = strip.Color(0, 0, 0);
//------------------------------------------
// Boot Time------------
// boot time for minutes
uint8_t setMin = 22;
// boot time for hours
uint8_t setHr = 8;
//--------------------------
// Current Time------------------------------------
// current time in seconds
uint32_t timeInSeconds = 0; 
// current time in minutes
uint32_t timeinMinutes = 0;
// the last time  
uint32_t lastTime = 0;
// clock over time 
uint8_t clockOver = 60;
//-------------------------------------------------
void setup() {
  // setup for Led strip
  strip.begin();
  // setup for buttons
  pinMode(hourPin, INPUT);
  pinMode(minutePin, INPUT);
  pinMode(secondPin, INPUT); 
  //Serial.begin(9600);
  // setup for the hands on the clock 
  int i;
  // hour
  for( i = 2; i <7 ;i++){
    hourMatrix[i][6] = 1;
  }
  // minute
  for(i =0; i <7 ;i++){
    minuteMatrix[i][6] = 2;
  }
  // second 
  for(i =6; i <7 ;i++){
    secondMatrix[i][6] = 3;
  }
  secondMatrix[0][6] = 3;
  // rotate the matrices to reflect the starting time
  // rotate Hour
  // 12 hour clock 60/12 = 5 -> moving 1 hour equals 5 rotations of the matrix
  for(i = 0; i < setHr*5;i++){
    rotateMatrix(hourLayer,hourMatrix);
  }
  //rotate Minute 
  for(i = 0; i < setMin;i++){
    rotateMatrix(minuteLayer,minuteMatrix);
    // set count to the current time
    timeinMinutes ++;
  }
}
void loop() {
  // get current time in seconds
  uint32_t theTime = millis()/1000;
  // check for when current Time is different that the last known time. 
  if(theTime != lastTime){
    // rotate seconds matrix
    rotateMatrix(secondLayer,secondMatrix);
    // rotate minute matrix if time in seconds is >= 60
    if(timeInSeconds >= clockOver){
      // subtract time in seconds by 60 due to buttons 
      timeInSeconds -= clockOver;
      // add +1 to time in minutes
      timeinMinutes++;
      // rotate minute matrix
      rotateMatrix(minuteLayer,minuteMatrix);
    }
    //Serial.println(timeinMinutes);
    // add + 1 to the current time in seconds
    timeInSeconds++; 
    // rotate hour matrix if time in minutes is >= 60
    if(timeinMinutes >= clockOver){
      // subtract time in miuntes by 60 due to buttons 
      timeinMinutes -= clockOver;
      // rotate hour matrix
      for(int i = 0; i < 5; i ++)
        rotateMatrix(hourLayer,hourMatrix);
    }
    // last time is equal to the current time
    lastTime = theTime;
    // update the strip with the changes done to the matrices
    updateMatrix();
    // show the strip change
    strip.show(); 
    
  }
  // if the hour button is hit, add +1 to the hour
  if(digitalRead(hourPin) == HIGH){
    for(int i = 0; i < 5; i ++)
    rotateMatrix(hourLayer,hourMatrix);
    delay(300);
  }
  // if the minute button is hit, add +1 to the minute
  if(digitalRead(minutePin) == HIGH){
    rotateMatrix(minuteLayer,minuteMatrix);
    timeinMinutes ++;
    delay(300);
  }
  // if the second button is hit, add +1 to the second
  if(digitalRead(secondPin) == HIGH){
    rotateMatrix(secondLayer,secondMatrix);
    timeInSeconds++;
    delay(300);
  }     
}
// rotate a give matrix around the center at the same speed
void rotateMatrix(double layer[layerSize], int mat[rowSize][columnSize]){
  // around the origin starting with the outer layer to the inter layer. 
  for (int j = 6; j > 0; j--) {
    // 13*13: outer layer is 48. so layer handles the speed of each layer   
        layer[j] += j*8.0; // or j*((rowSize*4)-4)/60*10
        // if that layer cross the clockOver line, it moves
        if(layer[j] >= clockOver ) {
          // move that layer
          rotate(abs(j-6),mat);
          // substract that move but leave the extra if any
          layer[j] -=clockOver;
        }       
    }
}
// update the LED strip with the values of the matrices
void updateMatrix(){
 // transverse each element in the matrices
 for(int i = 0; i < rowSize; i++){
  // the hardware is set up in a zig-zag like:
  // row 0: ------------->
  // row 1: <-------------
  // row 2: ------------->
  // so odd rows should be transversed in the oposite direction
  // even row
    if(i%2 == 0){
      for(int j = 0; j < columnSize; j ++){
        changeColor(i,j,false);   
      }
     // odd row
    }else{
      for(int j = columnSize-1; j >=0; j --){
        changeColor(i,j,true);
      }
    }
 }
}
// change the color of a selectied element but is flip is true invert the column index
void changeColor(int i,int j,bool flip){
  // default type -> use to control the color 
  int type = 0;
  // if the invert is needed 
  if(flip){
    // change the type to the given matrix if at that location, the element != 0
    // the order of the check determines the which color will shown if there is overlap -> first on the bottom | last on the top 
    if (minuteMatrix[i][abs(j-12)]!= 0) type = minuteMatrix[i][abs(j-12)];//minute 
    if (hourMatrix[i][abs(j-12)]!= 0) type = hourMatrix[i][abs(j-12)];//hour
    if (secondMatrix[i][abs(j-12)]!= 0) type = secondMatrix[i][abs(j-12)];//second 
  // if the inver is not needed
  }else{
   // change the type to the given matrix if at that location, the element != 0
   // the order of the check determines the which color will shown if there is overlap -> first on the bottom | last on the top 
   if (minuteMatrix[i][j]!= 0) type = minuteMatrix[i][j];//minute
   if (hourMatrix[i][j]!= 0) type = hourMatrix[i][j];//hour
   if (secondMatrix[i][j]!= 0) type = secondMatrix[i][j]; //second 
  }
  //change that pixels color 
  selectColor(i,j,type);
}

// change the color of a selected pixel in the strip array given by a matrix 
// type controls the color of the pixel
void selectColor(int i,int j, int type){
  switch(type){
      case 3:
        strip.setPixelColor(rowSize*i+j,blue); 
        break;
      case 2:
        strip.setPixelColor(rowSize*i+j,green); 
        break;
      case 1: 
        strip.setPixelColor(rowSize*i+j,red); 
        break;
      case 0:
        strip.setPixelColor(rowSize*i+j, blank); 
        break;
    }
}
// rotate the matrix clockwise once 
// credit to https://www.geeksforgeeks.org/rotate-matrix-elements/ 
void rotate(int layer,int matrix[rowSize][columnSize]) {
  int row = layer, col = layer; 
  int prev, curr; 
  int m = 13-layer;
  int n = 13-layer;
  // Store the first element of next 
  // row, this element will replace  
  // first element of current row 
  prev = matrix[row + 1][col]; 
  // Move elements of first row  
  // from the remaining rows  
  for (int i = col; i < n; i++){ 
    curr = matrix[row][i]; 
    matrix[row][i] = prev; 
    prev = curr; 
  } 
    row++;    
  // Move elements of last column 
  // from the remaining columns  
  for (int i = row; i < m; i++) { 
    curr = matrix[i][n-1]; 
    matrix[i][n-1] = prev; 
    prev = curr; 
  } 
  n--; 
  // Move elements of last row  
  // from the remaining rows  
  if (row < m) { 
    for (int i = n-1; i >= col; i--) { 
      curr = matrix[m-1][i]; 
      matrix[m-1][i] = prev; 
      prev = curr; 
    } 
  } 
  m--; 
  // Move elements of first column 
  // from the remaining rows  
  if (col < n) { 
    for (int i = m-1; i >= row; i--) { 
      curr = matrix[i][col]; 
      matrix[i][col] = prev; 
      prev = curr; 
    } 
  } 
  col++;      
}