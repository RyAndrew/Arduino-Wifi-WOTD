
#include "U8glib.h";

U8GLIB_SH1106_128X64_2X  u8g = U8GLIB_SH1106_128X64_2X(U8G_I2C_OPT_NO_ACK);

String wodWord, wodDefinition;

//word of the day - word buffer
char wodWordChar[40];
//for scrolling word:
u8g_uint_t wodWordCharWidth;
int16_t wodWordCharXPosScroll;

//word of the day - definition buffer
char wodDefinitionChar[100];
uint16_t wodDefinitionCharLen;

//for scrolling definition:
uint16_t wodDefinitionPos = 0;

uint8_t DefinitionLetterCountTypical = 16;
boolean endOfDefinitionFlag = false;

char scrollTextOne[18];
char scrollTextTwo[18];

u8g_uint_t scrollTextOneWidth;
u8g_uint_t scrollTextTwoWidth;

int16_t scrollTextOneXPosScroll;
int16_t scrollTextTwoXPosScroll;

void setup() {
  
  //For Debug Only:
  Serial.begin(115200);
  Serial.println("booting");
  
  Serial1.begin(57600); // AT+UART=115200,8,1,None,NFC

  //show boot message on screen:
  u8g.firstPage();  
  do {
    drawboot();
  } while( u8g.nextPage() );
  
  delay(10000); // wait 10 seconds to connect to wifi
  getWordOfTheDay();

}


void loop() {
  //debug
  // decrement scroller
  //char xPosScrollChar[3];
  //itoa(xPosScroll, xPosScrollChar, 10);
  //Serial.println(xPosScrollChar);


  checkWordScrollPosition();
  
  //Serial.println("------");
  //Serial.print("wodDefinitionPos: ");
  //Serial.println(wodDefinitionPos);

  //getNextDisplayTextChunk(scrollTextOne, scrollTextOneWidth);
  
  //Serial.print("scrollTextOne: ");
  //Serial.println(scrollTextOne);
  //Serial.print("scrollTextOneWidth: ");
  //Serial.println(scrollTextOneWidth);
  //Serial.print("scrollTextOneXPosScroll: ");
  //Serial.println(scrollTextOneXPosScroll);

  //getNextDisplayTextChunk(scrollTextTwo, scrollTextTwoWidth);
  
  //Serial.print("scrollTextTwo: ");
  //Serial.println(scrollTextTwo);
  //Serial.print("scrollTextTwoWidth: ");
  //Serial.println(scrollTextTwoWidth);
  //Serial.print("scrollTextTwoXPosScroll: ");
  //Serial.println(scrollTextTwoXPosScroll);

  
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
  
  //check if we need to scroll the word
  if(wodWordCharWidth > 128){
    wodWordCharXPosScroll -= 3;
  }
  //decrement scrollers for definition word chunks
  scrollTextOneXPosScroll -= 3;
  scrollTextTwoXPosScroll -= 3;
  
  //Serial.print("wodWordCharXPosScroll: ");
  //Serial.println(wodWordCharXPosScroll);
  
  //Delay for debugging
  //delay(250);
}

void drawboot(){
  u8g.setFont(u8g_font_helvR12); //gentium
  u8g.setFontPosTop();
  u8g.drawStr( 0, 0, "Booting!" );
}

void getWordOfTheDay(){
  
  //test word
  //strncpy(wodWordChar, "Comeuppance\0", 13);
  //test definition
  //strncpy(wodDefinitionChar, "Deserved reward or just deserts, usually unpleasant.\0", 54);
  
  enterCommandMode();
  sendHttpRequestSockB();
  
  //for debug
  //Serial.print("wodWord String:");
  //Serial.println(wodWord);
  //Serial.print("wodDefinition String:");
  //Serial.println(wodDefinition);
  
  wodWord.toCharArray(wodWordChar, wodWord.length() + 1);
  wodDefinition.toCharArray(wodDefinitionChar, wodDefinition.length() + 1);
  
  //for debug
  //Serial.print("wodWordChar []:");
  //Serial.println(wodWordChar);
  //Serial.print("wodDefinitionChar []:");
  //Serial.println(wodDefinitionChar);
  
  u8g.setFont(u8g_font_gdb14);
  wodWordCharWidth = u8g.getStrWidth(wodWordChar);
  
  wodDefinitionCharLen = strlen(wodDefinitionChar);
  
  //Serial.print("wodDefinitionChar: ");
  //Serial.println(wodDefinitionChar);
  //Serial.print("wodDefinitionCharLen: ");
  //Serial.println(wodDefinitionCharLen);
  
  //reset all scroll related vars
  scrollTextOneWidth = 0; //initial value, set a width equal to the screen width. This will trigger grabbing the first chunk of text
  scrollTextTwoWidth = 128; //initial value is specific to allow scrollTextOneXPosScroll to be calculated properly 

  scrollTextOneXPosScroll = 0;
  scrollTextTwoXPosScroll = 0;
}

