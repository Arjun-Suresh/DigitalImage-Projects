// =============================================================================
// VIZA654/CSCE646 at Texas A&M University
// Homework 8
// Compositing
// 1. Normal, multiply, max, min
// 2. Chroma key compositing using over operator and median of weighted averages
// output files are generated in the same folder
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
#include <float.h>

#define WIDTHVALUEID 1
#define HEIGHTVALUEID 2
#define MAXCOLORVALUEID 3
#define COLORFOREGROUND 0
#define COLORBACKGROUND 1

#define REDOFFSET 0
#define GREENOFFSET 1
#define BLUEOFFSET 2


#define maximum(x, y, z) ((x) > (y)? ((x) > (z)? (x) : (z)) : ((y) > (z)? (y) : (z)))
#define minimum(x, y, z) ((x) < (y)? ((x) < (z)? (x) : (z)) : ((y) < (z)? (y) : (z)))

#define diff(a,b) ((a>b)?(a-b):(b-a))

#define KERNELSIZE 2
#define KERNELARRAYSIZE 4
using namespace std;
// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height, width1, height1, maxColorValue, magicNo;
unsigned char *pixmapBackGround, *pixmapForeGround, *pixmapTriMap, *pixmapComputed;

inline double mod(double x) 
{
  return (x > 0 ? (x) : (-1*x));
}

//Function to resize ppm file buffer array
unsigned char* resizeArray(unsigned char* oldArray, long int oldSize, long int& newSize) 
{
    newSize = oldSize * 2;
    unsigned char* newArray = new unsigned char[newSize];
    std::memcpy( newArray, oldArray, oldSize * sizeof(unsigned char) );
    return newArray;
}

//Set red, green and blue in the new pixMap array to be written to the ppm file
void setPixelColor(int y, int x, int red, int green, int blue)
{
  int i = (y * width + x) * 3; 
  pixmapComputed[i++] = red;
  pixmapComputed[i++] = green;
  pixmapComputed[i] = blue;
}

void setPixelColorOrig(long int &val, long int& index, unsigned char* fileBuffer, int option)
{
  if(option == 1)
    pixmapBackGround[val++]=fileBuffer[index++];
  else if(option == 2)
    pixmapForeGround[val++]=fileBuffer[index++];
  else if(option == 3)
    pixmapTriMap[val++]=fileBuffer[index++];
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
  glOrtho(0,(w/2),0,(h/2),0,1); 
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity() ;
}
static void windowDisplay(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(0,0);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixmapComputed);
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




//****************************************************************************************************************************
//**********************Functions to read data from ppm file******************************************************************
//****************************************************************************************************************************
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

bool parseValue(long int& index, unsigned char* fileBuffer, long int numOfCharacters, int& parseValue, int valueId, int option)
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
      if(option == 1 || option == 3)
      	width=value;
      else
        width1=value;
      break;
    case HEIGHTVALUEID:
      if(option == 1 || option == 3)
      	height=value;
      else
        height1=value;
      break;
    case MAXCOLORVALUEID:
      maxColorValue=value;
      break;
  }
  return true;
}

bool fillPixelsBin(long int& index, unsigned char* fileBuffer, long int numOfCharacters, int option)
{
  int wVal,hVal;
  wVal=width;
  hVal=height;
  long int rowVal=0,colVal=0;
  while(rowVal<hVal)
  {
    long int val=(((hVal-rowVal-1)*wVal)+colVal)*3;
    for(int i=0;i<3;i++)
    	setPixelColorOrig(val, index, fileBuffer, option);
    colVal=(colVal+1)%wVal;
    if(!colVal)
      rowVal++;
  }
  return true;
}

bool readPPMFile(char* filePath, int option)
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
      if(!parseValue(index, fileBuffer, numOfCharacters, widthParsed, WIDTHVALUEID, option))
      {
        return false;
      }
      continue;
    }
    if(magicNumberParsed && widthParsed && !heightParsed)
    {
      if(!parseValue(index, fileBuffer, numOfCharacters, heightParsed, HEIGHTVALUEID, option))
      {
        return false;
      }
      continue;
    }
    if(magicNumberParsed && widthParsed && heightParsed && !maxColorValueParsed)
    {
      if(!parseValue(index, fileBuffer, numOfCharacters, maxColorValueParsed, MAXCOLORVALUEID, option))
      {
        return false;
      }
      continue;
    }
    if(magicNumberParsed && widthParsed && heightParsed && maxColorValueParsed)
    {
      if(option == 1)
        pixmapBackGround = new unsigned char[width*height*3];
      else if(option == 2)
        pixmapForeGround = new unsigned char[width1*height1*3];
      else if(option == 3)
        pixmapTriMap = new unsigned char[width*height*3];
      if(!(fillPixelsBin(index, fileBuffer, numOfCharacters, option)))
        return false;
      delete[] fileBuffer;
      return true;
    }
  }
}





