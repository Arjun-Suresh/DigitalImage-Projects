// =============================================================================
// VIZA654/CSCE646 at Texas A&M University
// Homework 4
// Basic filters with stationary kernels
// For color manipulation, input is taken from txt file on the same folder
// Data format in the txt file:
// r0 r1 r2 r3 .... rn
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

#define REDOFFSET 2
#define GREENOFFSET 1
#define BLUEOFFSET 0


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

void setPixelColorOrig(long int &val, long int& index, unsigned char* fileBuffer)
{
  pixmapOrig[val++]=fileBuffer[index++];
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

bool fillPixelsBin(long int& index, unsigned char* fileBuffer, long int numOfCharacters)
{
  int wVal,hVal;
  wVal=width;
  hVal=height;
  long int rowVal=0,colVal=0;
  while(rowVal<hVal)
  {
    long int val=(((hVal-rowVal-1)*wVal)+colVal)*3;
    for(int i=0;i<3;i++)
    	setPixelColorOrig(val, index, fileBuffer);
    colVal=(colVal+1)%wVal;
    if(!colVal)
      rowVal++;
  }
  return true;
}

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
      pixmapOrig = new unsigned char[width*height*3];
      if(!(fillPixelsBin(index, fileBuffer, numOfCharacters)))
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
      strcpy(fileName,"outputBlur.ppm");
      break;
    case 2: 
      strcpy(fileName,"outputDerivative.ppm");
      break;
    case 3: 
      strcpy(fileName,"outputDilationMorphological.ppm");
      break;
    case 4: 
      strcpy(fileName,"outputErosionMorphological.ppm");
      break;
    case 5: 
      strcpy(fileName,"outputMotionBlur.ppm");
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





//***********************************************************************************************
//****************************Read data from txt file to get the curve points********************
//***********************************************************************************************
int* readData(char* filePath, int& m, int& n)
{
  std::fstream inputFile;
  int x,y,count=0;
  char c, cArray[10];
  int k=0;  
  cout<<"Reading data from "<<filePath<<"\n";
  inputFile.open(filePath, std::fstream::in);
  while(inputFile.get(c))
  {
    if(c == ' ')
    {
      cArray[k]='\0';
      k=0;
      string num(cArray);
      istringstream(num) >> n;
      break;
    }
    else
      cArray[k++]=c;
  }
  
  while(inputFile.get(c))
  {
    if(c == '\n')
    {
      cArray[k]='\0';
      k=0;
      string num(cArray);
      istringstream(num) >> m;
      break;
    }
    else
      cArray[k++]=c;
  }
  int* kernel = new int[m*n];
  while(inputFile.get(c))
  {
    if(c == ' ' || c == '\n')
    {
      if(k!=0)
      {
        cArray[k]='\0';
        k=0;
        string num(cArray);
        istringstream(num) >> x;
        kernel[count++]=x;
      }
    }
    else
      cArray[k++]=c;
  }
  return kernel;
}





//*****************************************************************************************************
//*************************************Stationary filter functions******************************
//*****************************************************************************************************

void processKernelArray(int* kernelArray, double* processKernel, int m, int n)
{
  double avgVal=0, normalizeAvg=0;
  for(int y=0;y<m;y++)
    for(int x=0;x<n;x++)
      avgVal+=kernelArray[y*n+x];
  avgVal = avgVal/(double)(m*n);
  for(int y=0;y<m;y++)
  {
    for(int x=0;x<n;x++)
    {
      processKernel[y*n+x]=(double)kernelArray[y*n+x]-avgVal;
      normalizeAvg+=mod(processKernel[y*n+x]);
    }
  }
  normalizeAvg = normalizeAvg/2.0;
  for(int y=0;y<m;y++)
    for(int x=0;x<n;x++)
      processKernel[y*n+x] = processKernel[y*n+x]/normalizeAvg;
}

int getDerivedValue(int i, int j, int colorOffset, int m, int n, double* processKernel)
{
  double newColorValue=0;
  int maxColorInWindow = 0;
  for(int y=0;y<m;y++)
  {
    for(int x=0;x<n;x++)
    {
      int rowVal=(i-(n/2)+x);
      int colVal=(j-(m/2)+y);
      if(rowVal<0)
        rowVal+=width;
      if(colVal<0)
        colVal+=height;
      //Boundary conditions
      rowVal = rowVal % width;
      colVal = colVal % height;
      int pixIndex = ((colVal*width+rowVal)*3)+colorOffset;
      newColorValue += (processKernel[y*n+x] * (double)pixmapOrig[pixIndex]);
      if(maxColorInWindow < pixmapOrig[pixIndex])
        maxColorInWindow = pixmapOrig[pixIndex];
     }
  }
  if(!maxColorInWindow)
    maxColorInWindow=1;
  newColorValue = (newColorValue+maxColorInWindow)/(double)(2*maxColorInWindow);
  if(newColorValue<=0.6)
    return 0;
  else
    return 255;
}

void applyDerivativeFilter(int* kernelArray, int m, int n)
{
  double* processKernel = new double[m*n];
  int redNew, greenNew, blueNew;
  processKernelArray(kernelArray, processKernel, m, n);
  for(int j=0;j<height;j++)
  {
    for(int i=0;i<width;i++)
    {
      redNew = getDerivedValue(i,j,REDOFFSET, m, n, processKernel);
      greenNew = getDerivedValue(i,j,GREENOFFSET, m, n, processKernel);
      blueNew = getDerivedValue(i,j,BLUEOFFSET, m, n, processKernel);
      if(redNew || greenNew || blueNew)
        setPixelColor(j,i,255,255,255);
      else
        setPixelColor(j,i,0,0,0);
    }
  }      
}


int getMorphologicalValue(int i, int j, int colorOffset, int m, int n, int* kernelArray, int option)
{
  int maxKernelVal=0, minKernelVal=999;
  for(int y=0;y<m;y++)
  {
    for(int x=0;x<n;x++)
    {
      if(maxKernelVal<kernelArray[(y*n)+x])
        maxKernelVal = kernelArray[(y*n)+x];
      if(minKernelVal>kernelArray[(y*n)+x])
        minKernelVal = kernelArray[(y*n)+x];
    }
  }
  int maxValColor=0, minValColor=999;
  for(int y=0;y<m;y++)
  {
    for(int x=0;x<n;x++)
    {
      int rowVal=(i-(n/2)+x);
      int colVal=(j-(m/2)+y);
      if(rowVal<0)
        rowVal+=width;
      if(colVal<0)
        colVal+=height;
      //Boundary conditions
      rowVal = rowVal % width;
      colVal = colVal % height;
      int pixIndex = ((colVal*width+rowVal)*3)+colorOffset;
      if(maxValColor<(kernelArray[(y*n)+x]*pixmapOrig[pixIndex]))
        maxValColor=(kernelArray[(y*n)+x]*pixmapOrig[pixIndex]);
      if(minValColor>(kernelArray[(y*n)+x]*pixmapOrig[pixIndex]))
        minValColor=(kernelArray[(y*n)+x]*pixmapOrig[pixIndex]);
    }
  }
  if(option == 3)
    return maxValColor/maxKernelVal;
  else
  {
    if(minKernelVal == 0)
      return 0;
    return minValColor/minKernelVal;
  }
}


void applyMorphologicalFilter(int* kernelArray, int m, int n, int option)
{
  int redNew, greenNew, blueNew;
  for(int j=0;j<height;j++)
  {
    for(int i=0;i<width;i++)
    {
      redNew = getMorphologicalValue(i,j,REDOFFSET, m, n, kernelArray, option);
      greenNew = getMorphologicalValue(i,j,GREENOFFSET, m, n, kernelArray, option);
      blueNew = getMorphologicalValue(i,j,BLUEOFFSET, m, n, kernelArray, option);
      setPixelColor(j,i,blueNew,greenNew,redNew);
    }
  }            
}


int getBlurValue(int i, int j, int colorOffset, int m, int n, int* kernelArray)
{
  int output=0,sumKernel=0;
  for(int y=0;y<m;y++)
  {
    for(int x=0;x<n;x++)
    {
      int rowVal=(i-(n/2)+x);
      int colVal=(j-(m/2)+y);
      if(rowVal<0)
        rowVal+=width;
      if(colVal<0)
        colVal+=height;
      //Boundary conditions
      rowVal = rowVal % width;
      colVal = colVal % height;
      int pixIndex = ((colVal*width+rowVal)*3)+colorOffset;
      output+=(kernelArray[(y*n)+x]*pixmapOrig[pixIndex]);
      sumKernel+=kernelArray[(y*n)+x];
    }
  }
  return output/sumKernel;
}


void applyBlurFilter(int* kernelArray, int m, int n)
{
  int redNew, greenNew, blueNew;
  for(int j=0;j<height;j++)
  {
    for(int i=0;i<width;i++)
    {
      redNew = getBlurValue(i,j,REDOFFSET, m, n, kernelArray);
      greenNew = getBlurValue(i,j,GREENOFFSET, m, n, kernelArray);
      blueNew = getBlurValue(i,j,BLUEOFFSET, m, n, kernelArray);
      setPixelColor(j,i,blueNew,greenNew,redNew);
    }
  }
}

void applyFilter(int* kernelArray, int m, int n, int option)
{
  switch(option)
  {
    case 1:
    case 5:
      applyBlurFilter(kernelArray, m, n);
      break;
    case 2:
      applyDerivativeFilter(kernelArray, m, n);
      break;
    case 3:
    case 4:
      applyMorphologicalFilter(kernelArray, m, n,option);
      break;
  }
}

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{
  int option;
  cout<<"Enter options:\n1. Blur filter\n2. Derivative filter\n3. Morphological Dilation filter\n4. Morphological Erosion filter\n5. Motion Blur\n";
  cin>>option;
  int m=0,n=0, *kernelArray;
  std::fstream inputFile;
  int rArray[1000];
  char filePath[100];
  switch(option)
  {
    case 1:
      strcpy(filePath, "inputBlur.txt");
      break;
    case 2:
      strcpy(filePath, "inputDerivative.txt");
      break;
    case 3:
      strcpy(filePath, "inputDilation.txt");
      break;
    case 4:
      strcpy(filePath, "inputErosion.txt");
      break;
    case 5:
      strcpy(filePath, "inputMotion.txt");
      break;
  }
    
  kernelArray=readData(filePath, m, n);
  char inputPPMFile[100];
  cout<<"Enter the ppm file to be manipulated\n";
  cin>>inputPPMFile;
  readPPMFile(inputPPMFile);
  pixmapComputed = new unsigned char[width * height * 3];
  applyFilter(kernelArray,m,n,option);
  
  generatePPMFile(option);


  glutInit(&argc, argv);
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
     