void draw() {
  
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_gdb14); //gentium
  u8g.setFontPosTop();
  u8g.drawStr( wodWordCharXPosScroll, 0, wodWordChar );
  
  u8g.setFont(u8g_font_gdb11);
  u8g.drawStr( scrollTextOneXPosScroll, 40, scrollTextOne );
  u8g.drawStr( scrollTextTwoXPosScroll, 40, scrollTextTwo );
}

void checkWordScrollPosition(){
  if(scrollTextOneXPosScroll <= scrollTextOneWidth * -1){
    getNextDisplayTextChunk(scrollTextOne, &scrollTextOneWidth);
    scrollTextOneXPosScroll = scrollTextTwoXPosScroll + scrollTextTwoWidth;
  }
  if(scrollTextTwoXPosScroll <= scrollTextTwoWidth * -1){
    getNextDisplayTextChunk(scrollTextTwo, &scrollTextTwoWidth);
    scrollTextTwoXPosScroll = scrollTextOneXPosScroll + scrollTextOneWidth;
  }
}

void getNextDisplayTextChunk(char* newCharChunk, u8g_uint_t* newCharChunkWidth){
  uint16_t charsToRead = DefinitionLetterCountTypical;
  
  //Serial.println("getNextDisplayTextChunk!");
  //Serial.print("wodDefinitionPos: ");
  //Serial.println(wodDefinitionPos);
  
  //are we at the end of the definition?
  if(wodDefinitionPos + DefinitionLetterCountTypical > wodDefinitionCharLen ){
    charsToRead = wodDefinitionCharLen - wodDefinitionPos;
  }
  
  //Serial.print("charsToRead: ");
  //Serial.println(charsToRead);
  //Serial.print("wodDefinitionPos: ");
  //Serial.println(wodDefinitionPos);
  
  strncpy(newCharChunk, wodDefinitionChar + wodDefinitionPos, charsToRead);
  newCharChunk[charsToRead] = '\0';

  if(wodDefinitionPos + DefinitionLetterCountTypical > wodDefinitionCharLen ){
    wodDefinitionPos = 0;
    endOfDefinitionFlag = true;
  }else{
    wodDefinitionPos += charsToRead;
  }

    if(endOfDefinitionFlag){
      *newCharChunkWidth = 128;
      endOfDefinitionFlag = false;
    }else{
      u8g.setFont(u8g_font_gdb11);
      *newCharChunkWidth = u8g.getStrWidth(newCharChunk);
    }
}

///////////////////////////////////////
// WIFI STUFF:

boolean sendWifiSerialCmd(String command){
  Serial1.print(command + "\r");
}
boolean sendWifiSerialCmdWaitForOk(String command){
  
  sendWifiSerialCmd(command);
  
  if(Serial1.find("+ok")){
    return true;
  }else{
    //Serial.println("Error! No +ok received from command!");
    return false;
  }
  
}

