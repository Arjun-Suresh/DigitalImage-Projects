// =============================================================================
// VIZA654/CSCE646 at Texas A&M University
// Homework 10
// Stitching and carving
// 1. Carve an aimage from original width to the input width by repeatedly identifying the least impact seams.
// 2. Stitch input image 1 on the left with input image 2 on the right with a seam having least difference between the 2 images
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
int width, height, maxColorValue, magicNo,widthControl, heightControl;
unsigned char *pixmapOrig, *pixmapControl, *pixmapComputed;

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
    pixmapOrig[val++]=fileBuffer[index++];
  else
  {
    pixmapControl[val++]=fileBuffer[index++];
  }
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
      if (option == 1)
        width=value;
      else
        widthControl=value;
      break;
    case HEIGHTVALUEID:
      if (option == 1)
        height=value;
      else
        heightControl = value;
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
  if (option == 1)
  {
    wVal=width;
    hVal=height;
  }
  else
  {
    wVal=widthControl;
    hVal=heightControl;
  }
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
        pixmapOrig = new unsigned char[width*height*3];
      else
        pixmapControl = new unsigned char[widthControl*heightControl*3];
      if(!(fillPixelsBin(index, fileBuffer, numOfCharacters, option)))
        return false;
      delete[] fileBuffer;
      return true;
    }
  }
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
    cout<<"Enter the first ppm image\n";
    cin>>backgroundPPMFile;
    cout<<"Enter the second ppm image\n";
    cin>>foregroundPPMFile;
    readPPMFile(backgroundPPMFile,1);
    readPPMFile(foregroundPPMFile,2);
  }
}

//************************************************************************************************************
//**********************Functions to save from pixelMap to ppm file*******************************************
//************************************************************************************************************

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

void writeHeader(unsigned char* fileBuffer, long int& index)
{
  maxColorValue=255;    
  char widthString[5], heightString[5], maxColorString[5];
  int w=width,h=height;
  sprintf(widthString, "%d", w);
  sprintf(heightString, "%d", h);
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


unsigned char* writePixelBufferToFileBuffer(unsigned char* fileBuffer, long int& index, long int origSize, unsigned char* pixmapFile = NULL)
{
  int charCount=0;
  int w=width,h=height;
  for(int i=0;i<h;i++)
  {
    for(int j=0;j<w;j++)
    {
      if(index >= origSize-50)
      {
        unsigned char* tempBuffer = resizeArray(fileBuffer, origSize, origSize);
        delete[] fileBuffer;
        fileBuffer=tempBuffer;
      }
      int k=((h-i-1)*w+j)*3;
      for(int x=0;x<3;x++)
      if(!pixmapFile)
        fileBuffer[index++]=pixmapComputed[k++];
      else
        fileBuffer[index++]=pixmapFile[k++];
    }
  }
  return fileBuffer;
}

void generatePPMFile(int option, unsigned char* pixmapFile = NULL)
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
    case 3:
      strcpy(fileName,"ScaledImage2.ppm");
      break;
  }
  ppmFile.open(fileName,std::fstream::out);
  long int index=0;
  unsigned char* fileBuffer = new unsigned char[10000];
  writeHeader(fileBuffer, index);
  fileBuffer=writePixelBufferToFileBuffer(fileBuffer, index, 10000, pixmapFile);
  writeToFile(fileBuffer, index, ppmFile);
  ppmFile.close();
}

//************************************************************************************************************
//*************************************Stitching and carving functions**************************************************
//************************************************************************************************************
//Functions used for scaling the image 2 to be stitched to the size of image 1
bool verifyResult(int xRes, int yRes)
{
  if(xRes<width && xRes>=0 && yRes<height && yRes>=0)
    return true;
  return false;
}

void initMatrix(double* pixelMatrix, int x, int y)
{
  pixelMatrix[0]=x;
  pixelMatrix[1]=y;
  pixelMatrix[2]=1;
}

void getValues(double* resultMatrix, int& x, int& y)
{
  x=(int)(resultMatrix[0]+0.5);
  y=(int)(resultMatrix[1]+0.5);
}

