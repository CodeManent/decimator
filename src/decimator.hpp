#ifndef __DECIMATOR_HPP_
#define __DECIMATOR_HPP_

#include <string>
#include <vector>

#include <CL/cl.hpp>

#include "object.hpp"
/*
	Structure used by OpenCL for the pointers from the vertices
	to the triangles
*/
struct arrayInfo{
	cl_uint position;
	cl_uint size;
	cl_uint continuesTo;
};

/*
	The class that handles the simplification of the model
*/
class Decimator{
	enum {SORT_ASCENDING = CL_TRUE, SORT_DESCENDING = CL_FALSE};

	bool runOnCPU;

	std::string kernelFilename;
	cl::Context *context;
	cl::CommandQueue *queue;
	cl::Device device;
	cl::Program *program;			// A pointer to the compiled program for the device that will be used

	cl_int maxWorkgroupSize;		// The maximum number og workgroups supported by the device
	cl_uint maxComputeUnits;		// The number of compute units of the device
	cl_uint maxVertexToIndices;		// The maximum number of triangles on which a vertex resides
	cl_ulong maxLocalSize;			// The size of the device's local memory

	cl_uint *iArray;	// Triangle array for simplification on the CPU
	cl_float *vArray;	// Vertex array for simplification on the CPU

	std::vector<cl::Memory> &glBuffers; // The buffer objects that will be used if the model resides on the GPU

	cl::Memory *glIndices;			// Memory buffer (OpenCL) for the triangles of the model
	cl::Memory *glVertices;			// Memory buffer for the vertices of the model

	cl::Buffer *triangleQuadrics;	// Memory buffer for the intermediate triangle quadrics
	cl::Buffer *quadrics;			// Memory buffer for the final triangle quadrics
	cl::Buffer *errorArray;			// Memory buffer for the error
	cl::Buffer *usedArray;			// Memory buffer that is used to find the independent points

	cl::Buffer *vertexToIndicesPointers; // Helper memory buffer to know at which point of the pointer array we must begin for each vertex
	 // TODO: Βοηθητικό memory buffer για να ξέρουμε σε ποιό σημείο του πίνακα των δεικτών πρέπει να ξεκινήσουμε για κάθε κορυφή
	cl::Buffer *vertexToIndicesData; // Memory buffer with the pointers from the vertices to the triangles

	double maxIndependentPointsToVertices; // The maximum number of independent vertices in relation to the number of vertices of the model
	float independentPointsPerPassFactor; // The percentage of the independent vertices that will bew used on each pass
    int independentPointsAlgorithm;	// The algorithm that will be used to find the independent vertices
	cl_int pointsFound;				// The number of the independent vertices found
	cl::Buffer *independentPoints;	// Memory buffer holding the independent vertices
	cl::Buffer *failedAttemptsBuffer;

	//initializes the Memory buffers
	cl_int initialiseBuffers		(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	//Calculates the pointers from the vertices to the triangles
	cl_int computeVertexToIndices	(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	//Calcuates the independent points
	cl_int computeIndependentPoints	(const Object &obj, unsigned int remainingVertices, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Initial serial algorithm that finds the independent points
	cl_int computeIndependentPoints1(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Independent points algorithm based ont the error
	cl_int computeIndependentPoints2(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// The final independent points algorithm
	cl_int computeIndependentPoints3(const Object &obj, unsigned int remainingVertices, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Computes the triangle quadrics
	cl_int computeTriangleQuadrics	(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Computes the vertices quadrics
	cl_int computeFinalQuadrics		(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Computes the error for each vertex
	cl_int computeDecimationError	(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Sorts the independent points based on the error
	cl_int sortDecimationError		(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Simplifies the model on the independent points
	cl_int decimateOnPoints			(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0, unsigned int * const verticesToTarget = 0);

	// Methods to validate the data (while executing on the CPU)
	cl_int validateData				(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);
	cl_int validateIndependentPoints(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Collects the resulting model from OpenCL
	cl_int collectResults			(Object &ret, const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);
	cl_int cleanup					(					const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Finda the maximum number of triangles that a vertex participates
	cl_int getMaxVertexToIndices(const Object &obj);

public:
	//Decimator();
	Decimator();
	~Decimator();

	void setRunOnCPU(bool b);
	void setIndependentPointsPerPassFactor(float f);
	void setKernelFilename(const char * str);
	void setIndependentPointsAlgorithm(int i);

	void initialise();
	void decimate(Object &obj, Object &newObject, unsigned int targetVertices);

	cl_int getWorkgroupSize(cl::Kernel &kernel, std::string funcName);
	cl_int getWorkSize(cl_uint actualSize, cl_int workgroupSize);
};

/*
	Checks the returned value from the OpenCL commands
*/
void clAssert(cl_int err, const char * msg = NULL);

#define debugWait(x) /* clAssert(queue->finish(), "finish queue: " #x); clAssert(x.wait(), "waitting: " #x)*/

#endif

