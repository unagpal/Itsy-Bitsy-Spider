//Importing necessary libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <time.h> 
#include <stdlib.h>
//Initializing variables
int difficulty = 1;
boolean LLOn = false;
boolean LROn = false;
int scr;
boolean RLOn = false;
boolean RROn = false;
boolean gameOver = false;
double totTime = 0;
int lastRefreshTime;
int lives = 1000;
int previousTime;
double lastUpdateLeft;
double lastUpdateRight;

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assigning human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define GRAY    0x8410
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//Initializing additional variables
boolean isSpikeLL = false;
boolean isSpikeLR = false;
boolean isSpikeRL = false;
boolean isSpikeRR = false;
boolean isLClear = true;
boolean isRClear = true; 
byte spikeLHeight = 0;
byte spikeRHeight = 0; 

//Creating TFTLCD screen object
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

//Boolean values for current spider locations
boolean isSpiderRL = true;
boolean isSpiderLL = true;
boolean isSpiderRR = false;
boolean isSpiderLR = false;  

//Setup function - runs once to begin the game
//Displays starting screen and enables user
//to choose difficulty level using buttons
void setup(void) {
  //Resetting the screen from previous game
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  Serial.begin(9600);
  tft.setRotation(2);
  
  //Displaying title screen until a button is pressed
  //and when the button is pressed, changing the 
  //difficulty accordingly
  testText();
  while(digitalRead(11)==HIGH && digitalRead(12)==HIGH){
    delay(50);
  }
  if (digitalRead(11)==LOW){
    difficulty=2;
  }
}

//Loop method - runs repeatedly until game is over
void loop(void) {
  //Setting initial score and time to 0
  scr = 0;
  totTime = 0;
  //Initializing the player's lives based on 
  //difficulty setting
  if (difficulty==1){
    lives = 1000;
    scr = 0;
  }
  if (difficulty==2){
    lives = 600;
    scr = 0;
  }
  tft.setRotation(0);
  int refreshInterval = 500;
  //Displaying background on TFTLCD
  setBackground(tft);
  //Loop to keep running while game is in easy
  //mode
  while(gameOver == false && difficulty==1){
    unsigned long now = millis();
    //Updating score once every 0.5 seconds
    if (now%500==00){
      updateScore(tft);
    }
    //Deducting lives every 20ms that a spider
    //is in a water stream
    if (now%20==00){
      deductLives();
    }
    //Creating new streams of water every 3
    //seconds (in random locations)
    if (now%3000==0){
      startStream();
    }
    //Refreshing screen every 0.5 seconds
    if ((now - lastRefreshTime) > refreshInterval)
    {   
        lastRefreshTime = now;
    }
    //Updating spider positions
    if (digitalRead(11)==LOW || digitalRead(12)==LOW){
      if (now - lastUpdateLeft > 1000 || now - lastUpdateRight > 1000){
        updateSpiders(now);
      }
    }
    //Updating total time elapsed
    totTime = now/10;
  }
  //Loop to keep game running while game is in
  //hard mode
  while(gameOver==false && difficulty==2){
    unsigned long now = millis();
    //Updating score every 500ms
    if (now%500==00){
      updateScore(tft);
    }
    //Deducting lives every 20ms if spider
    //is currently in a water stream
    if (now%20==00){
      deductLives();
    }
    //Creating new streams of water every
    //3 seconds (in random locations)
    if (now%3000==0){
      startStream();
    }
    //Refreshing screen every 500ms
    if ((now - lastRefreshTime) > refreshInterval)
    {   
        lastRefreshTime = now;
    }
    //Updating spider locations
    if (digitalRead(11)==LOW || digitalRead(12)==LOW){
      if (now - lastUpdateLeft > 1000 || now - lastUpdateRight > 1000){
        updateSpiders(now);
      }
    }
    //Updating total elapsed time variable
    totTime = now/10;
  }
  //Displaying game over screen after
  //user has lost
  gameText();
  //Continuing to display game over screen
  //until a button is pressed, indicating 
  //that the player wishes to play again
  while (digitalRead(11)==HIGH && digitalRead(12)==HIGH){
    delay(50);
  }
  //Setting game over variable to false
  //after player chooses to play again
  gameOver = false;
}

