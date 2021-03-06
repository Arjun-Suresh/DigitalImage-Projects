// =============================================================================
// VIZA654/CSCE646 at Texas A&M University
// Homework 0
// Created by Arjun Suresh based on the template put up on the project
// description page. This program primarily can read either P3 or P6 ppm file
// and write a p3 ppm file.
//
// =============================================================================

#include <cstdlib>
#include <iostream>
#include <GL/glut.h>
#include <math.h>
#include <fstream>
#include <cassert>
#include <sstream>
#include <cstring>
#include <malloc.h>

#define WIDTHVALUEID 1
#define HEIGHTVALUEID 2
#define MAXCOLORVALUEID 3
#define COLORFOREGROUND 0
#define COLORBACKGROUND 1
using namespace std;

// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height, maxColorValue, magicNo;
unsigned char* pixmap;
//Resizing the read buffer for large image files
unsigned char* resizeArray(unsigned char* oldArray, long int oldSize, long int& newSize) 
{
    newSize = oldSize * 2;
    unsigned char* newArray = new unsigned char[newSize];
    std::memcpy( newArray, oldArray, oldSize * sizeof(unsigned char) );
    return newArray;
}

// =============================================================================
// setPixels()
//
// This function stores the RGB values of each pixel to "pixmap."
// Then, "glutDisplayFunc" below will use pixmap to display the pixel colors.
// =============================================================================
void setPixels(int x, int y, unsigned char* color)
{
  int i = (y * width + x) * 3; 
  pixmap[i++] = color[0];
  pixmap[i++] = color[1]; //Do you know what "0xFF" represents? Google it!
  pixmap[i] = color[2]; //Learn to use the "0x" notation to your advantage.
}



// =============================================================================
// OpenGL Display and Mouse Processing Functions.
//
// You can read up on OpenGL and modify these functions, as well as the commands
// in main(), to perform more sophisticated display or GUI behavior. This code
// will service the bare minimum display needs for most assignments.
// =============================================================================
static void windowResize(int w, int h)
{   
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,width,0,height,0,1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity() ;
}

static void windowDisplay(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(0,0);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixmap);
  glFlush();
}

static void processMouse(int button, int state, int x, int y)
{
  if(state == GLUT_UP)
  exit(0);               // Exit on mouse click.
}

static void init(void)
{
  glClearColor(1,1,1,1); // Set background color.
}

//**********************Functions to parse the header fields of the ppm file********************
void parseCommentLine(long int& index, unsigned char* fileBuffer, long int numOfCharacters)
{
  while(index<numOfCharacters && fileBuffer[index]!='\n')
  index++;
}

bool parseMagicNumber(long int& index, unsigned char* fileBuffer, long int numOfCharacters, int& magicNumberParsed)
{
  while(index<numOfCharacters)
  {
    if(fileBuffer[index]=='#')
    {
      parseCommentLine(index, fileBuffer, numOfCharacters);
      break;
    }
    if(fileBuffer[index]=='P' || fileBuffer[index]=='p')
    {
      index++;
      if(fileBuffer[index]=='1' || fileBuffer[index]=='2'|| fileBuffer[index]=='3' || fileBuffer[index]=='4' || fileBuffer[index]=='5' || fileBuffer[index]=='6')
      {
        magicNumberParsed=1;
	magicNo=fileBuffer[index]-48;
        break;
      }
      else 
        return false;
    }
    if(isspace(fileBuffer[index]) && fileBuffer[index]!='\n')
      index++;
    else
      return false;
  }
  if(magicNumberParsed)
  {
    index++;
    if(index<numOfCharacters && !(isspace(fileBuffer[index]) || fileBuffer[index]=='#'))
      return false;
    else
    {
      if(fileBuffer[index]=='#')
      {
        parseCommentLine(index, fileBuffer, numOfCharacters);
      }
    }
        
  }
  return true;
}