//************************************************************************************************************
//**********************Functions to save from pixelMap to ppm file*******************************************
//************************************************************************************************************
void fillCharacters(unsigned char* fileBuffer, long int& index, char* data)
{
  for(int i=0;i<strlen(data);i++)
  {
    fileBuffer[index++]=data[i];
  }
}

void writeToFile(unsigned char* fileBuffer, long int numberOfCharacters, fstream& ppmFile)
{  
  for(long int i=0; i< numberOfCharacters; i++)
  {
    ppmFile << fileBuffer[i];
  }
}

void writeHeader(unsigned char* fileBuffer, long int& index)
{
  maxColorValue=255;    
  char widthString[5], heightString[5], maxColorString[5];
  sprintf(widthString, "%d", width);
  sprintf(heightString, "%d", height);
  sprintf(maxColorString, "%d", maxColorValue);
  char magicNumberString[3];
  strcpy(magicNumberString, "P6");
  fillCharacters(fileBuffer, index, magicNumberString);
  fileBuffer[index++]='\n';
  fillCharacters(fileBuffer, index, widthString);
  fileBuffer[index++]=' ';
  fillCharacters(fileBuffer, index, heightString);
  fileBuffer[index++]='\n';
  fillCharacters(fileBuffer, index, maxColorString);
  fileBuffer[index++]='\n';  
}


unsigned char* writePixelBufferToFileBuffer(unsigned char* fileBuffer, long int& index, long int origSize)
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
      int k=((height-i-1)*width+j)*3;
      for(int x=0;x<3;x++)
      fileBuffer[index++]=pixmapComputed[k++];
    }
  }
  return fileBuffer;
}

void generatePPMFile(int option)
{
  std::fstream ppmFile;
  char fileName[50];
  switch(option)
  {
    case 1: 
      strcpy(fileName,"outputCarving.ppm");
      break;
    case 2: 
      strcpy(fileName,"outputStitching.ppm");
      break;
  }
  ppmFile.open(fileName,std::fstream::out);
  long int index=0;
  unsigned char* fileBuffer = new unsigned char[10000];
  writeHeader(fileBuffer, index);
  fileBuffer=writePixelBufferToFileBuffer(fileBuffer, index, 10000);
  writeToFile(fileBuffer, index, ppmFile);
  ppmFile.close();
}

void readAllPPMFiles(int option)
{
  char foregroundPPMFile[100], backgroundPPMFile[100], inputPPMFile[100];
  if(option==1)
  {
    cout<<"Enter the input ppm image\n";
    cin>>inputPPMFile;
    readPPMFile(inputPPMFile,1);
  }  
  else if (option == 2)
  {
    cout<<"Enter the foreground ppm image\n";
    cin>>foregroundPPMFile;
    cout<<"Enter the background ppm image\n";
    cin>>backgroundPPMFile;
    readPPMFile(backgroundPPMFile,1);
    readPPMFile(foregroundPPMFile,2);
  }
}

//************************************************************************************************************
//*************************************Compositing functions**************************************************
//************************************************************************************************************

double getHue(int x, int y)
{
  double red, green, blue;
  double max, min, delta;
  
  int hue;
  int val = (y * width + x)*3;

  red =  pixmapBackGround[val++]/255.0; 
  green = pixmapBackGround[val++]/255.0; 
  blue = pixmapBackGround[val]/255.0;

  max = maximum(red, green, blue);
  min = minimum(red, green, blue);

  if(max == 0)
    hue = 0;

  else
  {
    delta = max - min;
    if(delta == 0)
      hue = 0;
    else
    {
      if(red == max)                  
        hue = (green - blue) / delta;
      else if(green == max)
        hue = 2.0 + (blue - red) / delta;
      else /* (blue == max) */
        hue = 4.0 + (red - green) / delta;
      hue = hue * 60.0;
      if(hue < 0)
        hue = hue + 360.0;
    }
  }
  return hue;
}