void multiplyMatrix(double* pixelMatrix, double transformation[][3], double* resultMatrix)
{
  for(int i=0;i<3;i++)
  {
    resultMatrix[i]=0;
    for(int j=0;j<3;j++)
    {
      resultMatrix[i]+=(transformation[i][j]*pixelMatrix[j]);
    }
  }
  for(int i=0;i<2;i++)
    resultMatrix[i] = resultMatrix[i]/resultMatrix[2];
}

void antiAliaseScaling(double xScale, double yScale, unsigned char *pixmapKernel)
{
  double scalingMatrix[3][3];
  double pixelMatrix[3],resultMatrix[3];
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
        scalingMatrix[i][j]=0;      
    }
  }

  scalingMatrix[2][2]=1;
  scalingMatrix[0][0]=1/(double)xScale;
  scalingMatrix[1][1]=1/(double)yScale;
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int input = (y * width + x) * 3;
      if(pixmapKernel[input] == 0 && pixmapKernel[input+1] == 0 && pixmapKernel[input+2] == 0)
      {
        initMatrix(pixelMatrix,x,y);
        multiplyMatrix(pixelMatrix, scalingMatrix, resultMatrix);
        int xRes, yRes;
        getValues(resultMatrix,xRes,yRes);
        if(xRes<widthControl && xRes>=0 && yRes<heightControl && yRes>=0)
        { 
          int output = (yRes * widthControl + xRes) * 3; 
          pixmapKernel[input++] = pixmapControl[output++];
          pixmapKernel[input++] = pixmapControl[output++];
          pixmapKernel[input] = pixmapControl[output];
        }
      }
    }
  }
}

void scaling(double xScale,double yScale, unsigned char *pixmapKernel)
{
  double scalingMatrix[3][3];
  double pixelMatrix[3],resultMatrix[3];
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
        scalingMatrix[i][j]=0;      
    }
  }

  scalingMatrix[2][2]=1;
  scalingMatrix[0][0]=xScale;
  scalingMatrix[1][1]=yScale;

  for(int y=0;y<heightControl;y++)
  {
    for(int x=0;x<widthControl;x++)
    {
      initMatrix(pixelMatrix,x,y);
      multiplyMatrix(pixelMatrix, scalingMatrix, resultMatrix);
      int xRes, yRes;
      getValues(resultMatrix,xRes,yRes);
      if(verifyResult(xRes,yRes))
      {
        int input = (y * widthControl + x) * 3;
        int output = (yRes * width + xRes) * 3; 
        pixmapKernel[output++] = pixmapControl[input++];
        pixmapKernel[output++] = pixmapControl[input++];
        pixmapKernel[output] = pixmapControl[input];
      }
    }
  } 
  antiAliaseScaling(xScale, yScale, pixmapKernel);
}





//Hue is being used as the parameter whiich is checked for impact
//Using the algorithm provided by the professor as template
double getHue(int x, int y, unsigned char* pixmapFile=NULL)
{
  double red, green, blue;
  double max, min, delta;
  
  int hue;
  int val = (y * width + x)*3;

  if(!pixmapFile)
  {
    red =  pixmapOrig[val++]/255.0; 
    green = pixmapOrig[val++]/255.0; 
    blue = pixmapOrig[val]/255.0;
  }
  
  else
  {
    red =  pixmapFile[val++]/255.0; 
    green = pixmapFile[val++]/255.0; 
    blue = pixmapFile[val]/255.0;
  }

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


//Carving functions
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
      currentXValue-=1;
      pathMatrix[y]=currentXValue;
      currentHue = hue2;
    }
    else
    {
      sumMinimumHues+=diff(currentHue,hue3);
      currentXValue+=1;
      pathMatrix[y]=currentXValue;
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
      pixmapOrig[val1++]=pixmapOrig[val2++];
      pixmapOrig[val1++]=pixmapOrig[val2++];
      pixmapOrig[val1]=pixmapOrig[val2];
    }
    int lastColumn = (y*width+(width-1))*3;
    pixmapOrig[lastColumn++]=0;      
    pixmapOrig[lastColumn++]=0;
    pixmapOrig[lastColumn]=0;
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
  delete[] minPathSeam;
}

