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
      strcpy(fileName,"outputRotation.ppm");
      break;
    case 2: 
      strcpy(fileName,"outputScaling.ppm");
      break;
    case 3: 
      strcpy(fileName,"outputShearing.ppm");
      break;
    case 4: 
      strcpy(fileName,"outputMirror.ppm");
      break;
    case 5: 
      strcpy(fileName,"outputTranslate.ppm");
      break;
    case 6: 
      strcpy(fileName,"outputPerspective.ppm");
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
bool verifyResult(int xRes, int yRes)
{
  if(xRes<width && xRes>=0 && yRes<height && yRes>=0)
    return true;
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




void antiAliaseRotation(double t1[][3], double t2[][3], double theta, int xPivot, int yPivot)
{
  double rotationMatrix[3][3];
  double pixelMatrix[3],resultMatrix[3],r1[3],r2[3];
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
      if(i==2 || j==2)
        rotationMatrix[i][j]=0;      
    }
  }
  rotationMatrix[2][2]=1;
  rotationMatrix[0][0]=cos(theta);
  rotationMatrix[0][1]=sin(theta);
  rotationMatrix[1][0]=(-1) * sin(theta);
  rotationMatrix[1][1]=cos(theta);
  
  for(int y=height-1;y>=0;y--)
  {
    for(int x=0;x<width;x++)
    {
      int input = ((height-y-1) * width + x) * 3;
      if(pixmapComputed[input] == 0 && pixmapComputed[input+1] == 0 && pixmapComputed[input+2] == 0)
      {
        initMatrix(pixelMatrix,x,height-y-1);
        multiplyMatrix(pixelMatrix, t1, r1);
        multiplyMatrix(r1, rotationMatrix, r2);
        multiplyMatrix(r2, t2, resultMatrix);
        int xRes, yRes;
        getValues(resultMatrix,xRes,yRes);
        if(verifyResult(xRes,yRes))
        { 
          int output = (yRes * width + xRes) * 3; 
          pixmapComputed[input++] = pixmapOrig[output++];
          pixmapComputed[input++] = pixmapOrig[output++];
          pixmapComputed[input] = pixmapOrig[output];
        }
      }
    }
  }
}
void rotation(int angle,int xPivot,int yPivot)
{
  double theta = ((double)angle*3.1416)/180.0;
  double rotationMatrix[3][3], translationMatrix1[3][3], translationMatrix2[3][3];
  double pixelMatrix[3],resultMatrix[3],r1[3],r2[3];
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
      translationMatrix1[i][j]=0;
      translationMatrix2[i][j]=0;
      if(i==2 || j==2)
        rotationMatrix[i][j]=0;      
    }
    translationMatrix1[i][i]=1;
    translationMatrix2[i][i]=1;    
  }

  rotationMatrix[2][2]=1;
  rotationMatrix[0][0]=cos(theta);
  rotationMatrix[0][1]=(-1) * sin(theta);
  rotationMatrix[1][0]=sin(theta);
  rotationMatrix[1][1]=cos(theta);
  translationMatrix1[0][2]=xPivot;
  translationMatrix2[0][2]=-xPivot;
  translationMatrix1[1][2]=yPivot;
  translationMatrix2[1][2]=-yPivot;

  for(int y=height-1;y>=0;y--)
  {
    for(int x=0;x<width;x++)
    {
      initMatrix(pixelMatrix,x,height-y-1);
      multiplyMatrix(pixelMatrix, translationMatrix1, r1);
      multiplyMatrix(r1, rotationMatrix, r2);
      multiplyMatrix(r2, translationMatrix2, resultMatrix);
      int xRes, yRes;
      getValues(resultMatrix,xRes,yRes);
      if(verifyResult(xRes,yRes))
      {
        int input = ((height-y-1) * width + x) * 3;
        int output = (yRes * width + xRes) * 3; 
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output] = pixmapOrig[input];
      }
    }
  }   
  antiAliaseRotation(translationMatrix1, translationMatrix2, theta, xPivot, yPivot);
}