//Method to create streams of 
//water every 3s
void startStream(){
  if (LLOn == true){
    dryLL();
    LLOn = false;
  }
  if (LROn ==true){
    dryLR();
    LROn = false;
  }
  if (RLOn == true){
    dryRL();
    RLOn = false;
  }
  if (RROn == true){
    dryRR();
    RROn = false;
  }
  delay(50);
  int selection = random(1,5);
  if (selection==1){
    wetLL();
    LLOn = true;
  }
  else if(selection==2){
    wetLR();
    LROn = true;
  }
  else if (selection==3){
    wetRL();
    RLOn = true;
  }
  else{
    wetRR();
    RROn = true;
  }
}

//Method to check if spider is 
//currently in a stream of water;
//if so, lives will be deducted.
boolean checkSame(){
  boolean isSame = false;
  if (LLOn==true && isSpiderLL==true){
    isSame = true;
  }
  if (RLOn==true && isSpiderRL==true){
    isSame = true;
  }
  if (RROn==true && isSpiderRR==true){
    isSame = true;
  }
  if (LROn==true && isSpiderLR==true){
    isSame = true;
  }
  return (isSame);
}

//Method to update spider locations if
//buttons are pressed
void updateSpiders (unsigned long now){
  if(digitalRead(11) == LOW){
      switchSpiderRight();
      lastUpdateRight = now;
  }
  if(digitalRead(12) == LOW){
      switchSpiderLeft(); 
      lastUpdateLeft = now;
  }
}

//Method to deduct lives if spider is
//in a stream of water
void deductLives(){
  if(checkSame()==true){
    lives--;
  }
  if (lives<0){
    gameOver = true;
  }
}

//Method to update score as time passes
void updateScore(Adafruit_TFTLCD tft){
  tft.setRotation(2);
  scr = totTime/100 - 5;
  int tempTot = totTime;
  if (tempTot%2==0){
    tft.fillRect(250, 15, 50, 50, GRAY);
    tft.setCursor(260,20);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.println(scr);
  }

  tft.setRotation(0);
}

//Method to set the background colors
//properly
void setBackground(Adafruit_TFTLCD tft){
  unsigned long start; 
  start = micros(); 
  tft.fillScreen(MAGENTA);

  tft.fillRect(0, 0, 80, 480, GRAY);
  tft.drawRect(0, 0, 80, 480, GRAY);

  tft.fillRect(80, 0, 160, 480, MAGENTA);
  tft.drawRect(80, 0, 160, 480, MAGENTA);

  tft.fillRect(240, 0, 80, 480, GRAY);
  tft.drawRect(240, 0, 80, 480, GRAY);

  tft.drawLine(120, 0, 120, 480, BLACK);
  tft.drawRect(160, 0, 160, 480, MAGENTA);
  tft.drawLine(200, 0, 200, 480, BLACK);

  drawSpiderLL(tft);
  drawSpiderRR(tft);
}

