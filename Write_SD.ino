/*
*This program will create a file for data storage on SD
*Automatically updates the file name with the amount of entries on the card
*circuit - UNO - MEGA
*MOSI - 11 - 51
*MISO - 12 - 50
*CLK - 13 - 52
*CS - 10 - 53
*/
#include <SPI.h>
#include <SD.h>
File root;
File dataFile;
String fileName;
const int chipSelect = 10; // port for CS
int fileCount = 0;

int data1, data2, data3;

void setup() {
  Serial.begin(115200);
  while (!Serial); //wait till serial link has been established
  
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
  Serial.println("initialization failed!");
  while (true); // Pause if initialization fails
  }
  Serial.println("initialization done.");
  
  root = SD.open("/");
  numOfFiles(root);
  fileName = "Entry-" + String(fileCount-2) + ".txt";
 
  
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    Serial.println("file opened successfully");
    DataWrite();
    dataFile.close(); // close the file:
  } 
  else {
    Serial.println("error opening file"); // if the file didn't open, print an error message
  }
}
void loop() {
// nothing happens after setup
}

void numOfFiles(File dir) {
  while(true) {
    File entry = dir.openNextFile();
    fileCount++; 
    if (!entry) {
      break; //no more files
    }
  }
  dir.close();
}
void DataWrite() {
  makeData();
  dataFile.print(data1);
  dataFile.print("|");
  dataFile.print(data2);
  dataFile.print("|");
  dataFile.print(data3);
  dataFile.println("|");
}
void makeData () {
  data1 = random(1000);
  data2 = random(1312312);
  data3 = random(121132);
}
