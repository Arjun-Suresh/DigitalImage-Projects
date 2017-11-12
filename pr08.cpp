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

#define diff(a,b) ((a>b)?(a-b):(b-a))

#define KERNELSIZE 25

using namespace std;
// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================
int width, height, width1, height1, maxColorValue, magicNo;
unsigned char *pixmapBackGround, *pixmapForeGround, *pixmapTriMap, *pixmapComputed;

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
    pixmapBackGround[val++]=fileBuffer[index++];
  else if(option == 2)
    pixmapForeGround[val++]=fileBuffer[index++];
  else if(option == 3)
    pixmapTriMap[val++]=fileBuffer[index++];
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
      if(option == 1 || option == 3)
      	width=value;
      else
        width1=value;
      break;
    case HEIGHTVALUEID:
      if(option == 1 || option == 3)
      	height=value;
      else
        height1=value;
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
  wVal=width;
  hVal=height;
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
        pixmapBackGround = new unsigned char[width*height*3];
      else if(option == 2)
        pixmapForeGround = new unsigned char[width1*height1*3];
      else if(option == 3)
        pixmapTriMap = new unsigned char[width*height*3];
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
  {
    fileBuffer[index++]=data[i];
  }
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
      strcpy(fileName,"outputNormal.ppm");
      break;
    case 2: 
      strcpy(fileName,"outputMultiply.ppm");
      break;
    case 3:
      strcpy(fileName,"outputSubtract.ppm");
      break;
    case 4:
      strcpy(fileName,"outputMax.ppm");
      break;
    case 5:
      strcpy(fileName,"outputMin.ppm");
      break;
    case 6:
      strcpy(fileName,"outputScreen.ppm");
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

void readAllPPMFiles(int option)
{
  char foregroundPPMFile[100], backgroundPPMFile[100], trimapFile[100];
  if(option<6)
  {
    cout<<"Enter the foreground ppm image\n";
    cin>>foregroundPPMFile;
    cout<<"Enter the background ppm image\n";
    cin>>backgroundPPMFile;
    readPPMFile(backgroundPPMFile,1);
    readPPMFile(foregroundPPMFile,2);
  }
  
  else
  {
    strcpy(foregroundPPMFile,"foreground.ppm");
    strcpy(backgroundPPMFile,"background.ppm");
    strcpy(trimapFile,"Trimap.ppm");
    readPPMFile(backgroundPPMFile,1);
    readPPMFile(foregroundPPMFile,2);
    readPPMFile(trimapFile,3);
  }
  pixmapComputed = new unsigned char[width * height * 3];
}

//************************************************************************************************************
//*************************************Compositing functions**************************************************
//************************************************************************************************************
double compute(int color1, int color2, int option)
{
  double c1 = (double)color1/255.0;
  double c2 = (double)color2/255.0;
  switch(option)
  {
    case 1:
      return (double)color2;
      break;
    case 2:
      return c2*c1*255.0;
      break;
    case 3:
      return (c2>c1)?((c2-c1)*255):(0);
      break;
    case 4:
      return (c2>c1)?(c2*255.0):(c1*255.0);
      break;
    case 5:
      return (c2<c1)?(c2*255.0):(c1*255.0);
      break;
  }
}
void applyCompositingOver(double alpha1, double alpha2, int option)
{
  int red1,red2,blue1,blue2,green1,green2;
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int iVal = (y * width + x) * 3; 
      double a1=alpha1, a2=alpha2;
      if( x < width1 && y < height1)
      {
        red2 = pixmapForeGround[iVal+1];
        green2 = pixmapForeGround[iVal+2];
        blue2 = pixmapForeGround[iVal+3];
      }
      else
      {
        red2=green2=blue2=0;
        a2=0;
        a1=1;
      }
      red1 = pixmapBackGround[iVal+1];
      green1 = pixmapBackGround[iVal+2];
      blue1 = pixmapBackGround[iVal+3];
      pixmapComputed[iVal++]=(int)((a2*compute(blue1,blue2,option)+(1.0-a2)*(double)blue1*a1)/(a2+(1-a2)*a1));
      pixmapComputed[iVal++]=(int)((a2*compute(red1,red2,option)+(1.0-a2)*(double)red1*a1)/(a2+(1-a2)*a1));
      pixmapComputed[iVal]=(int)((a2*compute(green1,green2,option)+(1.0-a2)*(double)green1*a1)/(a2+(1-a2)*a1));
    }
  }      
}