//Method to draw a spider using 
//individual lines and shapes
void drawSpider(int xPos, Adafruit_TFTLCD tft)
{
  tft.setCursor(0, 0);
  
  // Draw the central core of the spider
  const byte yPosCenter = 80;
  const byte xPosCenter = xPos; 
  const byte centralCircleRadius = 10;
  tft.drawCircle(xPosCenter, yPosCenter, centralCircleRadius, BLACK); 
  tft.fillCircle(xPosCenter, yPosCenter, centralCircleRadius, BLACK);

  // Draw the first leg of the spider
  const byte leg1xStart = xPosCenter;
  const byte leg1yStart = yPosCenter;
  const byte leg1xEnd = xPosCenter + 20;   
  const byte leg1yEnd = yPosCenter + 20; 
  tft.drawLine(leg1xStart, leg1yStart, leg1xEnd, leg1yEnd, BLACK); 

  // Draw the second leg of the spider
  const byte leg2xStart = xPosCenter;
  const byte leg2yStart = yPosCenter;
  const byte leg2xEnd = xPosCenter + 20;   
  const byte leg2yEnd = yPosCenter + 8; 
  tft.drawLine(leg2xStart, leg2yStart, leg2xEnd, leg2yEnd, BLACK);

  // Draw the third leg of the spider
  const byte leg3xStart = xPosCenter;
  const byte leg3yStart = yPosCenter;
  const byte leg3xEnd = xPosCenter + 20;   
  const byte leg3yEnd = yPosCenter - 8; 
  tft.drawLine(leg3xStart, leg3yStart, leg3xEnd, leg3yEnd, BLACK); 

  // Draw the fourth leg of the spider
  const byte leg4xStart = xPosCenter;
  const byte leg4yStart = yPosCenter;
  const byte leg4xEnd = xPosCenter + 20;   
  const byte leg4yEnd = yPosCenter - 20; 
  tft.drawLine(leg4xStart, leg4yStart, leg4xEnd, leg4yEnd, BLACK); 

  // Draw the fifth leg of the spider
  const byte leg5xStart = xPosCenter;
  const byte leg5yStart = yPosCenter;
  const byte leg5xEnd = xPosCenter - 20;   
  const byte leg5yEnd = yPosCenter + 20; 
  tft.drawLine(leg5xStart, leg5yStart, leg5xEnd, leg5yEnd, BLACK); 

  // Draw the sixth leg of the spider
  const byte leg6xStart = xPosCenter;
  const byte leg6yStart = yPosCenter;
  const byte leg6xEnd = xPosCenter - 20;   
  const byte leg6yEnd = yPosCenter + 8; 
  tft.drawLine(leg6xStart, leg6yStart, leg6xEnd, leg6yEnd, BLACK);

  // Draw the seventh leg of the spider
  const byte leg7xStart = xPosCenter;
  const byte leg7yStart = yPosCenter;
  const byte leg7xEnd = xPosCenter - 20;   
  const byte leg7yEnd = yPosCenter - 8; 
  tft.drawLine(leg7xStart, leg7yStart, leg7xEnd, leg7yEnd, BLACK); 

  // Draw the eigth leg of the spider
  const byte leg8xStart = xPosCenter;
  const byte leg8yStart = yPosCenter;
  const byte leg8xEnd = xPosCenter - 20;   
  const byte leg8yEnd = yPosCenter - 20; 
  tft.drawLine(leg8xStart, leg8yStart, leg8xEnd, leg8yEnd, BLACK); 
}