void sendHttpRequestSockB()
{
  //Serial.println("Send HTTP Request on Socket B!");
  
  //Serial.println("Disconnecting Socket B, sending AT+TCPDISB=off");
  sendWifiSerialCmdWaitForOk("AT+TCPDISB=off");
  
  //Serial.println("Setting Socket B Parameters, sending AT+SOCKB=");
  sendWifiSerialCmdWaitForOk("AT+SOCKB=TCP,80,www.2xod.com");
  
  //Serial.println("Connecting Socket B, sending AT+TCPDISB=on");
  sendWifiSerialCmdWaitForOk("AT+TCPDISB=on\r");
  
  //Serial.println("Wait for socket B to connect");
  waitForSocketBToConnect();

  String HttpRequest = "GET /wordOfTheDay/ HTTP/1.1\r\n";
      HttpRequest += "Host: www.2xod.com\r\n";
      HttpRequest += "\r\n";
  unsigned int HttpRequestLength = HttpRequest.length();
  
  String HttpRequestLengthChar = (String)HttpRequestLength;
  //Serial.println("Sending Data, sending AT+SNDB=");
  //Serial.println("AT+SNDB=" + HttpRequestLengthChar );
  Serial1.print("AT+SNDB=" + HttpRequestLengthChar  + "\r" );
  Serial1.find(">");
  
  //Serial.println("Sending HTTP Request");
  //Serial.print(HttpRequest);
  Serial1.print(HttpRequest);
  
  Serial1.flush();
  Serial1.find("+ok");
  
  //Serial.println("Receiving Back 300 Bytes");
  Serial1.print("AT+RCVB=300\r");
  //Expecting to see "HTTP/1.1 200 OK" on the terminal
  
  String response = readAllSerialOutputForMs(4000);

/*  
// TODO strip HTTP headers and such more cleanly
  int strSearch = response.indexOf(','); //Response start
    //Serial.println("HTTP Response Search:");
    //Serial.println(strSearch);
  if(strSearch){
    response = response.substring(strSearch);
  }else{
    //Serial.println("HTTP Response Not Found!");
  }
  
  strSearch = response.indexOf("\r\n\r\n"); //HTTP Body Start
    //Serial.println("HTTP Body Search:");
    //Serial.println(strSearch);
  if(strSearch){
    response = response.substring(strSearch + 4);
  }else{
    //Serial.println("HTTP Body Not Found!");
  }
  
  strSearch = response.indexOf('\0'); //HTTP Response End
    //Serial.println("HTTP Response End Search:");
    //Serial.println(strSearch);
  if(strSearch){
    response = response.substring(0, strSearch);
  }else{
    //Serial.println("HTTP Response End Not Found!");
  }
*/

  //debug
  //Serial.println("------full HTTP resp--");
  //Serial.println(response);
  //Serial.println("------");
  
  int strSearch = response.indexOf('|'); //HTTP Response Token Start
  //Serial.println("HTTP Response End Search:");
  //Serial.println(strSearch);
  if(strSearch){
    response = response.substring(strSearch + 1);
  }else{
    //Serial.println("HTTP Response End Not Found!");
  }

  strSearch = response.indexOf('|'); //HTTP Response Token Start
  //Serial.println("HTTP Response End Search:");
  //Serial.println(strSearch);
  if(strSearch){
    wodWord = response.substring(0, strSearch);
    response = response.substring(strSearch + 1);
  }else{
    //Serial.println("HTTP Response End Not Found!");
  }
  
  strSearch = response.lastIndexOf('|'); //HTTP Response Token Start
  //Serial.println("HTTP Response End Search:");
  //Serial.println(strSearch);
  if(strSearch){
    wodDefinition = response.substring(0, strSearch);
  }else{
    //Serial.println("HTTP Response End Not Found!");
  }
  
  //DEBUG
  //Serial.println("------word--");
  //Serial.println(wodWord);
  //Serial.println("------");
  //Serial.println("------definition--");
  //Serial.println(wodDefinition);
  //Serial.println("------");
  

  
  //Serial.println("Disconnecting Socket B, sending AT+TCPDISB=off");
  sendWifiSerialCmdWaitForOk("AT+TCPDISB=off");
  
}

boolean waitForSocketBToConnect(){
 unsigned long waitTimeout = 1000; // wait 1 second to connect
 unsigned long start = millis();
 char lastCmdResultChar;
 
  Serial1.print("AT+TCPLKB\r");
    
  while( millis() - start < waitTimeout ){
    
    if (Serial1.available()) { //Output should be "+ok=on" so skip 1 char
        lastCmdResultChar = Serial1.read();
        //uncomment this for debugging. Will show "+ok=off" until a connection is made
        //Serial.write(lastCmdResultChar);
    }
    if(lastCmdResultChar == 'n'){ // Command Returned "+ok=on"
       //Serial.println("Connected in " + (String)(millis() - start) + " ms");
      return true;
    }else if(lastCmdResultChar == 'f'){ // Command Returned "+ok=off"
      lastCmdResultChar = 'x'; //Reset the character so we don't issue this command repeatedly
      Serial1.print("AT+TCPLKB\r");
    }
  }
  return false;
}

char* readAllSerialOutputForMs(unsigned long wait)
{
  char readChar;
  char result[300];
  int index = 0;
 unsigned long start = millis();
  while( millis() - start < wait ){

    if (Serial1.available()) {
      readChar = Serial1.read();
      //Debug:
      //Serial.write(readChar);
      result[index] = readChar;
      index++;
    }
  }
// TODO? Fill rest of char[] with nulls
//  while(index < 300){
//    result[index] = '\0';
//    index++;
//  }
  result[index] = '\0';
  return result;
}

boolean enterCommandMode()
{
  //Serial.println("Enter Command Mode");
  
  Serial1.write('+');
  delay(50);
  Serial1.write('+');
  delay(50);
  Serial1.write('+');
  if(!Serial1.find("a")){
    //Serial.println("Error! No Ack from wifi module!");
    return false;
  }
  
  Serial1.write('a');
  if(!Serial1.find("+ok")){
    //Serial.println("Error! No OK received from wifi module!");
    return false;
  }  
  //Serial.println("Ready for commands!");
  return true;
}