void antiAliaseScaling(double xScale, double yScale)
{
  double scalingMatrix[3][3];
  double pixelMatrix[3],resultMatrix[3];
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
        scalingMatrix[i][j]=0;      
    }
  }

  scalingMatrix[2][2]=1;
  scalingMatrix[0][0]=1/(double)xScale;
  scalingMatrix[1][1]=1/(double)yScale;
  for(int y=height-1;y>=0;y--)
  {
    for(int x=0;x<width;x++)
    {
      int input = ((height-y-1) * width + x) * 3;
      if(pixmapComputed[input] == 0 && pixmapComputed[input+1] == 0 && pixmapComputed[input+2] == 0)
      {
        initMatrix(pixelMatrix,x,height-y-1);
        multiplyMatrix(pixelMatrix, scalingMatrix, resultMatrix);
        int xRes, yRes;
        getValues(resultMatrix,xRes,yRes);
        if(verifyResult(xRes,yRes))
        { 
          int output = (yRes * width + xRes) * 3; 
          pixmapComputed[input++] = pixmapOrig[output++];
          pixmapComputed[input++] = pixmapOrig[output++];
          pixmapComputed[input] = pixmapOrig[output];
        }
      }
    }
  }
}


void scaling(double xScale,double yScale)
{
  double scalingMatrix[3][3];
  double pixelMatrix[3],resultMatrix[3];
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
        scalingMatrix[i][j]=0;      
    }
  }

  scalingMatrix[2][2]=1;
  scalingMatrix[0][0]=xScale;
  scalingMatrix[1][1]=yScale;

  for(int y=height-1;y>=0;y--)
  {
    for(int x=0;x<width;x++)
    {
      initMatrix(pixelMatrix,x,height-y-1);
      multiplyMatrix(pixelMatrix, scalingMatrix, resultMatrix);
      int xRes, yRes;
      getValues(resultMatrix,xRes,yRes);
      if(verifyResult(xRes,yRes))
      {
        int input = ((height-y-1) * width + x) * 3;
        int output = (yRes * width + xRes) * 3; 
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output] = pixmapOrig[input];
      }
    }
  }   
  antiAliaseScaling(xScale, yScale);

}




void antiAliaseShearing(double xShear, double yShear)
{
  double shearingMatrix[3][3];
  double pixelMatrix[3],resultMatrix[3];
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
        shearingMatrix[i][j]=0;      
    }
    shearingMatrix[i][i]=1;
  }
  
  shearingMatrix[2][2]=(double)(1-xShear*yShear);
  shearingMatrix[0][1]=(-1*xShear);
  shearingMatrix[1][0]=(-1*yShear);

  for(int y=height-1;y>=0;y--)
  {
    for(int x=0;x<width;x++)
    {
      int input = ((height-y-1) * width + x) * 3;
      if(pixmapComputed[input] == 0 && pixmapComputed[input+1] == 0 && pixmapComputed[input+2] == 0)
      {
        initMatrix(pixelMatrix,x,height-y-1);
        multiplyMatrix(pixelMatrix, shearingMatrix, resultMatrix);
        int xRes, yRes;
        getValues(resultMatrix,xRes,yRes);
        if(verifyResult(xRes,yRes))
        { 
          int output = (yRes * width + xRes) * 3; 
          pixmapComputed[input++] = pixmapOrig[output++];
          pixmapComputed[input++] = pixmapOrig[output++];
          pixmapComputed[input] = pixmapOrig[output];
        }
      }
    }
  }
}


void shearing(double xShear,double yShear)
{
  double shearingMatrix[3][3];
  double pixelMatrix[3],resultMatrix[3];
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
        shearingMatrix[i][j]=0;      
    }
    shearingMatrix[i][i]=1;
  }

  shearingMatrix[0][1]=xShear;
  shearingMatrix[1][0]=yShear;

  for(int y=height-1;y>=0;y--)
  {
    for(int x=0;x<width;x++)
    {
      initMatrix(pixelMatrix,x,height-y-1);
      multiplyMatrix(pixelMatrix, shearingMatrix, resultMatrix);
      int xRes, yRes;
      getValues(resultMatrix,xRes,yRes);
      if(verifyResult(xRes,yRes))
      {
        int input = ((height-y-1) * width + x) * 3;
        int output = (yRes * width + xRes) * 3; 
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output] = pixmapOrig[input];
      }
    }
  }   
  antiAliaseShearing(xShear, yShear);
}