bool parseValue(long int& index, unsigned char* fileBuffer, long int numOfCharacters, int& parseValue, int valueId)
{
  int value;
  while(index<numOfCharacters)
  {
    if(fileBuffer[index]=='#')
    {
      parseCommentLine(index, fileBuffer, numOfCharacters);
      break;
    }
    if(isdigit(fileBuffer[index]))
    {
      char valueString[20];
      int k=0;
      valueString[k++]=fileBuffer[index++];

      while(isdigit(fileBuffer[index]) && index<numOfCharacters)
        valueString[k++]=fileBuffer[index++];
      valueString[k]='\0';
      value = atoi(valueString);
      parseValue=1;
      break;
    }
    if(isspace(fileBuffer[index]))
      index++;
    else
      return false;
  }
  if(parseValue)
  {
    if(index<numOfCharacters && !((isspace(fileBuffer[index]) && valueId != MAXCOLORVALUEID) || (valueId == MAXCOLORVALUEID && fileBuffer[index] == '\n') || fileBuffer[index]=='#'))
      return false;
    else
    {
      if(fileBuffer[index]=='#')
      {
        parseCommentLine(index, fileBuffer, numOfCharacters);
      }
    }        
  }
  switch(valueId)
  {
    case WIDTHVALUEID:
      width=value;
      break;
    case HEIGHTVALUEID:
      height=value;
      break;
    case MAXCOLORVALUEID:
      maxColorValue=value;
      break;
  }
  return true;
}

//**************Function to fill image pixmap data from a P3 PPM file*****************
bool fillPixels(long int& index, unsigned char* fileBuffer, long int numOfCharacters)
{
  unsigned char color[3];
  long int flag=0;
  int rowVal=0, colVal=0;
  while(index<numOfCharacters)
  {
    if(isdigit(fileBuffer[index]))
    {
      int value=0;
      char valueString[20];
      int k=0;
      valueString[k++]=fileBuffer[index++];
      while(isdigit(fileBuffer[index]) && index<numOfCharacters)
        valueString[k++]=fileBuffer[index++];
      valueString[k]='\0';
      value = atoi(valueString);
      if(value>maxColorValue)
        return false;
      color[flag%3]=(char)value;
      flag=(flag+1)%3;
      if(flag%3==0)
      {
        while(index<numOfCharacters && isspace(fileBuffer[index]))
          index++;
        setPixels(colVal, height-rowVal-1, color);
        colVal++;
        if(colVal==width)
        {
          rowVal++;
          colVal=0;
        }
      }
      else
      {
        if(!isspace(fileBuffer[index]))
          return false;
        index++;
      }
     }
     else
       index++;
   }
   return true;
}

//**************Function to fill image pixmap data from a P6 PPM file*****************
bool fillPixelsBin(long int& index, unsigned char* fileBuffer, long int numOfCharacters)
{
  long int rowVal=0,colVal=0;
  while(rowVal<height)
  {
    long int val=(((height-rowVal-1)*width)+colVal)*3;
    for(int i=0;i<3;i++)
    	pixmap[val++]=fileBuffer[index++];
    colVal=(colVal+1)%width;
    if(!colVal)
      rowVal++;
  }
  cout<<"Done\n";
  return true;
}

