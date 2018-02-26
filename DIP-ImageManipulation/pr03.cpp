// =============================================================================
// VIZA654/CSCE646 at Texas A&M University
// Homework 3
// Image color and hue manipulation
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


#define maximum(x, y, z) ((x) > (y)? ((x) > (z)? (x) : (z)) : ((y) > (z)? (y) : (z)))
#define minimum(x, y, z) ((x) < (y)? ((x) < (z)? (x) : (z)) : ((y) < (z)? (y) : (z)))

using namespace std;
// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height, maxColorValue, magicNo,widthControl, heightControl;
unsigned char *pixmapOrig, *pixmapControl, *pixmapComputed;

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
  switch (option)
  {
    case 1:
    case 2:
      pixmapOrig[val++]=fileBuffer[index++];
      break;
    case 3:
      pixmapControl[val++]=fileBuffer[index++];
      break;
    default:
      return;
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
      if(option<3)
      width=value;
      else
      widthControl=value;
      break;
    case HEIGHTVALUEID:
      if(option<3)
      height=value;
      else
      heightControl=value;
      break;
    case MAXCOLORVALUEID:
      maxColorValue=value;
      break;
  }
  return true;
}

bool fillPixelsBin(long int& index, unsigned char* fileBuffer, long int numOfCharacters,int option)
{
  int wVal,hVal;
  if(option<3)
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
      if( option < 3)
        pixmapOrig = new unsigned char[width*height*3];
      else
        pixmapControl = new unsigned char[widthControl*heightControl*3];
      if(!(fillPixelsBin(index, fileBuffer, numOfCharacters,option)))
        return false;
      return true;
    }
  }
  delete[] fileBuffer;
  return true;
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
      strcpy(fileName,"outputLinear.ppm");
      break;
    case 2: 
      strcpy(fileName,"outputCubic.ppm");
      break;
    case 3: 
      strcpy(fileName,"outputHueChanged.ppm");
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
void readData(char* filePath, int* rArray, int& num)
{
  std::fstream inputFile;
  int x,y,count=0;
  char c, cArray[10];
  int k=0;  
  cout<<"Reading data from "<<filePath<<"\n";
  inputFile.open(filePath, std::fstream::in);
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
        rArray[count++]=x;
      }
    }
    else
    {
      cArray[k++]=c;
    }
  }
  num=count;
}





//*****************************************************************************************************
//*************************************Image color Manipulation functions******************************
//*****************************************************************************************************

//Which interval of xarray does x belong to
int findInterval(float color, float* x, int n)
{
  int i;
  for(i=0;i<=n;i++)
  {
    if(color<x[i])
      break;
  }
  return i-1;
}

//get xi,yi from r0,....,rn
void formXYPoints(float* x, float* y, int* rArray, int n)
{
  x[0]=0;
  y[0]=rArray[0];
  for(int i=1;i<=n;i++)
  {
    x[i]=x[i-1]+1/(float)(n);
    y[i]=rArray[i];
  }
}

//get mi array used for cubic hermite spline
void formSlope(float* m, float* y, int n)
{
  m[0]=(y[1]*n)/2;
  for(int i=1;i<n;i++)
    m[i]=((y[i+1]-y[i-1])*n)/2;
  m[n]=-1*((y[n-1]*n)/2);
}


//Basic linear interpolation and cubic hermite spline interpolation used
int interpolate(int color, float* x, float* y, float* slope, int n, int option)
{
  float colorVal = (float)color/255;  
  int interval = findInterval(colorVal,x,n);
  float tVal = (colorVal-x[interval])/(x[interval+1]-x[interval]);
  float output=0;
  if(interval==n)
    return (int)y[n];
  if (option == 1)
  {
    output = (y[interval]*(1-tVal))+(y[interval+1]*tVal);
  }
  else if (option == 2)
  {
    float h0 = 2*pow(tVal,3) - 3*pow(tVal,2) + 1;
    float h1 = 3*pow(tVal,2) - 2*pow(tVal,3);
    float h2 = pow(tVal,3) - 2*pow(tVal,2) + tVal;
    float h3 = pow(tVal,3) - pow(tVal,2);
    output = y[interval]*h0 + y[interval+1]*h1 + slope[interval]*h2 + slope[interval+1]*h3;    
  }
  if(output>255)
    output=255;
  if(output<0)
    output=0;
  return (int)output;
}

//For each pixel, interpolate red, green and blue values into the o/p color space
void colorManipulate(int* rArray, int n, int option)
{
  int red,green,blue;
  float xArray[1000],yArray[1000],slope[1000];
  formXYPoints(xArray,yArray,rArray,n);
  if(option==2)
    formSlope(slope,yArray,n);
    
  for (int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {     
      int i = (y * width + x) * 3;
      red=pixmapOrig[i++];
      int redNew=interpolate(red,xArray,yArray,slope,n,option);
      green=pixmapOrig[i++];
      int greenNew=interpolate(green,xArray,yArray,slope,n,option);
      blue=pixmapOrig[i];
      int blueNew=interpolate(blue,xArray,yArray,slope,n,option);
      setPixelColor(y,x,redNew,greenNew,blueNew);
    }
  }
}





//*****************************************************************************************************
//*****************************************Image hue manipulation functions****************************
//*****************************************************************************************************