//Method to draw a magenta spider, based
//on background colors
void drawMAGENTASpider(int xPos, Adafruit_TFTLCD tft)
{
  tft.setCursor(0, 0);
  
  // Draw the central core of the spider
  const byte yPosCenter = 80;
  const byte xPosCenter = xPos; 
  const byte centralCircleRadius = 10;
  tft.drawCircle(xPosCenter, yPosCenter, centralCircleRadius, MAGENTA); 
  tft.fillCircle(xPosCenter, yPosCenter, centralCircleRadius, MAGENTA);

  // Draw the first leg of the spider
  const byte leg1xStart = xPosCenter;
  const byte leg1yStart = yPosCenter;
  const byte leg1xEnd = xPosCenter + 20;   
  const byte leg1yEnd = yPosCenter + 20; 
  tft.drawLine(leg1xStart, leg1yStart, leg1xEnd, leg1yEnd, MAGENTA); 

  // Draw the second leg of the spider
  const byte leg2xStart = xPosCenter;
  const byte leg2yStart = yPosCenter;
  const byte leg2xEnd = xPosCenter + 20;   
  const byte leg2yEnd = yPosCenter + 8; 
  tft.drawLine(leg2xStart, leg2yStart, leg2xEnd, leg2yEnd, MAGENTA);

  // Draw the third leg of the spider
  const byte leg3xStart = xPosCenter;
  const byte leg3yStart = yPosCenter;
  const byte leg3xEnd = xPosCenter + 20;   
  const byte leg3yEnd = yPosCenter - 8; 
  tft.drawLine(leg3xStart, leg3yStart, leg3xEnd, leg3yEnd, MAGENTA); 

  // Draw the fourth leg of the spider
  const byte leg4xStart = xPosCenter;
  const byte leg4yStart = yPosCenter;
  const byte leg4xEnd = xPosCenter + 20;   
  const byte leg4yEnd = yPosCenter - 20; 
  tft.drawLine(leg4xStart, leg4yStart, leg4xEnd, leg4yEnd, MAGENTA); 

  // Draw the fifth leg of the spider
  const byte leg5xStart = xPosCenter;
  const byte leg5yStart = yPosCenter;
  const byte leg5xEnd = xPosCenter - 20;   
  const byte leg5yEnd = yPosCenter + 20; 
  tft.drawLine(leg5xStart, leg5yStart, leg5xEnd, leg5yEnd, MAGENTA); 

  // Draw the sixth leg of the spider
  const byte leg6xStart = xPosCenter;
  const byte leg6yStart = yPosCenter;
  const byte leg6xEnd = xPosCenter - 20;   
  const byte leg6yEnd = yPosCenter + 8; 
  tft.drawLine(leg6xStart, leg6yStart, leg6xEnd, leg6yEnd, MAGENTA);

  // Draw the seventh leg of the spider
  const byte leg7xStart = xPosCenter;
  const byte leg7yStart = yPosCenter;
  const byte leg7xEnd = xPosCenter - 20;   
  const byte leg7yEnd = yPosCenter - 8; 
  tft.drawLine(leg7xStart, leg7yStart, leg7xEnd, leg7yEnd, MAGENTA); 

  // Draw the eigth leg of the spider
  const byte leg8xStart = xPosCenter;
  const byte leg8yStart = yPosCenter;
  const byte leg8xEnd = xPosCenter - 20;   
  const byte leg8yEnd = yPosCenter - 20; 
  tft.drawLine(leg8xStart, leg8yStart, leg8xEnd, leg8yEnd, MAGENTA); 
}

//Method to draw a spider on the leftmost
//water spout
void drawSpiderLL(Adafruit_TFTLCD tft)
{
  drawSpider(100, tft);
  isSpiderLL = true;
  isSpiderLR = false;
}

//Method to draw a spider on the water
//spout second from the left
void drawSpiderLR(Adafruit_TFTLCD tft)
{
  drawSpider(140, tft);
  isSpiderLL = false;
  isSpiderLR = true; 
}

//Method to draw a spider on the water
//spout second from the right
void drawSpiderRL(Adafruit_TFTLCD tft)
{
  tft.setCursor(0, 0);
  drawSpider(180, tft);
  isSpiderRL = true;
  isSpiderRR = false;
}

//Method to draw a spider on the rightmost
//water spout
void drawSpiderRR(Adafruit_TFTLCD tft)
{
  tft.setCursor(0, 0);
  drawSpider(220, tft);
  isSpiderRL = false;
  isSpiderRR = true;
}

//Method to draw a magenta spider in the 
//leftmost water spout
void drawSpiderMAGENTALL(Adafruit_TFTLCD tft)
{
  drawMAGENTASpider(100, tft);
  isSpiderLL = false;
  isSpiderLR = true;
}

//Method to draw a magenta spider in the 
//water spout second from the left
void drawSpiderMAGENTALR(Adafruit_TFTLCD tft)
{
  drawMAGENTASpider(140, tft); 
  isSpiderLL = true;
  isSpiderLR = false;
}

//Method to draw a magenta spider in the
//water spout second from the right
void drawSpiderMAGENTARL(Adafruit_TFTLCD tft)
{
  drawMAGENTASpider(180, tft);
  isSpiderRL = false;
  isSpiderRR = true;
}

//Method to draw a magenta spider in the 
//rightmost water spout
void drawSpiderMAGENTARR(Adafruit_TFTLCD tft)
{
  drawMAGENTASpider(220, tft);
  isSpiderRL = true;
  isSpiderRR = false;
}

