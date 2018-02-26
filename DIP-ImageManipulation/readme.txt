Problem Description:

The goal of this project is to learn global manipulations and use them to create aestetic results. Most of these operations can be seen under Image/Adjustments in Photoshop such as "Curves, Posterize or Replace Color" operations or in filters. You will also learn how to use parametric functions.

Project Requirements:
The three tasks in this case are the following. Our goal is to implement something that you cannot create by using commercial software. In each case, provide start with a standard approach and create your own using an unconventional input such an image.

    Manipulate colors in an image using an interpolating cubic curve. Curve parameters will be given as a list of numbers such as (r0,r1,r2,r3...,rn) which means a parametric curve that passes through from points (0,r0), (1/n,r1), (2/n,r2), (3/n,r3) and (1,rn). We suggest to start with piecewise-linear curve and extend to cubics later. If you finish piecewise-linear, you will get half-credit.
    Replace Hues in an image taking hues from another image. For the information about how to compute hues, see below.

Bonus: An curve interface can give you bonus points up to half of the project credit.

For the required parts of the project, implement your operations only using the basic programming operations such as while or for loops and basic mathematical operations such as addition, subtraction, multiplication, division and power. In other words, no high level operation provided by some programming languages such as HSV to RGB conversion is allowed.

Project Submission:

Please write the program in either Processing or Java or C or C++. For C and C++ use OpenGL and GLUTgraphics routines for the display. Upload your program and all essential files to webassign as a "as small as possible" zip directory. In your program, include comments about the program and your name. Also make sure to provide information that and instructions on how to run it. 