//***************Main read PPM file function********************************************
bool readPPMFile(char* filePath)
{
  std::fstream ppmFile;
  std::ifstream checkFile(filePath);
  long int numOfCharacters=0;
  int magicNumberParsed=0, widthParsed=0, heightParsed=0, maxColorValueParsed=0;
  if(!checkFile.good())
  {
    cout<<"Filename not found\n";
    return false;
  }
  ppmFile.open (filePath, std::ios::in|std::ios::binary);
  ppmFile.seekg(0,std::ios::end);
  ppmFile.seekg(0,std::ios::end);
  numOfCharacters = ppmFile.tellg();
  ppmFile.seekg(0,std::ios::beg);
  unsigned char* fileBuffer = new unsigned char[numOfCharacters+1];
  ppmFile.read((char *)fileBuffer, numOfCharacters);
  fileBuffer[numOfCharacters]='\0';
  ppmFile.close();
  for(long int index=0;index<numOfCharacters;index++)
  {
    if(fileBuffer[index]=='#')
    {
      parseCommentLine(index, fileBuffer, numOfCharacters);
      continue;
    }
    if(!magicNumberParsed)
    {
      if(!parseMagicNumber(index, fileBuffer, numOfCharacters, magicNumberParsed))
      {
        return false;
      }
        
      continue;
    }
    if(magicNumberParsed && !widthParsed)
    {
      if(!parseValue(index, fileBuffer, numOfCharacters, widthParsed, WIDTHVALUEID))
      {
        return false;
      }
      continue;
    }
    if(magicNumberParsed && widthParsed && !heightParsed)
    {
      if(!parseValue(index, fileBuffer, numOfCharacters, heightParsed, HEIGHTVALUEID))
      {
        return false;
      }
      continue;
    }
    if(magicNumberParsed && widthParsed && heightParsed && !maxColorValueParsed)
    {
      if(!parseValue(index, fileBuffer, numOfCharacters, maxColorValueParsed, MAXCOLORVALUEID))
      {
        return false;
      }
      continue;
    }
    if(magicNumberParsed && widthParsed && heightParsed && maxColorValueParsed)
    {
      pixmap = new unsigned char[width*height*3];
      if(magicNo==3)
      {
        
        if(!(fillPixels(index, fileBuffer, numOfCharacters)))
        {
          return false;
        }
        break;
      }
      else
      {
        if(!(fillPixelsBin(index, fileBuffer, numOfCharacters)))
          return false;
        return true;
      }
    }
  }
  delete[] fileBuffer;
  return true;
}

/* *********************************** Code for saving a ppm file*****************************/

void fillCharacters(unsigned char* fileBuffer, long int& index, char* data)
{
  for(int i=0;i<strlen(data);i++)
    fileBuffer[index++]=data[i];
}

void writeToFile(unsigned char* fileBuffer, long int numberOfCharacters, fstream& ppmFile)
{  
  for(long int i=0; i< numberOfCharacters; i++)
  {
    ppmFile << fileBuffer[i];
  }
}

void writeHeader(unsigned char* fileBuffer, long int& index, int format)
{
  width=1000;
  height=500;
  maxColorValue=255;
  char widthString[5], heightString[5], maxColorString[5];
  sprintf(widthString, "%d", width);
  sprintf(heightString, "%d", height);
  sprintf(maxColorString, "%d", maxColorValue);
  char magicNumberString[3];
  switch (format)
  {
    case 1:
      strcpy(magicNumberString, "P3");
      break;
    case 2:
      strcpy(magicNumberString, "P6");
      break;
  }
  fillCharacters(fileBuffer, index, magicNumberString);
  fileBuffer[index++]='\n';
  fillCharacters(fileBuffer, index, widthString);
  fileBuffer[index++]=' ';
  fillCharacters(fileBuffer, index, heightString);
  fileBuffer[index++]='\n';
  fillCharacters(fileBuffer, index, maxColorString);
  fileBuffer[index++]='\n';  
}

//********************This function fills in black for foreground and yellow for background
//This function is called for background fill if pixmap has 0 for that pixel and,
//foreground fill if pixmap has 1 for that pixel. Consequently, the drawing has a black circle
//in a yellow background*********************************************************************
void fillColor(unsigned char* fileBuffer, long int& index, int& charCount, int colorId, int format)
{
  if(format == 1)
  {
    char colorVal[20];
    switch(colorId)
    {
      case COLORFOREGROUND:
        strcpy(colorVal, "0 0 0 ");
        break;
      case COLORBACKGROUND:
        strcpy(colorVal, "255 255 0 ");
        break;
    }
    for(int i=0;i<strlen(colorVal);i++)
    {
      fileBuffer[index++]=colorVal[i];    
      charCount++;
    }
  }
  else
  {
    switch(colorId)
    {
      case COLORFOREGROUND:
        for(int i=0;i<3;i++)
          fileBuffer[index++]=0;
        break;
      case COLORBACKGROUND:
          fileBuffer[index++]=255;
          fileBuffer[index++]=255;
          fileBuffer[index++]=0;
        break;
    }
  }
}