//Method to switch the position of the
//spider on the right half of the screen
void switchSpiderRight()
{
  if(isSpiderRL == true)
  {
    isSpiderRL = false;
    isSpiderRR = true; 
    drawSpiderMAGENTARL(tft); 
    drawSpiderRR(tft); 
  }
  else
  {  
    isSpiderRL = true; 
    isSpiderRR = false; 
    drawSpiderMAGENTARR(tft); 
    drawSpiderRL(tft); 
  }
}

//Method to switch the position of 
//the spider on the left half of the screen
void switchSpiderLeft()
{
  if(isSpiderLL == true)
  {
    isSpiderLL = false;
    isSpiderLR = true;
    drawSpiderMAGENTALL(tft); 
    drawSpiderLR(tft); 
  }
  else
  {
    isSpiderLL = true;
    isSpiderLR = false; 
    drawSpiderMAGENTALR(tft); 
    drawSpiderLL(tft); 
  }
}

//Method to display the game over screen
unsigned long gameText() {
  tft.setRotation(2);
  tft.fillScreen(RED);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE); 
  tft.setTextSize(5);
  tft.println("********************");
  tft.setTextColor(BLACK);
  tft.setTextSize(8);
  tft.println("GAME");
  tft.println("OVER");
  tft.setTextSize(3);
  tft.println("Unplug and replug to play again.");
  tft.println("Your final score was:");
  tft.println(scr);
  tft.setTextColor(WHITE);
  tft.setTextSize(5);
  tft.println("********************");
  tft.setRotation(0);
  scr = 0;
}

//Method to display the starting screen
unsigned long testText() {
  tft.fillScreen(BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  
  tft.setTextColor(WHITE);  tft.setTextSize(5);
  tft.println("********************");
  tft.println("ITSY BITSY");
  tft.println("SPIDER");
  tft.println(); 
  
  tft.setTextColor(RED); 
  tft.setTextSize(3);
  tft.println("Press BLUE for");
  tft.println("beginner");
  tft.println("Press RED for");
  tft.println("advanced"); 
  tft.println(); 
  
  tft.setTextColor(MAGENTA);
  tft.setTextSize(4);
  tft.println(); 
  tft.println("By: Engenies");

  tft.setTextColor(WHITE);  
  tft.setTextSize(5);
  tft.println("********************");
  
  return micros() - start;
}

//Additional variables for use with 
//coordinates
int xLL = 80;
int xLMiddle = 120; 
int xCenter = 160;
int xRMiddle = 200;
int xRR = 240; 

int yBase = 480;
 
//Method to wet the leftmost rectangle
void wetLL()
{
  tft.fillRect(80, 0, 40, 480, CYAN);
  tft.drawRect(80, 0, 40, 480, CYAN);
}

//Method to wet the rectangle second
//from the left
void wetLR()
{
  tft.fillRect(120, 0, 40, 480, CYAN);
  tft.drawRect(120, 0, 40, 480, CYAN);
}

//Method to wet the rectangle second
//from the right
void wetRL()
{
  tft.fillRect(160, 0, 40, 480, CYAN);
  tft.drawRect(160, 0, 40, 480, CYAN);
}

//Method to wet the rightmost rectangle
void wetRR()
{
  tft.fillRect(200, 0, 40, 480, CYAN);
  tft.drawRect(200, 0, 40, 480, CYAN);
}

//Method to dry the leftmost rectangle
void dryLL()
{
  tft.fillRect(80, 0, 40, 480, MAGENTA);
  tft.drawRect(80, 0, 40, 480, MAGENTA);
}

//Method to dry the rectangle second
//from the left
void dryLR()
{
  tft.fillRect(120, 0, 40, 480, MAGENTA);
  tft.drawRect(120, 0, 40, 480, MAGENTA);
}

//Method to dry the rectangle second 
//from the right
void dryRL()
{
  tft.fillRect(160, 0, 40, 480, MAGENTA);
  tft.drawRect(160, 0, 40, 480, MAGENTA);
}

//Method to dry the rightmost rectangle
void dryRR()
{
  tft.fillRect(200, 0, 40, 480, MAGENTA);
  tft.drawRect(200, 0, 40, 480, MAGENTA);
}

