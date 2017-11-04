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
      strcpy(fileName,"outputBilinearWarp.ppm");
      break;
    case 2: 
      strcpy(fileName,"outputInverse.ppm");
      break;
    case 3:
      strcpy(fileName,"outputSineWarp.ppm");
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

bool verifyResult(int xRes, int yRes)
{
  if(xRes<width && xRes>=0 && yRes<height && yRes>=0)
    return true;
  return false;
}

bool checkRange(double val)
{
  if(0<=val && val<=1)
    return true;
  else
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





void getColorValues(int& red, int& green, int& blue, int x, int y)
{
  int output1 = (y*width+x)*3;
  int output2,output3,output4,output5;
  output2=output3=output4=output5=output1;
  if(x<width-1)
  {
    output2= (y*width+(x+1))*3;
    if(y<height-1)
    output3= ((y+1)*width+(x+1))*3;
  }
  if(x>0)
  {
    output4= (y*width+(x-1))*3;
    if(y>0)
    output5= ((y-1)*width+(x-1))*3;
  }
  red=(pixmapComputed[output1++]+pixmapComputed[output2++]+pixmapComputed[output3++]+pixmapComputed[output4++]+pixmapComputed[output5++])/5;
  green=(pixmapComputed[output1++]+pixmapComputed[output2++]+pixmapComputed[output3++]+pixmapComputed[output4++]+pixmapComputed[output5++])/5;
  blue=(pixmapComputed[output1]+pixmapComputed[output2]+pixmapComputed[output3]+pixmapComputed[output4]+pixmapComputed[output5])/5;

}


void getBoundaryPoints(double xPerspective,double yPerspective, int xVal, int yVal, int& resXVal, int& resYVal)
{
  double perspectiveMatrix[3][3];
  double pixelMatrix[3],resultMatrix[3],r1[3],r2[3];
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
      perspectiveMatrix[i][j]=0;
    }
    perspectiveMatrix[i][i]=1;   
  }
  perspectiveMatrix[2][0]=xPerspective/(width*10);
  perspectiveMatrix[2][1]=yPerspective/(height*10);
  initMatrix(pixelMatrix,xVal,yVal);
  multiplyMatrix(pixelMatrix, perspectiveMatrix, resultMatrix);
  getValues(resultMatrix,resXVal,resYVal);
}

void getInverseBilinearWarp(int* x,int* y,int xVal,int yVal,int& xRes,int& yRes)
{
  double a0,a1,a2,a3,b0,b1,b2,b3;
  a0=x[0];
  a1=x[3]-x[0];
  a2=x[1]-x[0];
  a3=x[2]+x[0]-x[1]-x[3];
  b0=y[0];
  b1=y[3]-y[0];
  b2=y[1]-y[0];
  b3=y[2]+y[0]-y[1]-y[3];
  double c0,c1,c2;
  c0=a1*(b0-yVal)+b1*(xVal-a0);
  c1=a3*(b0-yVal)+b3*(xVal-a0)+a1*b2-a2*b1;
  c2=a3*b2-a2*b3;
  double u1,v1,u2,v2,u,v;
  v1=((-1*c1)/(2*c2))+(1/(2*c2))*pow((pow(c1,2)-(4*c2*c0)),0.5);
  v2=((-1*c1)/(2*c2))-(1/(2*c2))*pow((pow(c1,2)-(4*c2*c0)),0.5);
  u1=(xVal-a0-a2*v1)/(a1+a3*v1);
  u2=(xVal-a0-a2*v2)/(a1+a3*v2);
  if(!((checkRange(u1) && checkRange(v1)) || (checkRange(u2) && checkRange(v2))))
  {
    xRes=-1;
    yRes=-1;
    return;
  }
  else if(checkRange(u1) && checkRange(v1))
  {
    u=u1;
    v=v1;
  }
  else if(checkRange(u2) && checkRange(v2))
  {
    u=u2;
    v=v2;
  }
  else
  {
    xRes=-1;
    yRes=-1;
    return;
  }
  xRes = (int)((width-1)*u+0.5);
  yRes = (int)((height-1)*v+0.5);
}