void fillKernel(double x,double y,double kernel[KERNELSIZE][KERNELSIZE])
{
  for(int i=0;i<KERNELSIZE;i++)
  {
    for(int j=0;j<KERNELSIZE;j++)
    {
      int rowVal=(i-(KERNELSIZE/2)+x);
      int colVal=(j-(KERNELSIZE/2)+y);
      if(rowVal<0)
        rowVal=0;
      if(colVal<0)
        colVal=0;
      if(rowVal>width-1)
        rowVal=width-1;
      if(colVal>height-1)
        colVal=height-1;
      int pixIndex = ((colVal*width+rowVal)*3);
      kernel[i][j]=(double)pixmapTriMap[pixIndex]/255.0;
      if(kernel[i][j]<0.48)
        kernel[i][j]=0;
      else if(kernel[i][j]>=0.8)
        kernel[i][j]=1;
    }
  } 
}

double checkKernel(double kernel[KERNELSIZE][KERNELSIZE])
{
  int val=0;
  for(int i=0;i<KERNELSIZE;i++)
  {
    for(int j=0;j<KERNELSIZE;j++)
    {
      if(kernel[i][j] < 1 && kernel[i][j]>0)
        return 0.5;
      val+=kernel[i][j];
    }
  }
  if(val)
    return 1.0;
  else
    return 0;
}

