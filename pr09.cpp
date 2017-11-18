// =============================================================================
// VIZA654/CSCE646 at Texas A&M University
// Homework 5
// Filters with non-stationary kernels
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

void writeHeader(unsigned char* fileBuffer, long int& index, int imageSize=0)
{
  maxColorValue=255;    
  char widthString[5], heightString[5], maxColorString[5];
  int w=width,h=height;
  if(imageSize)
    w=h=imageSize;
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


unsigned char* writePixelBufferToFileBuffer(unsigned char* fileBuffer, long int& index, long int origSize, unsigned char* pixmapFile = NULL, int imageSize=0)
{
  int charCount=0;
  int w=width,h=height;
  if(imageSize)
    w=h=imageSize;
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

void generatePPMFile(int option, unsigned char* pixmapFile = NULL, int imageSize = 0)
{
  std::fstream ppmFile;
  char fileName[50];
  switch(option)
  {
    case 1: 
      strcpy(fileName,"outputOrderedDithering.ppm");
      break;
    case 2: 
      strcpy(fileName,"outputErrorDiffusionDithering.ppm");
      break;
    case 3:
      strcpy(fileName,"kernelImage.ppm");
      break;
  }
  ppmFile.open(fileName,std::fstream::out);
  long int index=0;
  unsigned char* fileBuffer = new unsigned char[10000];
  writeHeader(fileBuffer, index, imageSize);
  fileBuffer=writePixelBufferToFileBuffer(fileBuffer, index, 10000, pixmapFile, imageSize);
  writeToFile(fileBuffer, index, ppmFile);
  ppmFile.close();
}


//*****************************************************************************************************
//*************************************Dithering functions*************************************
//*****************************************************************************************************

bool verifyResult(int xRes, int yRes, int kernelSize)
{
  if(xRes<kernelSize && xRes>=0 && yRes<kernelSize && yRes>=0)
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



void scaling(double xScale,double yScale, unsigned char *pixmapKernel, int kernelSize)
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
      if(verifyResult(xRes,yRes,kernelSize))
      {
        int input = (y * widthControl + x) * 3;
        int output = (yRes * kernelSize + xRes) * 3; 
        pixmapKernel[output++] = pixmapControl[input++];
        pixmapKernel[output++] = pixmapControl[input++];
        pixmapKernel[output] = pixmapControl[input];
      }
    }
  } 
}


void getProbabilities(unsigned char* pixmapKernel, double* probabilities, int kernelSize, int colorOffset)
{
  for(int k=0;k<256;k++)
    probabilities[k]=0;
  for(int j=0; j<kernelSize; j++)
  {
    for(int i=0; i<kernelSize; i++)
    {
      int val = ((j*kernelSize+i)*3)+colorOffset;
      probabilities[pixmapKernel[val]]++;
    }
  }
  for(int k=0;k<256;k++)
  {
    probabilities[k]=((double)(probabilities[k]))/(double)(kernelSize*kernelSize);
  }
}

double getCumulativeProbabilites(double* probabilities, int count)
{
  double sum=0.0;
  for(int i=0;i<=count;i++)
    sum+=probabilities[i];
  return sum;
}

void computeKernel(int* kernel, int kernelSize, unsigned char *pixmapKernel)
{
  double redProbabilities[256], greenProbabilities[256], blueProbabilities[256];
  getProbabilities(pixmapKernel, redProbabilities, kernelSize, 0);
  getProbabilities(pixmapKernel, greenProbabilities, kernelSize, 1);
  getProbabilities(pixmapKernel, blueProbabilities, kernelSize, 2);
  for(int j=0; j<kernelSize; j++)
  {
    for(int i=0; i<kernelSize; i++)
    {
      int val = ((j*kernelSize+i)*3);
      double redColorVal = getCumulativeProbabilites(redProbabilities, pixmapKernel[val++]);
      double greenColorVal = getCumulativeProbabilites(greenProbabilities, pixmapKernel[val++]);
      double blueColorVal = getCumulativeProbabilites(blueProbabilities, pixmapKernel[val]);      
      kernel[j*kernelSize+i]= (int)(255.0*((redColorVal+greenColorVal+blueColorVal)/3.0));
    }
  }  
}

