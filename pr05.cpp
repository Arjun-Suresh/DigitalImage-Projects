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

#define KERNELNUMROWS 30
#define KERNELNUMCOLUMNS 30
#define MAXVECTORFIELDSIZE 3

#define KERNELNUMROWSMORPH 10
#define KERNELNUMCOLUMNSMORPH 10

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
    pixmapControl[val++]=fileBuffer[index++];
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
      strcpy(fileName,"outputMotionBlur.ppm");
      break;
    case 2: 
      strcpy(fileName,"outputDilationMorphological.ppm");
      break;
    case 3: 
      strcpy(fileName,"outputErosionMorphological.ppm");
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


//*****************************************************************************************************
//*************************************Stationary filter functions*************************************
//*****************************************************************************************************


void getColorVals(int x, int y, int& rVal, int& gVal, int& bVal)
{
  int val = (y*width + x)*3;
  rVal = pixmapControl[val++];
  gVal = pixmapControl[val++];
  bVal = pixmapControl[val];  
}

//*********************************Morphological erosion and dilation functions************************

bool insideCircle(int x, int y, int xCenter, int yCenter, double radius)
{
  double xVal = pow((x-xCenter),2);
  double yVal = pow((y-yCenter),2);
  double compute = xVal/pow(radius,2) + yVal/pow(radius,2);
  if(compute <= 1)
    return true;
  return false;
}

double getRadius(int rVal, int gVal, int xCenter, int yCenter)
{
  int xDiff = rVal - xCenter;
  int yDiff = gVal - yCenter;
  double h = pow ((pow(xDiff,2) + pow(yDiff,2)),0.5);
  return h;
}

void createMorphologicalKernelArray(int* kernelArray, int m, int n, int i, int j)
{
  int xCenter = n/2;
  int yCenter = m/2;
  int redVal, greenVal, blueVal;
  getColorVals(i, j, redVal, greenVal, blueVal);
  redVal = (redVal * (n-1))/255;
  greenVal = (greenVal * (m-1))/255;
  if(redVal == xCenter && greenVal == yCenter)
  {
    for(int y=0; y<m; y++)
    {
      for(int x=0; x<n; x++)
      {
        kernelArray[y*n+x]=0;
      }
    }
    kernelArray[yCenter*n+xCenter]=1;
  }
  else
  {
    double radius = getRadius(redVal, greenVal, xCenter, yCenter);
    for(int y=0; y<m; y++)
    {
      for(int x=0; x<n; x++)
      {
        if(insideCircle(x,y,xCenter,yCenter,radius))
          kernelArray[(m-y-1)*n+x]=1;
        else
          kernelArray[(m-y-1)*n+x]=0;
      }
    }
  }
}


int getMorphologicalValue(int i, int j, int colorOffset, int m, int n, int* kernelArray, int option)
{
  double maxKernelVal=0.0, minKernelVal=999.0;
  double* kernelArrayReal = new double[m*n];
  for(int y=0;y<m;y++)
  {
    for(int x=0;x<n;x++)
    {
      if(kernelArray[(y*n)+x])
        kernelArrayReal[(y*n)+x] = kernelArray[(y*n)+x];
      else
        kernelArrayReal[(y*n)+x] = 0.5;
    }
  }
  for(int y=0;y<m;y++)
  {
    for(int x=0;x<n;x++)
    {
      if(maxKernelVal<kernelArrayReal[(y*n)+x])
        maxKernelVal = kernelArrayReal[(y*n)+x];
      if(minKernelVal>kernelArrayReal[(y*n)+x])
        minKernelVal = kernelArrayReal[(y*n)+x];
    }
  }
  double maxValColor=0.0, minValColor=999.0;
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
      if(maxValColor<(kernelArrayReal[(y*n)+x]*(double)pixmapOrig[pixIndex]))
        maxValColor=(kernelArrayReal[(y*n)+x]*(double)pixmapOrig[pixIndex]);
      if(minValColor>(kernelArrayReal[(y*n)+x]*(double)pixmapOrig[pixIndex]))
        minValColor=(kernelArrayReal[(y*n)+x]*(double)pixmapOrig[pixIndex]);
    }
  }
  delete[] kernelArrayReal;
  if(option == 2)
    return (int)(maxValColor/maxKernelVal);
  else  
    return (int)(minValColor/minKernelVal);
}


void applyMorphologicalFilter(int* kernelArray, int m, int n, int option)
{
  int redNew, greenNew, blueNew;
  for(int j=0;j<height;j++)
  {
    for(int i=0;i<width;i++)
    {
      createMorphologicalKernelArray(kernelArray,m,n,i,j);
      redNew = getMorphologicalValue(i,j,REDOFFSET, m, n, kernelArray, option);
      greenNew = getMorphologicalValue(i,j,GREENOFFSET, m, n, kernelArray, option);
      blueNew = getMorphologicalValue(i,j,BLUEOFFSET, m, n, kernelArray, option);
      setPixelColor(j,i,redNew,greenNew,blueNew);
    }
  }            
}


