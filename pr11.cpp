// =============================================================================
// VIZA654/CSCE646 at Texas A&M University
// Homework 7
// Warp transformations - Bilinear warp, Inverse warp using complex numbers and sine warp
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

#define MAXDEPTH 300

#define REFLECTMAX 0.866

#define ZL MAXDEPTH

using namespace std;
// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height, maxColorValue, magicNo, option;
unsigned char *pixmapOrig, *pixmapComputed;

class unitVector
{
  private:
    double x;
    double y;
    double z;
  public:
    unitVector(double xVal, double yVal, double zVal)
    {
      double length = pow((pow(xVal,2)+pow(yVal,2)+pow(zVal,2)),0.5);
      x=(double)xVal/length; y=(double)yVal/length; z=(double)zVal/length;
    }
    double getXValue()
    {
      return x;
    }
    double getYValue()
    {
      return y;
    }
    double getZValue()
    {
      return z;
    }
    double dotProduct(unitVector& vector2)
    {
      return (x*vector2.x + y*vector2.y + z*vector2.z);
    }
      
};

void diffuseIlluminate(int x, int y);
void generatePPMFile();
unitVector** normalVector;
double* zValues;
int XL=100, YL=100;

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
  {
    generatePPMFile();
    exit(0); 
  }
}

static void movementMouse(int x, int y)
{
  XL=x;
  YL=height-y;
  glutPostRedisplay();
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
void emboss(int x, int y, double& xGrad, double& yGrad)
{
  int iterator1, iterator2, iterator3, iterator4;
  if(x == 0)
    x+=width;
  if(y == 0)
    y+=height;
  iterator1 = ((y*width)+((x+1)%width))*3;
  iterator2 = ((y*width)+((x-1)%width))*3;
  iterator3 = ((((y+1)%height)*width)+x)*3;
  iterator4 = ((((y-1)%height)*width)+x)*3;
  double colorVal1 = (pixmapOrig[iterator1]+pixmapOrig[iterator1+1]+pixmapOrig[iterator1+2])/3;
  double colorVal2 = (pixmapOrig[iterator2]+pixmapOrig[iterator2+1]+pixmapOrig[iterator2+2])/3;
  double colorVal3 = (pixmapOrig[iterator3]+pixmapOrig[iterator3+1]+pixmapOrig[iterator3+2])/3;
  double colorVal4 = (pixmapOrig[iterator4]+pixmapOrig[iterator4+1]+pixmapOrig[iterator4+2])/3;
  xGrad = (colorVal1-colorVal2)/255;
  yGrad = (colorVal3-colorVal4)/255;
}

void fillNormalAndHeights()
{
  normalVector = (unitVector **) malloc(sizeof(unitVector*)*width*height);
  zValues = new double[width*height];
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int iterator = ((y*width)+x)*3;
      double colorVal = ((double)(pixmapOrig[iterator]+pixmapOrig[iterator+1]+pixmapOrig[iterator+2]))/(3*255);
      zValues[(y*width)+x]=colorVal * MAXDEPTH;
      double xGrad, yGrad;
      emboss(x,y,xGrad,yGrad);
      normalVector[(y*width)+x] = new unitVector(xGrad, yGrad, -1);
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

void generatePPMFile()
{
  std::fstream ppmFile;
  char fileName[50];
  switch(option)
  {
    case 1: 
      strcpy(fileName,"outputDiffuse.ppm");
      break;
    case 2:
      strcpy(fileName,"outputReflection.ppm");
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
//*************************************Warp functions**************************************************
//*****************************************************************************************************

void diffuseIlluminate()
{
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int iterator = ((y*width)+x)*3;
      unitVector lightVector((double)(x-XL), (double)(y-YL), zValues[(y*width)+x]-ZL);  
      double cosTheta = (normalVector[(y*width)+x])->dotProduct(lightVector); 
      int colorVal;
      if(cosTheta<=0)
        colorVal=0;
      else
        colorVal=cosTheta*255;
      pixmapComputed[iterator]=colorVal;
      pixmapComputed[iterator+1]=colorVal;
      pixmapComputed[iterator+2]=colorVal;
    }
  }
}

void specularReflect()
{
  double XI = (double)width/2;
  double YI = (double)height/2;
  double ZI = MAXDEPTH;
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int iterator = ((y*width)+x)*3;
      unitVector lightVector((double)(x-XL), (double)(y-YL), zValues[(y*width)+x]-ZL); 
      unitVector eyeVector((double)(x-XI), (double)(y-YI), zValues[(y*width)+x]-ZI);
      double prod1 = ((normalVector[(y*width)+x])->dotProduct(eyeVector))*2;
      unitVector reflection(((normalVector[(y*width)+x]->getXValue())*prod1)-eyeVector.getXValue(),((normalVector[(y*width)+x]->getYValue())*prod1)-eyeVector.getYValue(),((normalVector[(y*width)+x]->getZValue())*prod1)-eyeVector.getXValue());
      double cosTheta = reflection.dotProduct(lightVector);
      int colorVal1, colorVal2, colorVal;
      if(cosTheta>=REFLECTMAX)
        colorVal1=cosTheta*255;
      else
        colorVal1=0;
      double cosTheta1 = (normalVector[(y*width)+x])->dotProduct(lightVector); 
      if(cosTheta1<=0)
        colorVal2=0;
      else
        colorVal2=cosTheta1*255;
      
      colorVal=colorVal1+colorVal2;
      if(colorVal>255)
        colorVal=255;
      //colorVal=colorVal1;
      pixmapComputed[iterator]=colorVal;
      pixmapComputed[iterator+1]=colorVal;
      pixmapComputed[iterator+2]=colorVal;
    }
  }
}

void applyOperation()
{
  switch(option)
  {
    case 1:
      diffuseIlluminate();      
      windowDisplay();
      break;
    case 2:
      specularReflect();
      windowDisplay();
      break;
  }
}

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{
  char inputPPMFile[100];
  cout<<"Enter the height field ppm file\n";
  cin>>inputPPMFile;
  readPPMFile(inputPPMFile);
  fillNormalAndHeights();
  pixmapComputed = new unsigned char[width * height * 3];
  cout<<"Enter:\n1. Diffuse illumination\n2. Diffuse illumination + Specular reflection\n";
  cin>>option;
  
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100); // Where the window will display on-screen.
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutCreateWindow("Homework Seven");
  init();
  glutReshapeFunc(windowResize);
  glutDisplayFunc(applyOperation);
  glutMouseFunc(processMouse);
  glutPassiveMotionFunc(movementMouse);
  glutMainLoop();
  return 0; //This line never gets reached. We use it because "main" is type int.
}
     