unsigned char* writePixelBufferToFileBuffer(unsigned char* pixelBuffer, unsigned char* fileBuffer, long int& index, long int origSize, int format)
{
  int charCount=0;
  for(int i=0;i<height;i++)
  {
    for(int j=0;j<width;j++)
    {
      if(index >= origSize-50)
      {
        unsigned char* tempBuffer = resizeArray(fileBuffer, origSize, origSize);
        delete[] fileBuffer;
        fileBuffer=tempBuffer;
      }
      if(pixelBuffer[i*width+j])
        fillColor(fileBuffer, index, charCount, COLORFOREGROUND, format);
      else
        fillColor(fileBuffer, index, charCount, COLORBACKGROUND, format);
      if (charCount >= 60)
      {
        fileBuffer[index++]='\n';
        charCount=0;
      }
    }
  }
  fileBuffer[index++]='\n';
  return  fileBuffer;
}

bool checkIfPointInCircle(int x, int y, int xcenter, int ycenter, int radius)
{
  double dist = pow((pow((xcenter-x),2)+pow((ycenter-y),2)), 0.5);
  int distanceRoundOff = (int) dist;
  if(distanceRoundOff<=radius)
    return true;
  return false;
}

//***************Create a pixmap array with 1 for pixel if it is in a circle
//and 0 if it is outside****************************************************
void fillPixelBuffer(unsigned char* pixelBuffer)
{
  for(int i=0;i<height;i++)
    for(int j=0; j<width;j++)
      pixelBuffer[i*width+j]=0;
  int xcenter=500,ycenter=250,radius=125;
  for(int i=0;i<height;i++)
  {
    for(int j=0; j<width;j++)
    {
      if(checkIfPointInCircle(j,i,xcenter,ycenter,radius))
        pixelBuffer[i*width+j]=1;
      else
        pixelBuffer[i*width+j]=0;
    }
  }
}

void generatePPMFile()
{
  std::cout<<"Enter 1 for P3 and 2 for P6 format file\n";
  int format;
  cin>>format;
  while(!(format==1 || format==2))
  {
    std::cout<<"Wrong option\nEnter 1 for P3 and 2 for P6 format file\n";
    cin>>format;
  }
  std::fstream ppmFile;
  ppmFile.open("pr01.ppm",std::fstream::out);
  long int index=0;
  unsigned char* fileBuffer = new unsigned char[10000];
  writeHeader(fileBuffer, index, format);
  unsigned char *pixelBuffer = new unsigned char[width*height];
  fillPixelBuffer(pixelBuffer);
  fileBuffer=writePixelBufferToFileBuffer(pixelBuffer, fileBuffer, index, 10000, format);
  writeToFile(fileBuffer, index, ppmFile);
  ppmFile.close();
}

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{

  //initialize the global variables
  //OpenGL Commands:
  // Once "glutMainLoop" is executed, the program loops indefinitely to all
  // glut functions.  
  glutInit(&argc, argv);
  char ppmFilePath[100];
  int option;
  while(1)
  {
    cout<<"Enter\n1. Read a ppm file\n2. Save a ppm file\n";
    cin>>option;
    int flag=0;
    switch(option)
    {
      case 1:
        std::cout<<"Give the path of the ppm file\n";
        std::cin>>ppmFilePath;
        if(!readPPMFile(ppmFilePath))
        {
          cout<<"Error\n";
          exit(-1);
        }
        flag=1;
        break;
      case 2:
        generatePPMFile();
        cout<<"Your file 'pr01.ppm' is ready in the current directory..\n";
        exit(0);
      default:
        cout<<"Please choose the right option\n";
        break;
    }
    if(flag)
      break;
  }
  glutInitWindowPosition(100, 100); // Where the window will display on-screen.
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutCreateWindow("Homework Zero");
  init();
  glutReshapeFunc(windowResize);
  glutDisplayFunc(windowDisplay);
  glutMouseFunc(processMouse);
  glutMainLoop();

  return 0; //This line never gets reached. We use it because "main" is type int.
}

