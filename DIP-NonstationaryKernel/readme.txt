Problem Description:

You will develop non-stationary filters that can be applied to an image file. The goal of this project is to control filter kernels for each pixel to obtain aestetic results. Our goal is to implement something that you cannot directly create by using any stationary-filters. So, in this case we will dynamically create filters for each pixel. So, your filters will not simply use static kernels.

Project Requirements:
You are supposed to implement two of the following filters to get the full credit. Each one of the is equally weighted.

    Convolution filter - Motion Blur. In this case the direction and the size of the motion blur will be controlled by a vector field. (A Simple way of creating a vector field is to use an image. For instance, red and green channels of an image can be converted into x,y by a simple linear transformation.). This will give you look of Line Integral Convolution.
    Morphological Non-Stationary filter - In this case the size and shape of the Dilation and/or Erosion filter will be controlled by another image. 

Bonus: Any additional work can give you bonus points up to half of the project credit. For instance, you can create Smart Blur or Bi-directional Low-Pass filter.

For the required parts of the project, implement your operations only using the basic programming operations such as while or for loops and basic mathematical operations such as addition, subtraction, multiplication, division and power. In other words, no high level operation provided by some programming languages such as convolution is allowed.

Project Submission:

Please write the program in either Processing or Java or C or C++. For C and C++ use OpenGL and GLUTgraphics routines for the display. Upload your program and all essential files to webassign as a "as small as possible" zip directory. In your program, include comments about the program and your name. Also make sure to provide information that and instructions on how to run it.

