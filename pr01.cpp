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
#include <cstring>
#include <malloc.h>
using namespace std;

// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height;
unsigned char *pixmap;

//Resizing the read buffer for large image files
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

unsigned char* fillPPMBuffer(std::fstream& ppmFile, long int& numOfCharacters)
{
  char c;
  long int bufferSize;
  unsigned char *fileBuffer = (unsigned char*)malloc(10000*sizeof(unsigned char));
  bufferSize=10000;
  while(ppmFile.get(c))
  {
    if(numOfCharacters == bufferSize - 1)
    {
      unsigned char* tempBuffer = resizeArray(fileBuffer, bufferSize, bufferSize);
      delete[] fileBuffer;
      fileBuffer=tempBuffer;
    }
    fileBuffer[numOfCharacters++]=c;
  }
  return fileBuffer;
}

void parseCommentLine(int& index, unsigned char* fileBuffer, long int numOfCharacters)
{
  while(index<numOfCharacters && fileBUffer[index]!='\n')
  index++;
}

      
bool readPPMFile(char* filePath)
{
  std::fstream ppmFile;
  long int numOfCharacters=0;
  long int numOfParsedLines=0;
  int magicNumberParsed=0, widthParsed=0, heightParsed=0, maxColorValueParsed=0;
  ppmFile.open (filePath, std::fstream::in);
  unsigned char *fileBuffer = fillPPMBuffer(ppmFile, numOfCharacters);
  for(long int index=0;index<numOfCharacters;index++)
  {
    if(fileBuffer[index]=='#')
    {
      parseCommentLine(index, fileBuffer, numOfCharacters);
      continue;
    }
    if(!magicNumberParsed)
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
            break;
          }
          else 
            return false;
        }
        if(isspace(fileBUffer[index]) && fileBuffer[index]!='\n')
          index++;
        else
          return false;
      }
      continue;
    }
    if(!(magicNumberParsed

   
  }
  ppmFile.close();
} 

// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{

  //initialize the global variables
  width = 750;
  height = 750;
  pixmap = new unsigned char[width * height * 3];  //Do you know why "3" is used?
  char ppmFilePath[100];
  std::cout<<"Give the path of the ppm file\n";
  std::cin>>ppmFilePath;
  bool val=readPPMFile(ppmFilePath);
  setPixels();


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

