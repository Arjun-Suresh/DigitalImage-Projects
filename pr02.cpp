// =============================================================================
// VIZA654/CSCE646 at Texas A&M University
// Homework 0
// Created by Anton Agana based from Ariel Chisholm's template
// 05.23.2011
//
// This file is supplied with an associated makefile. Put both files in the same
// directory, navigate to that directory from the Linux shell, and type 'make'.
// This will create a program called 'pr01' that you can run by entering
// 'homework0' as a command in the shell.
//
// If you are new to programming in Linux, there is an
// excellent introduction to makefile structure and the gcc compiler here:
//
// http://www.cs.txstate.edu/labs/tutorials/tut_docs/Linux_Prog_Environment.pdf
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

using namespace std;
// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height, maxColorValue;
unsigned char *pixmap;

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
void setPixels()
{
   for(int y = 0; y < height ; y++) {
     for(int x = 0; x < width; x++) {
       int i = (y * width + x) * 3; 
       pixmap[i++] = 255;
       pixmap[i++] = 0xFF; //Do you know what "0xFF" represents? Google it!
       pixmap[i] = 0x00; //Learn to use the "0x" notation to your advantage.
     }
   }
}

void setForeGroundPixel(int y, int x)
{
  int i = (y * width + x) * 3; 
  pixmap[i++] = 255;
  pixmap[i++] = 0x00;
  pixmap[i] = 0x00;
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


int* formseq(int *x, int* y, int* xNew, int* yNew, int n)
{
  int min=999, minIndex;
  for (int i=0;i<n;i++)
  {
    if(min>x[i])
    {
      min=x[i];
      minIndex=i;
    }
    else if(min == x[i])
    {
      if(y[minIndex]>y[i])
        minIndex=i;
    }
  }
  xNew[0]=x[minIndex];
  yNew[0]=y[minIndex];
  int nextIndex,flag=0;
  if(y[(n+(minIndex-1))%n] > y[(n+(minIndex+1))%n])
  {
    nextIndex = (n+(minIndex-1))%n;
    flag=-1;
  }
  else
  {
    nextIndex = (n+(minIndex+1))%n;
    flag=1;
  }
  int k=1;
  while (nextIndex != minIndex)
  {
    xNew[k]=x[nextIndex];
    yNew[k]=y[nextIndex];
    k++;
    nextIndex=(n+nextIndex+flag)%n;
  }
}
 
float slope(int x0, int x1, int y0, int y1)
{
  return ((float)(y0-y1)/(float)(x0-x1));
}



int checkIfPointInsideLineSet(int* x, int* y, int n, double xtest, double ytest, int option)
{
  int neg=0;
  int pos=0;
  for(int i=0;i<n;i++)
  {
    int x0=x[i];
    int y0=y[i];
    int x1=x[(i+1)%n];
    int y1=y[(i+1)%n];
    if (x0==x1)
    {
      if(y1>y0)
      {
        if(xtest>=x0)
        {
          neg++;
        }
        else 
          pos++;
      }
      else
      { 
        if(xtest<=x0)
          neg++;
        else 
          pos++;
      }
    }
    else if (y0==y1)
    {
      if(x1>x0)
      {
        if(ytest>=y0)
          neg++;
        else 
          pos++;
      }
      else
      { 
        if(ytest<=y0)
          neg++;
        else 
          pos++;
      }
    }
    else
    {
      if(slope(x0,x1,y0,y1) > 0)
      {
        double val = ((ytest-y0)/(y1-y0)) - ((xtest-x0)/(x1-x0)); 
        if(val <= 0)
        {
          neg++;   
        }
        else
          pos++;
      }
      else
      {
        double val = ((xtest-x0)/(x1-x0)) - ((ytest-y0)/(y1-y0));
        if(val <= 0)
        {
          neg++;          
        }
        else
          pos++;
      }     
    }
  }
  switch (option)
  {
    case 1:
      if(neg == n)
        return 1;
      break;
    case 2:
      if(neg >= n-1)
        return 1;
      break;
  }
  return 0;
}


int checkIfPointInsideFunction(int x, int y)
{
  double xVal=(x*360)/width;
  double xRad = (xVal*3.14159)/180;
  double yVal=(100*sin(xRad))+500;
  if(yVal < 500)
  {
    if(y>=(int)yVal && y<= 500)
      return 1;
  }
  else
  {
    if(y<=(int)yVal && y>=500)
      return 1;
  }
  return 0;
}


int checkIfPointInCircle(int x, int y, double xcenter, double ycenter, int radius)
{
  double dist = pow((pow((x-xcenter),2)+pow((y-ycenter),2)),0.5);
  if ((int)dist<=radius)
    return 1;
  return 0;
}

int checkIfPointInsideBlob(int x, int y)
{
  if(x>=200 && x<=800)
  {
    double xVal = ((x-200)*180)/600;
    double xRad = (xVal*3.14159)/180;
    double ySineVal1=(300*sin(xRad))+400;
    double ySineVal2=(100*sin(xRad*9))+400;
    if(ySineVal1 < ySineVal2)
    {
      if(y>=(int)ySineVal1 && y<= ySineVal2)
      {
        if(checkIfPointInCircle(x, y, 500, 400, 250))
          return 1;
      }
    }
    else
    {
      if(y<=(int)ySineVal1 && y>=ySineVal2)
      {
        if(checkIfPointInCircle(x, y, 500, 400, 250))
          return 1;
      }
    }
  }
  return 0;
}

void setPixelColor(int y, int x, int green, int blue)
{
  int i = (y * width + x) * 3; 
  pixmap[i++] = 255;
  pixmap[i++] = green;
  pixmap[i] = blue;
}


int antialiazeCircle(int i, int j, int xcenter, int ycenter, int radius)
{
  double colorVal=0;
  for(int m=0;m<4;m++)
  {
    for(int l=0;l<4;l++)
    {
      double x= i + (double)l/4 + ((double)rand() / (double)RAND_MAX)/4;
      double y= j + (double)m/4 + ((double)rand() / (double)RAND_MAX)/4;
      if(checkIfPointInCircle(x,y,xcenter,ycenter,radius))
      {
        colorVal=colorVal+((double)1/(double)16);
      }
    }
  }
  int val = ceil(colorVal);
  return val;  
}




void fillShadedCircle()
{
  int radius=200, xcenter=500, ycenter=500, green=0, blue=0, step=0;
  while(radius>=1)
  {
    green=(step*255)/199;
    blue=(step*255)/199;
    for(int y=ycenter-radius-5;y<ycenter+radius+5;y++)
    {
      for(int x=xcenter-radius-5;x<xcenter+radius+5;x++)
      {
        if(antialiazeCircle(x,y,xcenter,ycenter,radius))
          setPixelColor(y,x,green, blue);
      }
    }
    step++;
    radius--;
  }          
}


int antialiazeLineFunction(int* xNew, int* yNew, int n, int i, int j, int option)
{
  double colorVal=0;
  for(int m=0;m<16;m++)
  {
    for(int l=0;l<16;l++)
    {
      double x= i + (double)m/16 + ((double)rand() / (double)RAND_MAX)/16;
      double y= j + (double)l/16 + ((double)rand() / (double)RAND_MAX)/16;
      if(checkIfPointInsideLineSet(xNew, yNew, n, x, y, option))
      {
        colorVal=colorVal+((double)1/(double)256);
      }
    }
  }
  int val = colorVal+0.5;
  return val;  
}



int antialiazeCurveFunctions(int i, int j, int option)
{
  double colorVal=0;
  for(int m=0;m<16;m++)
  {
    for(int l=0;l<16;l++)
    {
      double x= i + (double)l/16 + ((double)rand() / (double)RAND_MAX)/16;
      double y= j + (double)m/16 + ((double)rand() / (double)RAND_MAX)/16;
      int checkVal;
      switch(option)
      {
        case 3:
          checkVal = checkIfPointInsideFunction(x,y);
          break;
        case 4:
          checkVal = checkIfPointInsideBlob(x,y);
          break;
      }
      
      if(checkVal)
      {
        colorVal=colorVal+((double)1/(double)256);
      }
    }
  }
  int val = ceil(colorVal);
  return val;  
}


void fillShape(int* x, int* y, int n, int option)
{
  int xNew[100], yNew[100];
  if(option<3)
    formseq(x,y,xNew, yNew, n);
  if(option == 5)
    fillShadedCircle();
  else
  {
    for(int i=0;i<height;i++)
    {
      for(int j=0;j<width;j++)
      {
        int val;
        switch(option)
        {
          case 1:
          case 2:
            val=antialiazeLineFunction(xNew, yNew, n, j, i, option);
            break;
          case 3:
          case 4:
            val=antialiazeCurveFunctions(j,i, option);
            break;
        }
        if(val)
        {
          setForeGroundPixel(i,j);
        }
      }
    }
  }
}



void readData(std::fstream& inputFile, int* xpoints, int* ypoints)
{
  int x,y,count=0;
  char c, cArray[10],k=0;
  while(inputFile.get(c))
  {
    if(c == ' ')
    {
      cArray[k]='\0';
      k=0;
      string num(cArray);
      istringstream(num) >> x;
      xpoints[count]=x;
    }
    else if(c=='\n')
    {
      cArray[k]='\0';
      k=0;
      string num(cArray);
      istringstream(num) >> y;
      ypoints[count++]=y;
    }
    else
      cArray[k++]=c;
  }
}


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
      fileBuffer[index++]=pixmap[k++];
    }
  }
  return  fileBuffer;
}