int getxOffset(int mirrorOption, int x)
{
  if (mirrorOption==1)
    return width-x-1;
  return x;
}

int getyOffset(int mirrorOption, int y)
{
  if (mirrorOption==2)
    return height-y-1;
  return y;
}

void mirror(int mirrorOption)
{
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      int input = (y * width + x) * 3;
      int output = (getyOffset(mirrorOption,y) * width + getxOffset(mirrorOption,x)) * 3; 
      pixmapComputed[output++] = pixmapOrig[input++];
      pixmapComputed[output++] = pixmapOrig[input++];
      pixmapComputed[output] = pixmapOrig[input];
    }
  }
}




void translation(int xTranslate,int yTranslate)
{
  double translationMatrix[3][3];
  double pixelMatrix[3],resultMatrix[3],r1[3],r2[3];
  for(int i=0;i<3;i++)
  {
    for(int j=0;j<3;j++)
    {
      translationMatrix[i][j]=0;
    }
    translationMatrix[i][i]=1;   
  }

  translationMatrix[0][2]=xTranslate;
  translationMatrix[1][2]=yTranslate;

  for(int y=height-1;y>=0;y--)
  {
    for(int x=0;x<width;x++)
    {
      initMatrix(pixelMatrix,x,height-y-1);
      multiplyMatrix(pixelMatrix, translationMatrix, resultMatrix);
      int xRes, yRes;
      getValues(resultMatrix,xRes,yRes);
      if(verifyResult(xRes,yRes))
      {
        int input = ((height-y-1) * width + x) * 3;
        int output = (yRes * width + xRes) * 3; 
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output++] = pixmapOrig[input++];
        pixmapComputed[output] = pixmapOrig[input];
      }
    }
  }   
}



void perspective(int xPerspective,int yPerspective)
{

}
void applyTransformation(int option)
{
  switch(option)
  {
    case 1:
    int angle, xPivot, yPivot;
    cout<<"Enter the angle of rotation in degrees\n";
    cin>>angle;
    cout<<"Consider the image space with respect to origin (0,0) as:\n";
    cout<<"(0,"<<height<<")\t\t\t("<<width<<","<<height<<")\n\n\n(0,0)\t\t\t("<<width<<",0)\n";
    cout<<"Enter the x and y pivot values with respect to origin (0,0)\n";
    cin>>xPivot>>yPivot;
    rotation(angle,xPivot,yPivot);
    break;
    case 2:
    double xScale, yScale;
    cout<<"Enter the x and y scale values\n";
    cin>>xScale>>yScale;
    scaling(xScale,yScale);
    break;
    case 3:
    double xShear, yShear;
    cout<<"Enter the x and y shear values\n";
    cin>>xShear>>yShear;
    shearing(xShear,yShear);
    break;
    case 4:
    int mirrorOption;
    cout<<"Enter:\n1. Mirror along x\n2. Mirror long y\n";
    cin>>mirrorOption;
    mirror(mirrorOption);
    break;
    case 5:
    int xTranslate, yTranslate;
    cout<<"Enter the x and y translate values\n";
    cin>>xTranslate>>yTranslate;
    translation(xTranslate,yTranslate);
    break;
    case 6:
    int xPerspective, yPerspective;
    cout<<"Enter the x and y perspective values\n";
    cin>>xPerspective>>yPerspective;
    perspective(xPerspective,yPerspective);
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

  int option;
  cout<<"Enter options:\n1. Rotation\n2. Scaling\n3. Shearing\n4. Mirror\n5. Translation\n6. Perspective\n";
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
     