//*****************************************Motion blur functions*******************
bool insideElipse(int x, int y, int xCenter, int yCenter, double theta, double aVal, double bVal)
{
  double xVal = pow(((x-xCenter)*cos(theta) + (y-yCenter)*sin(theta)),2);
  double yVal = pow(((x-xCenter)*sin(theta) - (y-yCenter)*cos(theta)),2);
  double compute = xVal/pow(aVal,2) + yVal/pow(bVal,2);
  if(compute <= 1)
    return true;
  return false;
}

double getAngle(int rVal, int gVal, int xCenter, int yCenter)
{
  int xDiff = rVal - xCenter;
  int yDiff = gVal - yCenter;
  double h = pow ((pow(xDiff,2) + pow(yDiff,2)),0.5);
  double sinVal = (double)yDiff/h;
  double cosVal = (double)xDiff/h;
  if (sinVal == 0)   // angle is 0 deg or 180 deg
  {
    if(cosVal == 1)
      return 0;
    else
      return 3.1416;
  }
  if (cosVal == 0)  //angle is 90 deg or 270 deg
  {
    if(sinVal == 1)
      return (3.1416/2);
    else
      return (1.5 * 3.1416);
  }
  double angle = atan((double)yDiff/(double)xDiff);
  if (sinVal > 0)
  {
    if(cosVal < 0)
      angle += 3.1416;
  }
  else
  {
    if(cosVal > 0)
      angle += (2 * 3.1416);
    else if (cosVal < 0)
      angle += 3.1416;
  }
  return angle;
}

double getMinorAxis(int blueVal, int max)
{
  double bVal = (double)(blueVal*max)/(double)255;
  if(bVal == 0)
    bVal += 0.1;
  if(bVal < 1)
    bVal = 1;

  return bVal;
}

void createMotionKernelArray(int* kernelArray, int m, int n, int i, int j)
{
  int xCenter = n/2;
  int yCenter = m/2;
  double aVal = n/2;
  int redVal, greenVal, blueVal;
  getColorVals(i, j, redVal, greenVal, blueVal);
  redVal = (redVal * (n-1))/255;
  greenVal = (greenVal * (m-1))/255;
  if(redVal == xCenter && greenVal == yCenter)
  {
    for(int y=0; y<m; y++)
    {
      for(int x=0; x<n; x++)
      {
        kernelArray[y*n+x]=0;
      }
    }
    kernelArray[yCenter*n+xCenter]=1;
  }
  else
  {
    double theta = getAngle(redVal, greenVal, xCenter, yCenter);
    double bVal = getMinorAxis(blueVal, MAXVECTORFIELDSIZE/2);
    for(int y=0; y<m; y++)
    {
      for(int x=0; x<n; x++)
      {
        if(insideElipse(x,y,xCenter,yCenter,theta,aVal,bVal))
          kernelArray[(m-y-1)*n+x]=1;
        else
          kernelArray[(m-y-1)*n+x]=0;
      }
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
      createMotionKernelArray(kernelArray,m,n,i,j);
      redNew = getBlurValue(i,j,REDOFFSET, m, n, kernelArray);
      greenNew = getBlurValue(i,j,GREENOFFSET, m, n, kernelArray);
      blueNew = getBlurValue(i,j,BLUEOFFSET, m, n, kernelArray);
      setPixelColor(j,i,redNew,greenNew,blueNew);
    }
  }
}


//********************************Main filter function**************************
void applyFilter(int option)
{
  int * kernelArray; 
  switch(option)
  {
    case 1:
      kernelArray = new int [KERNELNUMROWS*KERNELNUMCOLUMNS];
      applyBlurFilter(kernelArray, KERNELNUMROWS, KERNELNUMCOLUMNS);
      break;
    case 2:
    case 3:
      kernelArray = new int [KERNELNUMROWSMORPH*KERNELNUMCOLUMNSMORPH];
      applyMorphologicalFilter(kernelArray, KERNELNUMROWSMORPH, KERNELNUMCOLUMNSMORPH,option);
      break;
  }
}





// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{
  int option;
  cout<<"Enter options:\n1. Motion blur filter\n2. Morphological Dilation filter\n3. Morphological Erosion filter\n";
  cin>>option;
  char inputPPMFile[100], controlPPMFile[100];
  cout<<"Enter the ppm file to be manipulated\n";
  cin>>inputPPMFile;
  readPPMFile(inputPPMFile,1);

  cout<<"Enter the control ppm file\n";
  cin>>controlPPMFile;
  readPPMFile(controlPPMFile,2);
  pixmapComputed = new unsigned char[width * height * 3];

  applyFilter(option);
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
     