void generatePPMFile(int option)
{
  std::fstream ppmFile;
  char fileName[50];
  switch(option)
  {
    case 1: 
      strcpy(fileName,"outputConvex.ppm");
      break;
    case 2: 
      strcpy(fileName,"outputStar.ppm");
      break;
    case 3: 
      strcpy(fileName,"outputFunction.ppm");
      break;
    case 4: 
      strcpy(fileName,"outputBlobby.ppm");
      break;
    case 5: 
      strcpy(fileName,"outputShaded.ppm");
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

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{

  //initialize the global variables
  width = 1000;
  height = 1000;
  std::fstream inputFile;
  int xpoints[100], ypoints[100];
  pixmap = new unsigned char[width * height * 3];  //Do you know why "3" is used?  
  setPixels();
  int option;
  cout<<"Enter:\n1 for convex shape\n2 for star\n3 for function\n4 for blobby\n5 for shaded circle\n";
  cin >> option;
  switch(option)
  {
    case 1:
      cout<<"Reading data from 'inputConvex.txt'\n";
      inputFile.open("inputConvex.txt", std::fstream::in);
      readData(inputFile, xpoints, ypoints);
      inputFile.close();
      fillShape(xpoints, ypoints, 4, option);
      break;
    case 2:
      cout<<"Reading data from 'inputStar.txt'\n";
      inputFile.open("inputStar.txt", std::fstream::in);
      readData(inputFile, xpoints, ypoints);
      inputFile.close();
      fillShape(xpoints, ypoints, 5, option);
      break;
    case 3:
    case 4:
    case 5:
      fillShape(NULL, NULL, 0, option);
      break;
  }
  generatePPMFile(option);
  // OpenGL Commands:
  // Once "glutMainLoop" is executed, the program loops indefinitely to all
  // glut functions.  
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
     