void copyResult(int outputWidth)
{
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<outputWidth;x++)
    {
      int val1 = (y*width+x)*3;
      int val2 = (y*outputWidth+x)*3;
      pixmapComputed[val2++]=pixmapOrig[val1++];
      pixmapComputed[val2++]=pixmapOrig[val1++];
      pixmapComputed[val2]=pixmapOrig[val1];
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


//Stitching functions
double findLeastCostPathStitching(int x, int* pathMatrix, unsigned char* pixmapFile)
{
  pathMatrix[0]=x;
  double sumMinimumHues=0;
  sumMinimumHues += diff(getHue(x,0),getHue(x,0,pixmapFile));
  int currentXValue = x;
  for(int y=1;y<height;y++)
  {
    double hue1=DBL_MAX, hue2=DBL_MAX, hue3=DBL_MAX, minHueValue;
    double hue11=DBL_MIN, hue22=DBL_MIN, hue33=DBL_MIN;
    hue1 = getHue(currentXValue,y);
    if(currentXValue>0)
      hue2 = getHue(currentXValue-1,y);
    if(currentXValue<width-1)
      hue3 = getHue(currentXValue+1,y);
    hue11 = getHue(currentXValue,y,pixmapFile);
    if(currentXValue>0)
      hue22 = getHue(currentXValue-1,y,pixmapFile);
    if(currentXValue<width-1)
      hue33 = getHue(currentXValue+1,y,pixmapFile);
    double diffhue1 = diff(hue1,hue11);
    double diffhue2 = diff(hue2,hue22);
    double diffhue3 = diff(hue3,hue33);

    minHueValue = minimum(diffhue1, diffhue2, diffhue3);
    if(minHueValue == diffhue1)
    {
      sumMinimumHues+=diffhue1;
      pathMatrix[y]=currentXValue;
    }
    else if(minHueValue == diffhue2)
    {
      sumMinimumHues+=diffhue2;
      currentXValue-=1;
      pathMatrix[y]=currentXValue;
    }
    else
    {
      sumMinimumHues+=diffhue3;
      currentXValue+=1;
      pathMatrix[y]=currentXValue;
    }
  }
  return sumMinimumHues;    
}

void stitch(int* minPathSeam,  unsigned char* pixmapFile)
{
  for(int y=0; y<height;y++)
  {
    int x = minPathSeam[y];
    
    for(int xVal = 0; xVal<x;xVal++)
    {
      int val1 = (y*width+xVal)*3;
      pixmapComputed[val1]=pixmapOrig[val1];
      pixmapComputed[val1+1]=pixmapOrig[val1+1];
      pixmapComputed[val1+2]=pixmapOrig[val1+2];
    }
    
    for(int xVal = x; xVal<width;xVal++)
    {
      int val1 = (y*width+xVal)*3;
      pixmapComputed[val1]=pixmapFile[val1];
      pixmapComputed[val1+1]=pixmapFile[val1+1];
      pixmapComputed[val1+2]=pixmapFile[val1+2];
    }
  }
}

void applyStitching()
{
  double leastSum = DBL_MAX;
  unsigned char *pixmap = new unsigned char[width*height*3];
  double xScale = (double)width/(double)widthControl;
  double yScale = (double)height/(double)heightControl;
  scaling(xScale, yScale, pixmap);
  generatePPMFile(3, pixmap);
  int* pathMatrix = (int*) malloc (sizeof(int) * height);
  int* minPathSeam= (int*) malloc (sizeof(int) * height);
  for(int x=1;x<width-1;x++)
  {
    double currentSum = findLeastCostPathStitching(x, pathMatrix, pixmap);
    if(currentSum < leastSum)
    {
      leastSum = currentSum;
      copySeam(minPathSeam, pathMatrix);
    }
  }
  pixmapComputed = new unsigned char[width * height * 3];
  stitch(minPathSeam, pixmap);
  delete[] pathMatrix;
  delete[] minPathSeam;
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
  else
    applyStitching();
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
     