void getColors(double& red, double& green, double& blue, int x, int y, double kernel[KERNELSIZE][KERNELSIZE], int option)
{
  long int totalRedColorVal=0, totalGreenColorVal=0, totalBlueColorVal=0;
  long int totalNums=0;
  int kernelValCheck;
  unsigned char* pixmapCheck;
  if(option == COLORBACKGROUND)
  {
    kernelValCheck=0;
    pixmapCheck=pixmapBackGround;
  }
  else
  {
    kernelValCheck=1;
    pixmapCheck=pixmapForeGround;
  }
  for(int i=0;i<KERNELSIZE;i++)
  {
    for(int j=0;j<KERNELSIZE;j++)
    {
      if(kernel[i][j] == kernelValCheck)
      {
        int rowVal=(i-(KERNELSIZE/2)+x);
        int colVal=(j-(KERNELSIZE/2)+y);
        if(rowVal<0)
        rowVal=0;
      if(colVal<0)
        colVal=0;
      if(rowVal>width-1)
        rowVal=width-1;
      if(colVal>height-1)
        colVal=height-1;
        int pixIndex = ((colVal*width+rowVal)*3);
        totalRedColorVal+=pixmapCheck[pixIndex++];
        totalGreenColorVal+=pixmapCheck[pixIndex++];
        totalBlueColorVal+=pixmapCheck[pixIndex];
        totalNums++;
      }
    }
  }
  if(!totalNums)
  {
    red=green=blue=255.0;
  }
  else
  {
    red = (double)totalRedColorVal/(double)totalNums;  
    green = (double)totalGreenColorVal/(double)totalNums;  
    blue = (double)totalBlueColorVal/(double)totalNums;
  }
}
double getColorVal(int offset, int x, int y, double kernel[KERNELSIZE][KERNELSIZE])
{
  double totalColorVal=0;
  long int totalNums=0;
  for(int i=0;i<KERNELSIZE;i++)
  {
    for(int j=0;j<KERNELSIZE;j++)
    {
      int rowVal=(i-(KERNELSIZE/2)+x);
      int colVal=(j-(KERNELSIZE/2)+y);
      if(rowVal<0)
        rowVal=0;
      if(colVal<0)
        colVal=0;
      if(rowVal>width-1)
        rowVal=width-1;
      if(colVal>height-1)
        colVal=height-1;
      int pixIndex = ((colVal*width+rowVal)*3)+offset;
      unsigned char* pixmapCheck;
      totalColorVal+=(pixmapForeGround[pixIndex]*kernel[i][j]+pixmapBackGround[pixIndex]*(1-kernel[i][j]));
      totalNums++;
    }
  }
  return totalColorVal/(double)totalNums; 
}
double findAlpha(int x, int y)
{
  double kernel[KERNELSIZE][KERNELSIZE];
  fillKernel(x,y,kernel);
  double val = checkKernel(kernel);
  if(val == 0)
    return 0;
  if(val == 1)
    return 1.0;
  double redBack, redFore, greenBack, greenFore, blueBack, blueFore;
  getColors(redBack, greenBack, blueBack, x, y, kernel, COLORBACKGROUND);
  getColors(redFore, greenFore, blueFore, x, y, kernel, COLORFOREGROUND);
  //cout<<greenFore<<" "<<redFore<<" "<<blueFore<<endl;
  if(diff(redBack,redFore)!=0 && maximum(diff(redBack,redFore), diff(greenBack,greenFore), diff(blueBack,blueFore)) == diff(redBack,redFore))
    return (getColorVal(REDOFFSET,x,y,kernel)-redBack)/(redFore-redBack);
    //return 0;
  else if(diff(greenBack,greenFore)!=0 && maximum(diff(redBack,redFore), diff(greenBack,greenFore), diff(blueBack,blueFore)) == diff(greenBack,greenFore))
    return (getColorVal(GREENOFFSET,x,y,kernel)-greenBack)/(greenFore-greenBack);
  else if(diff(blueBack,blueFore)!=0 && maximum(diff(redBack,redFore), diff(greenBack,greenFore), diff(blueBack,blueFore)) == diff(blueBack,blueFore))
    return (getColorVal(BLUEOFFSET,x,y,kernel)-blueBack)/(blueFore-blueBack);
    //return 0;
  else
    return 1.0;    
}

void applyScreening()
{
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int iVal = (y * width + x) * 3; 
      double alpha;
      alpha = findAlpha(x,y);
      int red2 = pixmapForeGround[iVal+1];
      int green2 = pixmapForeGround[iVal+2];
      int blue2 = pixmapForeGround[iVal+3];
      int red1 = pixmapBackGround[iVal+1];
      int green1 = pixmapBackGround[iVal+2];
      int blue1 = pixmapBackGround[iVal+3];
      pixmapComputed[iVal++]=(int)(alpha*blue2+(1.0-alpha)*(double)blue1);
      pixmapComputed[iVal++]=(int)(alpha*red2+(1.0-alpha)*(double)red1);
      pixmapComputed[iVal]=(int)(alpha*green2+(1.0-alpha)*(double)green1);
    }
  }      
}
void applyCompositing(int option)
{
  double alpha1, alpha2;
  if(option < 6)
  {
    cout<<"Enter the transparency of foreground image\n";
    cin>>alpha2;
    cout<<"Enter the transparency of background image\n";
    cin>>alpha1;
    applyCompositingOver(alpha1, alpha2, option);
  }
  else
    applyScreening();
}
// =============================================================================
// main() Program Entry
// =============================================================================
int main(int argc, char *argv[])
{
  int option;
  cout<<"Enter options:\n1. Normal compositing\n2. Multiply compositing\n3. Subtract compositing\n4. Max compositing\n5. Min compositing\n6. Green screening\n";
  cin>>option;

  readAllPPMFiles(option);

  applyCompositing(option);

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
     

