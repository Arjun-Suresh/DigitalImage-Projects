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

#include <fstream>
#include <cassert>
#include <sstream>
#include <string>

using namespace std;

// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height;
unsigned char *pixmap;


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



int checkIfPointInside(int* x, int* y, int n, int xtest, int ytest, int option)
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
          neg++;
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
        if(((((float)(ytest-y0))/((float)(y1-y0)))-(((float)(xtest-x0))/((float)(x1-x0)))) <= 0)
          neg++;
        else
          pos++;
      }
      else
      {
        if(((((float)(xtest-x0))/((float)(x1-x0)))-(((float)(ytest-y0))/((float)(y1-y0)))) <= 0)
          neg++;
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


void fillShape(int* x, int* y, int n, int option)
{
  int xNew[100], yNew[100];
  formseq(x,y,xNew, yNew, n);
  for(int i=0;i<height;i++)
  {
    for(int j=0;j<width;j++)
    {
      int pos=0, neg=0;
      int val=checkIfPointInside(xNew, yNew, n, j, i, option);
      if(val)
        setForeGroundPixel(i,j);
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

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{

  //initialize the global variables
  width = 300;
  height = 300;
  pixmap = new unsigned char[width * height * 3];  //Do you know why "3" is used?
  std::fstream inputFile;
  inputFile.open("input.txt", std::fstream::in);
  int xpoints[100], ypoints[100];
  readData(inputFile, xpoints, ypoints);
  inputFile.close();  
  setPixels();
  int option, n;
  cout<<"Enter 1 for convex shape and 2 for star\n";
  cin >> option;
  switch(option)
  {
    case 1:
      n=4;
      break;
    case 2:
      n=5;
      break;
  }
  fillShape(xpoints, ypoints, n, option);
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
     