void applyKernel(int* kernel, int kernelSize)
{
  for(int y=0; y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int val = (y*width+x)*3;
      int redVal = pixmapOrig[val++]-kernel[(y%kernelSize)*kernelSize+(x%kernelSize)];
      int greenVal = pixmapOrig[val++]-kernel[(y%kernelSize)*kernelSize+(x%kernelSize)];
      int blueVal = pixmapOrig[val]-kernel[(y%kernelSize)*kernelSize+(x%kernelSize)];
      if(redVal<=0)
        redVal=0;
      else
        redVal=255;
      if(greenVal<=0)
        greenVal=0;
      else
        greenVal=255;
      if(blueVal<=0)
        blueVal=0;
      else
        blueVal=255;
      val = (y*width+x)*3;
      pixmapComputed[val++]=redVal;
      pixmapComputed[val++]=greenVal;
      pixmapComputed[val]=blueVal;    
    }
  }  
}

void orderedDither(int kernelSize)
{
  unsigned char *pixmapKernel = new unsigned char[kernelSize*kernelSize*3];
  double xScale = (double)kernelSize/(double)widthControl;
  double yScale = (double)kernelSize/(double)heightControl;
  scaling(xScale, yScale, pixmapKernel, kernelSize);
  generatePPMFile(3, pixmapKernel,kernelSize);
  int* kernel = new int[kernelSize*kernelSize];
  computeKernel(kernel, kernelSize, pixmapKernel);
  applyKernel(kernel, kernelSize);
}

void initPixelsMatrix(int** pixelsMatrix, int colorOffset)
{
  for(int y=0;y<height; y++)
  {
    for(int x=0;x<width;x++)
    {
      int val = ((y*width+x)*3)+colorOffset;
      pixelsMatrix[y][x] = pixmapOrig[val];
    }
  }
}

void writeBack(int** pixelsMatrix, int colorOffset)
{
  for(int y=0;y<height; y++)
  {
    for(int x=0;x<width;x++)
    {
      int val = ((y*width+x)*3)+colorOffset;
      pixmapComputed[val] = pixelsMatrix[y][x];
    }
  }
}

void applyFloydSteinbergErrorDiffusion()
{
  int **pixelsMatrix = (int **)malloc(height * sizeof(int *));
  for (int i=0; i<height; i++)
    pixelsMatrix[i] = (int *)malloc(width * sizeof(int));
  for(int color=0;color<3;color++)
  {
    initPixelsMatrix(pixelsMatrix, color);
    for(int y=0;y<height; y++)
    {
      for(int x=0;x<width;x++)
      {
        int val = ((y*width+x)*3)+color;
        int oldColorValue = pixelsMatrix[y][x];
        int newColorValue;
        if(oldColorValue<0)
          newColorValue=0;
        else if(oldColorValue>255)
          newColorValue=255;
        else
          newColorValue = (oldColorValue/128)*255;
        pixelsMatrix[y][x] = newColorValue;/*
        double quant_error = oldColorValue - newColorValue;
        if(x<width-1)
          pixelsMatrix[y][x+1] = pixelsMatrix[y][x+1] +(int)(quant_error * 7.0/16.0);
        if(x>0 && y<height-1)
          pixelsMatrix[y+1][x-1] = pixelsMatrix[y+1][x-1] + (int)(quant_error * 3.0/16.0);
        if(y<height-1)
          pixelsMatrix[y+1][x] = pixelsMatrix[y+1][x] + (int)(quant_error * 5.0/16.0);
        if(y<height-1 && x<width-1)
        pixelsMatrix[y+1][x+1] = pixelsMatrix[y+1][x+1] + (int)(quant_error * 1.0/16.0);*/
      }
    }
    writeBack(pixelsMatrix,color);
  }  
}

void applyDithering(int option)
{
  switch(option)
  {
    case 1:
    char kernelFile[100];
    int kernelSize;
    cout<<"Enter the image to be used for kernel\n";
    cin>>kernelFile;
    cout<<"Enter the kernel size (N) of the N X N kernel\n";
    cin>>kernelSize;
    readPPMFile(kernelFile,2);
    orderedDither(kernelSize);
    break;
    case 2:
    applyFloydSteinbergErrorDiffusion();
    break;
  }
}

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{
  char inputPPMFile[100], controlPPMFile[100];
  cout<<"Enter the ppm file to be manipulated\n";
  cin>>inputPPMFile;
  readPPMFile(inputPPMFile,1);
  pixmapComputed = new unsigned char[width * height * 3];

  int option;
  cout<<"Enter options:\n1. Ordered dithering\n2. Error Diffusion dithering\n";
  cin>>option;

  applyDithering(option);

  generatePPMFile(option);


  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100); // Where the window will display on-screen.
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutCreateWindow("Homework Four");
  init();
  glutReshapeFunc(windowResize);
  glutDisplayFunc(windowDisplay);
  glutMouseFunc(processMouse);
  glutMainLoop();

  return 0; //This line never gets reached. We use it because "main" is type int.
}
     

