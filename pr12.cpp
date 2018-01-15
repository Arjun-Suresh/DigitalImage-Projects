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

#define MAXDEPTH 1500

using namespace std;
// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height, widthBack, heightBack, maxColorValue, magicNo, option;
unsigned char *pixmapOrig, *pixmapComputed, *pixmapBackground;

void generatePPMFile();
class unitVector
{
  private:
    double x;
    double y;
    double z;
  public:
    unitVector(double xVal, double yVal, double zVal)
    {
      x=xVal;
      y=yVal;
      z=zVal;
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

unitVector** normalVector;
double* zValues;
int XI=100, YI=100, ZI=MAXDEPTH;

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
    if(option==1)
        pixmapOrig[val++]=fileBuffer[index++];
    else
        pixmapBackground[val++]=fileBuffer[index++];
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
  XI=x;
  YI=height-y;
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
      if(option == 1)
        width=value;
      else
        widthBack=value;
      break;
    case HEIGHTVALUEID:
      if(option == 1)
        height=value;
      else
        heightBack=value;
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
  if(option == 1)
  {
    wVal=width;
    hVal=height;
  }
  else
  {
    wVal=widthBack;
    hVal=heightBack;
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
        pixmapBackground = new unsigned char[widthBack*heightBack*3];
      if(!(fillPixelsBin(index, fileBuffer, numOfCharacters, option)))
        return false;
      delete[] fileBuffer;
      return true;
    }
  }
}



//************************************************************************************************************
void emboss(int x, int y, double& xGrad, double& yGrad)
{
  long int iterator1, iterator2, iterator3, iterator4;
  if(x == 0)
    x+=width;
  if(y == 0)
    y+=height;
  iterator1 = (((y%height)*width)+((x+1)%width))*3;
  iterator2 = (((y%height)*width)+((x-1)%width))*3;
  iterator3 = ((((y+1)%height)*width)+(x%width))*3;
  iterator4 = ((((y-1)%height)*width)+(x%width))*3;
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
      zValues[(y*width)+x]=colorVal * (MAXDEPTH/10);
      double xGrad, yGrad;
      emboss(x,y,xGrad,yGrad);
      double len = pow((pow(xGrad,2)+pow(yGrad,2)+1),0.5);
      normalVector[(y*width)+x] = new unitVector(xGrad/len, yGrad/len, -1/len);
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

void reflect()
{
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int iterator = ((y*width)+x)*3;
      unitVector eyeVector((double)(x-XI), (double)(y-YI), zValues[(y*width)+x]-ZI);
      double prod1 = ((normalVector[(y*width)+x])->dotProduct(eyeVector))*2;
      int xRes = -(((normalVector[(y*width)+x]->getXValue())*prod1)-eyeVector.getXValue())+x;
      int yRes = -(((normalVector[(y*width)+x]->getYValue())*prod1)-eyeVector.getYValue())+y;
    
      if(xRes<0)
      {
        xRes=xRes*(-1);
        xRes=xRes%widthBack;
        xRes=xRes*(-1);
        xRes+=widthBack;
      }
      if(yRes<0)
      {
        yRes=yRes*(-1);
        yRes=yRes%heightBack;
        yRes=yRes*(-1);
        yRes+=heightBack;
      }
      long int finalRes = ((yRes%heightBack)*widthBack+(xRes%widthBack))*3;
      pixmapComputed[iterator++]=pixmapBackground[finalRes++];
      pixmapComputed[iterator++]=pixmapBackground[finalRes++];
      pixmapComputed[iterator]=pixmapBackground[finalRes];
    }
  }
}

void applyOperation()
{
  switch(option)
  {
    case 1:
      reflect();      
      windowDisplay();
      break;
    case 2:
      reflect();
      windowDisplay();
      break;
  }
}

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{
  char inputPPMFile[100], envFile[100];
  cout<<"Enter the height field ppm file\n";
  cin>>inputPPMFile;
  readPPMFile(inputPPMFile,1);
  cout<<"Enter the environment image file\n";
  cin>>envFile;
  readPPMFile(envFile,2);
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
     

