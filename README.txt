Alexandros Papageorgiou

1. Description
2. Program Structure
3. Dependencies
4. Execution

1. Description
============
This program was created as part of the thesis "Triangular mesh simplification on the GPU". The implementation is comprised of the parallel triangle simplification algorithm in OpenCL and the appropriate shell to handle and visualize the 3D models. The program reads a 3D models from a .ply file, simplifies it and can save it on a similar file.

2. Program structure
====================
The program is comprised of three parts:
|-Model
| =====
| |-object     representation of a model
| |-plyObject  A model that is read/written in a .ply file.
| \-rply       The library that us used to read.write the .ply files.
|
|-Viewer
| ======
| |-camera          Camera handling to allow viewing the model from multiple
| |                 angle
| |                 
| |-scene           The scene that holds the model (before and after the simplification)
| |                 
| \-glut_callbacks  Handles window and user input.
|
\-Decimator
  =========
  \-decimator  The class that drives the simplification of the model through OpenCL.
    |          The implementation (due to size) has been split into several files
    |          depending the functionality of each part:
    |          
    |-decimator.cpp                   Initialization, parameter setting and edge collapse
    |                                 
    |-decimatorPrepareData.cpp        Parts of the algorithm that are called before the 
    |                                 iterative structure, Allocates memory objects, computes
    |                                 the initial quadrics and the pointers from the vertices
    |                                 to the triangles
    |                                 
    |-decimatorIndependentPoints.cpp  Implementation of three algorithms that find
    |                                 independent vertices.
    |                                 
    |-decimatorSort.cpp               Sorts independent vertices using bitonic sort
    | 
    |-kernels.cl                      The implementation of the OpenCL kernels
    | 
    \-decimatorDataValidator.cpp      Data validations (while executing on the CPU)


3. Dependencies
===============
The dependencies (apart from the standard c++ libraries) are:
  OpenGL (libGL libGLU)
  GLEW (libGLEW)
  glut (libglut)
  OpenCL (libOpenCL)

4. Execution
===========
By passing the "--help" parameter to the program, the following help message is displayed

decimator inFile [-o outfile] [--overwrite] [--cw] [--ccw] [--antialiasing] [--gpu] [--cpu] [--kernels kernelsFile] [--target numberOfVertices] [--pointsPerPassFactor factor] [--independentPointsAlgorithm (1|2|3)] [--help]

    -o                              The output file
    --overwrite                     Overwrites the output file if exists
    --ccw                           Counterclockwise triangles in infile
    --cw                            clockwise triangles in infile
    --antialiasing                  Enable antialiasig in the display
    --gpu                           OpenCL run on GPU
    --cpu                           openCL runs on cpu
    --kernels                       Te location of the kernels file
    --target                        The number of vertices for the result of the decimation ( > 0 )
    --pointsPerPassFactor           Percentage of the independent points that are used at every pass of the decimation algorithm
    --independentPointsAlgorithm    The algorithm to be used
    --help                          This Message

Default Values
--------------
run on: gpu
triangle orientation: ccw
kernels: kernels.cl
target 0.5
pointsPerPassFactor: 0.85
independentPointsAlgorithm: 3

The most significant parameters that are needed are the path to the model and the simplification target. For example to simplify the model stored in the file horse.ply to 5000 vertices we call the program with the following command:

decimator horse.ply --target 5000

As soon as the program loads the model, it creates a window and shows the model in its initial state. By pressing the button 'd' we initiate the simplification. As soon as it is completed, the simplified model is presented in the window replacing the original model.

If we want to save the model, we must first pass the '-o' parameter followed by the path of the output file (and possibly the --overwrite parameter).After we simplify the model, we can save it by pressing the 's' button.