void bilinearWarp(double xPerspective, double yPerspective)
{
  int x[4],y[4];
  getBoundaryPoints(xPerspective, yPerspective, 0, 0, x[0], y[0]);
  getBoundaryPoints(xPerspective, yPerspective, 0, height-1, x[1], y[1]);
  getBoundaryPoints(xPerspective, yPerspective, width-1, height-1, x[2], y[2]);
  getBoundaryPoints(xPerspective, yPerspective, width-1, 0, x[3], y[3]);
  for(int yVal=0;yVal<height-1;yVal++)
  {
    for(int xVal=0;xVal<width-1;xVal++)
    {
      int xRes, yRes;
      getInverseBilinearWarp(x,y,xVal,yVal,xRes,yRes);
      if(verifyResult(xRes,yRes))
      {
        int input = (yRes * width + xRes) * 3;
        int output = (yVal*width + xVal) * 3;
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output] = pixmapOrig[input];
      }
    }
  }
}

double getTheta(int x, int y)
{
  if(x==0)
    return (3.1416/2);
  double cosVal = (double)x/(pow((pow(x,2)+pow(y,2)),0.5));
  double sineVal = (double)y/(pow((pow(x,2)+pow(y,2)),0.5));
  double theta = atan((double)y/(double)x);
  if(cosVal > 0 && sineVal < 0)
    theta+=(2*3.1416);
  else if(cosVal < 0 && sineVal < 0) 
    theta+=3.1416;
  else if(cosVal <  0 && sineVal > 0)
    theta+=3.1416;
  return theta;
}

void getNewComplexNumbers(double r, double theta, int& xRes, int& yRes)
{
  xRes = r*cos(theta);
  yRes = r*sin(theta);
}

void inverseWarp(int flag)
{
  for(int yVal=0;yVal<height-1;yVal++)
  {
    for(int xVal=0;xVal<width-1;xVal++)
    {
      int xRes, yRes;
      if(!flag)
      {
        double theta = getTheta(xVal,yVal);
        double r = pow((pow(xVal,2)+pow(yVal,2)),0.5);
        getNewComplexNumbers(pow(r,(1/1.2)),theta/1.2, xRes, yRes);
      }
      else
      {
        xRes = xVal;
        yRes = (int)(yVal-(double)5*sin((double)xVal/10));
      }
      if(verifyResult(xRes,yRes))
      {
        int input = (yRes * width + xRes) * 3;
        int output = (yVal*width + xVal) * 3;
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output] = pixmapOrig[input];
      }
    }
  }
}

void applyTransformation(int option)
{
  switch(option)
  {
    case 1:
    double xPerspective, yPerspective;
    cout<<"Enter the x and y perspective values\n";
    cin>>xPerspective>>yPerspective;
    bilinearWarp(xPerspective,yPerspective);
    break;
    case 2:
    inverseWarp(0);
    break;
    case 3:
    inverseWarp(1);
    break;
  }
}


void initOutputPixMap()
{
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int i=(y*width+x)*3;
      pixmapComputed[i++]=0;
      pixmapComputed[i++]=0;
      pixmapComputed[i]=0;
    }
  }
}

void swap(int& a, int& b)
{
  int t;
  t=a;
  a=b;
  b=t;
}

void reconfigureInputPixMap()
{
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int i=(y*width+x)*3;
      int old = ((height-y-1)*width+x)*3;
      swap(pixmapOrig[i++],pixmapOrig[old++]);
      swap(pixmapOrig[i++],pixmapOrig[old++]);
      swap(pixmapOrig[i],pixmapOrig[old]);
    }
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
  initOutputPixMap();
  reconfigureInputPixMap();

  int option;
  cout<<"Enter options:\n1. Bilinear warp\n2. Inverse warp\n3. Sine warp\n";
  cin>>option;

  applyTransformation(option);

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
     

