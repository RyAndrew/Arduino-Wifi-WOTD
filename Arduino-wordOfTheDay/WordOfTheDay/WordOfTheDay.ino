
#include "U8glib.h"
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NO_ACK);	// Display which does not send ACK

//TODO: Use 2X variants for better screen refresh rates?
//HW SPI 	u8g_dev_sh1106_128x64_2x_hw_spi 	U8GLIB_SH1106_128X64_2X(cs, a0 [, reset]) 	impl. 	impl. 	n.a. 	impl.
//I2C 	u8g_dev_sh1106_128x64_2x_i2c 	U8GLIB_SH1106_128X64_2X(U8G_I2C_OPT_NONE) 	impl. 	n.a. 	n.a. 	impl. 

// handle scrolling
uint8_t xPosScroll = 0;


char wodWordChar[20] = "Word\0", wodDefinitionChar[300] = "Definition\0";

void setup() {
  
  //For Debug Only:
  //Serial.begin(115200);
  //Serial.println("booting");
  
  Serial1.begin(57600); // AT+UART=115200,8,1,None,NFC
  
  delay(6000); // wait 6 seconds to connect to wifi
  enterCommandMode();
  sendHttpRequestSockB();
}

void draw(uint8_t xPos) {
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_gdb14); //gentium
  u8g.setFontPosCenter();
  
  u8g.drawStr( 0, 10, wodWordChar );
  u8g.setFont(u8g_font_gdb11); //gentium
  u8g.drawStr( xPos, 40, wodDefinitionChar );
}

void loop() {
  
  // decrement scroller
  xPosScroll -= 10;
  
  // picture loop
  u8g.firstPage();  
  do {
    draw(xPosScroll);
  } while( u8g.nextPage() );
  
}

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
// TODO strip HTTP headers and such correctly
  int strSearch = response.indexOf(','); //Response start
    //Serial.println("HTTP Response Search:");
    //Serial.println(strSearch);
  if(strSearch){
    response = response.substring(strSearch);
  }else{
    Serial.println("HTTP Response Not Found!");
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
  
  String wodWord, wodDefinition;

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
  
  wodWord.toCharArray(wodWordChar, wodWord.length() + 1);
  wodDefinition.toCharArray(wodDefinitionChar, wodDefinition.length() + 1);
  
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