double findLeastCostPath(int x, int* pathMatrix, int iVal)
{
  pathMatrix[0]=x;
  double currentHue = getHue(x,0);
  double sumMinimumHues=0;
  sumMinimumHues += currentHue;
  int currentXValue = x;
  for(int y=1;y<height;y++)
  {
    double hue1=DBL_MAX, hue2=DBL_MAX, hue3=DBL_MAX, minHueValue;
    hue1 = getHue(currentXValue,y);
    if(currentXValue>0)
      hue2 = getHue(currentXValue-1,y);
    if(currentXValue<width-1-iVal)
      hue3 = getHue(currentXValue+1,y);
    minHueValue = minimum(diff(currentHue,hue1), diff(currentHue,hue2), diff(currentHue,hue3));
    if(minHueValue == diff(currentHue,hue1))
    {
      sumMinimumHues+=diff(currentHue,hue1);
      pathMatrix[y]=currentXValue;
      currentHue = hue1;
    }
    else if(minHueValue == diff(currentHue,hue2))
    {
      sumMinimumHues+=diff(currentHue,hue2);
      pathMatrix[y]=currentXValue-1;
      currentHue = hue2;
    }
    else
    {
      sumMinimumHues+=diff(currentHue,hue3);
      pathMatrix[y]=currentXValue+1;
      currentHue = hue3;
    }
  }
  return sumMinimumHues;    
}

void applyShift(int* minPathSeam)
{
  for(int y=0; y<height;y++)
  {
    int x = minPathSeam[y];
    for(int xVal = x; xVal<width-1;xVal++)
    {
      int val1 = (y*width+xVal)*3;
      int val2 = (y*width+(xVal+1))*3;
      pixmapBackGround[val1++]=pixmapBackGround[val2++];
      pixmapBackGround[val1++]=pixmapBackGround[val2++];
      pixmapBackGround[val1]=pixmapBackGround[val2];
    }
    int lastColumn = (y*width+(width-1))*3;
    pixmapBackGround[lastColumn++]=0;      
    pixmapBackGround[lastColumn++]=0;
    pixmapBackGround[lastColumn]=0;
  }
}

void copySeam(int* minPathSeam, int* pathMatrix)
{
  for(int y=0;y<height;y++)
    minPathSeam[y]=pathMatrix[y];
}

void applyCarving(int iVal)
{
  double leastSum = DBL_MAX;
  int* pathMatrix = (int*) malloc (sizeof(int) * height);
  int* minPathSeam= (int*) malloc (sizeof(int) * height);
  for(int x=0;x<width-iVal;x++)
  {
    double currentSum = findLeastCostPath(x, pathMatrix, iVal);
    if(currentSum < leastSum)
    {
      leastSum = currentSum;
      copySeam(minPathSeam, pathMatrix);
    }
  }
  applyShift(minPathSeam);
  delete[] pathMatrix;
}

void copyResult(int outputWidth)
{
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<outputWidth;x++)
    {
      int val1 = (y*width+x)*3;
      int val2 = (y*outputWidth+x)*3;
      pixmapComputed[val2++]=pixmapBackGround[val1++];
      pixmapComputed[val2++]=pixmapBackGround[val1++];
      pixmapComputed[val2]=pixmapBackGround[val1];
    }
  }      
}

void applyRepeatedCarving(int outputWidth)
{
  for(int i=0;i<width-outputWidth;i++)
  {
    applyCarving(i);
  }
  pixmapComputed = new unsigned char[outputWidth * height * 3];
  copyResult(outputWidth);
  width = outputWidth;
}

void applyOperation(int option)
{
  int outputWidth;
  if(option == 1)
  {
    cout<<"Width of the original image is "<<width<<". Enter the width required in the carved image\n";
    cin>>outputWidth;
    applyRepeatedCarving(outputWidth);
  }
  //else
    //applyStitching();
}
// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{
  int option;
  cout<<"Enter options:\n1. Carving\n2. Stitching\n";
  cin>>option;

  readAllPPMFiles(option);

  applyOperation(option);

  generatePPMFile(option);


  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100); // Where the window will display on-screen.
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutCreateWindow("Homework Seven");
  init();
  glutReshapeFunc(windowResize);
  glutDisplayFunc(windowDisplay);
  glutMouseFunc(processMouse);
  glutMainLoop();

  return 0; //This line never gets reached. We use it because "main" is type int.
}
     