//I have borrowed the code given in the project page
void RGBtoHSV(int r, int g, int b, double &h, double& s, double& v){

  double red, green, blue;
  double max, min, delta;

  red = r / 255.0; green = g / 255.0; blue = b / 255.0;  /* r, g, b to 0 - 1 scale */

  max = maximum(red, green, blue);
  min = minimum(red, green, blue);

  v = max;        /* value is maximum of r, g, b */

  if(max == 0){    /* saturation and hue 0 if value is 0 */
    s = 0;
    h = 0;
  }
  else{
    s = (max - min) / max;           /* saturation is color purity on scale 0 - 1 */

    delta = max - min;
       if(delta == 0)                    /* hue doesn't matter if saturation is 0 */
      h = 0;
    else{
      if(red == max)                  /* otherwise, determine hue on scale 0 - 360 */
        h = (green - blue) / delta;
      else if(green == max)
        h = 2.0 + (blue - red) / delta;
      else /* (blue == max) */
        h = 4.0 + (red - green) / delta;
      h = h * 60.0;
      if(h < 0)
        h = h + 360.0;
    }
  }
}

//Basic conversion using checking which interval h falls into
void HSVtoRGB(double h, double s, double v, int& r, int& g, int& b)
{
  double redVal, greenVal, blueVal;
  if(v == 0.0)
  {
    r=0;
    g=0;
    b=0;
    return;
  }
  if(s == 0.0)
  {
    redVal=v;
    greenVal=redVal;
    blueVal=redVal;
  }
  else if(0.0<=h && h<=60.0)
  {
    h=h/60;
    redVal=v;
    blueVal=v-s*v;
    greenVal=blueVal+(h*(redVal-blueVal));
    
  }
  else if(300.0<=h && h<=360.0)
  {
    h-=360;
    h=h/60;
    redVal=v;
    greenVal=v-s*v;
    blueVal=greenVal-(h*(redVal-greenVal));
  }
  else if(60.0<=h && h<=120.0)
  {
    h=h/60;
    h-=2;
    greenVal=v;
    blueVal=v-s*v;
    redVal=blueVal-(h*(greenVal-blueVal));    
  }
  else if(120.0<=h && h<=180.0)
  {
    h=h/60;
    h-=2;
    greenVal=v;
    redVal=v-s*v;
    blueVal=redVal+(h*(greenVal-redVal));
  }
  else if(180.0<=h && h<=240.0)
  {
    h=h/60;
    h-=4;
    blueVal=v;
    redVal=v-s*v;
    greenVal=redVal-(h*(blueVal-redVal));    
  }
  else if(240.0<=h && h<=300.0)
  {
    h=h/60;
    h-=4;
    blueVal=v;
    greenVal=v-s*v;
    redVal=greenVal+(h*(blueVal-greenVal));
  }
  r=(int)(redVal*255.0);
  g=(int)(greenVal*255.0);
  b=(int)(blueVal*255.0);
}

//Get hue from control image
void fillControlHue(double* controlHue)
{
  int count=0, red, green, blue;
  for (int y=0;y<heightControl;y++)
  {
    for(int x=0;x<widthControl;x++)
    {     
      int i = (y * widthControl + x) * 3;
      red=pixmapControl[i++];
      green=pixmapControl[i++];
      blue=pixmapControl[i];
      double hue,sat,val;
      RGBtoHSV(red,green,blue,hue,sat,val);
      controlHue[count++]=hue;
    }
  }
}

//Replace hue values of original image using that of control image
//SInce the control image can be of any size, I am cycling through the hues if the control image is smaller
void hueManipulate()
{
  double* controlHue = new double[widthControl*heightControl];
  fillControlHue(controlHue);
  
  for (int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {     
      int i = (y * width + x) * 3;       
      int red,green, blue;
      red=pixmapOrig[i++];
      green=pixmapOrig[i++];
      blue=pixmapOrig[i];

      double hue,sat,val;
      RGBtoHSV(red,green,blue,hue,sat,val);
      double newHue = controlHue[(y * width + x)%(widthControl*heightControl)];

      int nRed,nBlue,nGreen;
      HSVtoRGB(newHue,sat,val,nRed,nGreen,nBlue);

      setPixelColor(y,x,nRed,nGreen,nBlue);      
    }
  }
  
}

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{
  int option;
  cout<<"Enter options:\n1.Linear interpolation\n2.Cubic hermitian interpolation\n3. Hue manipulation\n";
  cin>>option;
  
  if(option<3)
  {
    int num=0;
    std::fstream inputFile;
    int rArray[1000];
    char filePath[100]="inputFunction.txt";
    readData(filePath, rArray, num);
    char inputPPMFile[100];
    cout<<"Enter the ppm file to be manipulated\n";
    cin>>inputPPMFile;
    readPPMFile(inputPPMFile,1);
    pixmapComputed = new unsigned char[width * height * 3];
    colorManipulate(rArray,num-1,option);
  }
  
  if(option == 3)
  {
    char inputPPMFile[100], controlPPMFile[100];
    cout<<"Enter the ppm file to be manipulated\n";
    cin>>inputPPMFile;
    cout<<"Enter the control ppm file\n";
    cin>>controlPPMFile;
    readPPMFile(inputPPMFile,2);
    readPPMFile(controlPPMFile,3);
    pixmapComputed = new unsigned char[width * height * 3];
    hueManipulate();
  }
  
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
     

