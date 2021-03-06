#define NOMINMAX
//#define WINDOWS_TIMING

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <memory>

#include <CL/cl.hpp>
#include <CL/cl_gl.h>


#include "decimator.hpp"

Decimator::Decimator():
	runOnCPU(false),
	kernelFilename("kernels.cl"),
	context(NULL),
	queue(NULL),
	program(NULL),
	maxWorkgroupSize(1),
	iArray(NULL),
	vArray(NULL),
	glBuffers(*(new std::vector<cl::Memory>())),
	glIndices(NULL),
	glVertices(NULL),
	triangleQuadrics(NULL),
	quadrics(NULL),
	errorArray(NULL),
	usedArray(NULL),
    maxIndependentPointsToVertices(0.05),
    independentPointsPerPassFactor(0.2f),
    independentPointsAlgorithm(3)
{
}

 /**********************************************************
 Helper function that is called from the OpenCL runtime incase 
 of an error and through which the driver passes some 
 relevant information.
 **********************************************************/
void CL_CALLBACK notifyFunction(const char *msg, const void* , ::size_t , void* )
{
	std::cerr << "Error notification: " << msg;
}



Decimator::~Decimator()
{
	delete &glBuffers;
}




 /**********************************************************
 Sets the variable that specifiec the execution platform
 **********************************************************/
void Decimator::setRunOnCPU(bool b)
{
	runOnCPU = b;
}




 /**********************************************************
 Sets the variable that specifies the percentile of the independent
 vertices that will be used during the decimation
 **********************************************************/
void Decimator::setIndependentPointsPerPassFactor(float f)
{
	if(f > 0.0f && f <= 1.0f)
	{
		independentPointsPerPassFactor = f;
	}
}




 /**********************************************************
 Sets the variable that specifies the path for the kernels
 **********************************************************/
void Decimator::setKernelFilename(const char *str)
{
	kernelFilename = str;
}




 /**********************************************************
 Sets the variable that specifes the sorting algorithm
 **********************************************************/
void Decimator::setIndependentPointsAlgorithm(int i)
{
	if( i < 1 || i > 3)
	{
		throw(std::out_of_range("Decimator::setIndependentPointsAlgorithm: Bad algorithm number"));
		//return;
	}
	else
	{
		independentPointsAlgorithm = i;
	}
}




/*
	Decimator initialization
	
	

	Παίρνουμε τις OpenCL platforms που υπάρχουν στον υπολογιστή.
	Σε κάθε μία διατρέουμε τις συσκευές μέχρι να βρούμε κάποια
	που να μπορέσουμε να δημιουργήσουμε context. Διανάζουμε τον
	κώδικα με τους kernels από το αρχείο και τον μεταγλωττίζουμε
	για τη συσκευή που θα χρησιμοποιήσουμε. Τέλος δημιουργούμε
	μια ουρά εκτέλεσης με βάση το context που έχουμε και τη
	συσκευή που θα χρησιμοποιήσουμε.
*/
void Decimator::initialise(){
	cl_int err = CL_SUCCESS;
	unsigned int pos = 0;
	cl_device_type deviceType = 0;
	cl_context_properties cprops[10];

	std::vector<cl::Platform> platformList;

	// get the platform list
	cl::Platform::get(&platformList);
	if(platformList.size() == 0)
	{
		throw(std::runtime_error("Decimator::initialise: Couldn't find an OpenCL platform"));
	}

	deviceType = runOnCPU ? CL_DEVICE_TYPE_CPU : CL_DEVICE_TYPE_GPU;

	std::vector<cl::Device> devices;
	for(unsigned int i = 0; i < platformList.size(); ++i)
	{
		err = platformList[i].getDevices(deviceType, &devices);

		if(err == CL_DEVICE_NOT_FOUND)
		{
			continue;
		}
		else
		{
			clAssert(err, "Decimator::initialise: getDevices");
		}

		if(devices.size() == 0)
		{
			continue;
		}

		pos = 0;
		cprops[pos] = CL_CONTEXT_PLATFORM; ++pos;
		cprops[pos] = (cl_context_properties) (platformList[i])(); ++pos;


		cprops[pos] = 0; ++pos;

		if(context != NULL)
		{
			delete context;
		}
		context = new cl::Context(devices, cprops, notifyFunction, 0, &err);
		clAssert(err, "Decimator::initialise: creating context");

		break;
	}

	if(devices.size() == 0)
	{
		throw(std::runtime_error("Decimator::initialise: Couldn't create an openCL context"));
	}

	// read the source code from the file
	std::ifstream file(this->kernelFilename.c_str(), std::ios_base::in);
	if(!file) {
		throw std::runtime_error("Failed to open the kernels file: " + this->kernelFilename);
	}

	std::string kernelSource(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));

	if(!kernelSource.length()) {
		throw std::runtime_error("Kernles file is empty: " + this->kernelFilename);
	}

	// get the spurce code and compile it
	cl::Program::Sources source(1, std::make_pair(kernelSource.c_str(), kernelSource.length()));

	//cl::Program program(context, source);
	if(program != NULL){
		delete program;
		program = NULL;
	}

	if(program != NULL)
	{
		delete program;
	}
	program = new cl::Program(*context, source, &err);
	clAssert(err, "Decimator::initialise: Creating the program object");

//	if(runOnCPU)
//	{
		//build with debug information
//		err = program->build(devices, " -g -s /home/oriong/projects/decimator/linux/kernels.cl");
//	}
//	else
//	{
		err = program->build(devices);
//	}

	if(err != CL_SUCCESS){
		cl_int build_log_err = CL_SUCCESS;

		//get the build log
		std::string buildLog = program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0], &build_log_err);
		clAssert(build_log_err, "Decimator::initialise: Retrieving the build log after failed build");

		std::cerr << buildLog << std::endl;
	}
	clAssert(err, "Decimator::initialise: Build failed");
	// std::clog << "Decimator::initialise: Build success" << std::endl;

	if(queue != NULL){
		delete queue;
	}

	this->queue = new cl::CommandQueue(*context, devices[0], CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE /* | CL_QUEUE_PROFILING_ENABLE*/, &err);
	clAssert(err, "Decimator::initialise: Creating command queue");

	maxWorkgroupSize = (cl_int) devices[0].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
	maxComputeUnits = devices[0].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
	maxLocalSize = devices[0].getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();

	this->device = devices[0];
}





 /**********************************************************
 The method that is used to simplify a model
 
 Here we have the general structure of the algorithm.
Each part is implemented on the relevant methods which call
the appropriate kernels 
 **********************************************************/
void Decimator::decimate(Object &obj, Object &newObject, unsigned int targetVertices){
	unsigned int verticesToTarget = (unsigned int) obj.vertices.size() - targetVertices;

	cl::Event
		initialiseArraysEvent,
		computeVTIEvent,
		computeIndependentPointsEvent,
		computeQuadricsEvent,
		computeFinalQuadricsEvent,
		computeDecimationErrorEvent,
		sortEvent,
		decimateOnPointsEvent,
		collectResultsEvent
		;

	std::vector<cl::Event> waitVector;



    if(obj.indices.size() == 0)
    {
        throw(std::invalid_argument("Decimator::decimat: Source object has no triangles"));
    }

	int iteration = 0;

	try{
		//std::cout << "initailise\n";
		waitVector.clear();
		initialiseBuffers(obj, 0,  &initialiseArraysEvent);

		debugWait(initialiseArraysEvent);

		//std::cout << "computeVertexToIndices\n";
		waitVector.clear();
		waitVector.push_back(initialiseArraysEvent);
		computeVertexToIndices(obj, &waitVector, &computeVTIEvent);

		debugWait(computeVTIEvent);

		//std::cout << "computeTriangleQuadrics\n";
		waitVector.clear();
		waitVector.push_back(initialiseArraysEvent);
		computeTriangleQuadrics(obj, &waitVector, &computeQuadricsEvent);

		debugWait(computeQuadricsEvent);

		//std::cout << "computeFinalQuadrics\n";
		waitVector.clear();
		waitVector.push_back(computeQuadricsEvent);
		waitVector.push_back(computeVTIEvent);
		computeFinalQuadrics(obj, &waitVector, &computeFinalQuadricsEvent);

		debugWait(computeFinalQuadricsEvent);

		//std::cout << "computeDecimationError\n";
		waitVector.clear();
		waitVector.push_back(computeFinalQuadricsEvent);
		computeDecimationError(obj, &waitVector, &computeDecimationErrorEvent);

		debugWait(computeDecimationErrorEvent);

		waitVector.clear();
		waitVector.push_back(computeVTIEvent);

		do
		{
			iteration++;
/*
			//debug - data validation
			cl::Event validateEvent;
			if(runOnCPU)
			{
				validateData(obj, &waitVector, &validateEvent);
				waitVector.clear();
				waitVector.push_back(validateEvent);
			}
//*/

			//std::cout << "computeIndependentPoints\n";
			computeIndependentPoints(obj,targetVertices + verticesToTarget, &waitVector, &computeIndependentPointsEvent);
			waitVector.clear();
			waitVector.push_back(computeIndependentPointsEvent);
			if(iteration == 1)
			{
				waitVector.push_back(computeDecimationErrorEvent);
			}
			//std::cout << "pointsFound = " << pointsFound << std::endl;
			debugWait(computeIndependentPointsEvent);

//*
			if(this->pointsFound > 1)
			{
                //std::clog << "sortDecimationError" << std::endl;
				sortDecimationError(obj, &waitVector, &sortEvent);
				waitVector.clear();
				waitVector.push_back(sortEvent);

				debugWait(sortEvent);
			}
			else if(this->pointsFound == 0)
			{
				break;
			}
//*/
/*
            if(runOnCPU)
            {
                validateIndependentPoints(obj,&waitVector);
            }
//*/
			try{
				//std::clog << "decimateOnPoints" << std::endl;
				decimateOnPoints(obj, &waitVector, &decimateOnPointsEvent, &verticesToTarget);
			}
			catch(std::underflow_error &e)
			{
				std::cout << " stopping decimation: " << e.what() << std::endl;
				break;
			}

			waitVector.clear();
			waitVector.push_back(decimateOnPointsEvent);

			debugWait(decimateOnPointsEvent);

            //break;
		}while(verticesToTarget != 0);



        //std::clog << "collect" << std::endl;
		collectResults(newObject, obj, &waitVector, &collectResultsEvent);
		waitVector.clear();
		waitVector.push_back(collectResultsEvent);

		// std::clog << "iterations: " << iteration << std::endl;
	}
	catch(std::exception &e)
	{
        std::cerr << "Decimation error: " << e.what() << std::endl;
		try{
			cleanup(&waitVector);
		}
		catch(std::exception &e2)
		{
            std::cerr << "Cleanup error: " << e2.what() << std::endl;
			throw;
		}
		throw;
	}

	try{

		cleanup(&waitVector);
	}
	catch(std::exception &e)
	{
        std::cerr << "Cleanup error: " << e.what() << std::endl;
		throw;
	}

	std::cout << "target: " << targetVertices << '\t'
	          << "result: " << newObject.vertices.size() << '\t'
	          << "iterations: " << iteration << '\t'
			  << std::endl;
	queue->finish();
}

 /**********************************************************
 The method that drives the edge collapse
 
 The number of the vertices that wil be used is computed using 
 the independentPointsPerPassFactor var.
 
 The kernel that performs the collapses is called  decimateOnPoint.
 
 As soon as the kernel finishes its execution, we reaf the memory
 buffer for the number of the failed collapses. If all the collapses
 fail an exception is raised.
 **********************************************************/
cl_int Decimator::decimateOnPoints(const Object &, const std::vector<cl::Event> *const waitVector, cl::Event *const returnedEvent, unsigned int *const verticesToTarget)
{
	//clAssert(queue->flush(), "flushing queue");
	debugWait((*waitVector)[0]);

	cl_int err = CL_SUCCESS;

	cl_uint pointsToDecimate = (cl_uint)ceil(((float)pointsFound*independentPointsPerPassFactor));
	cl::Event decimateOnPointEvent, readEvent;

	if(verticesToTarget)
	{
		pointsToDecimate = std::min(pointsToDecimate, *verticesToTarget);
	}
    //std::clog <<"workSize: " << workSize << std::endl;
	cl_uint failed = 0;

	cl::Event writeEvent;
	queue->enqueueWriteBuffer(*failedAttemptsBuffer, CL_FALSE, 0, sizeof(failed), &failed, waitVector, &writeEvent);
	clAssert(err, "Decimator::decimateOpPoints: initializing failedAttemptsBuffer");
    std::vector<cl::Event> writeInternalWaitVector(1, writeEvent);



	cl::Kernel decimateOnPoint(*program, "decimateOnPoint", &err);
	clAssert(err, "Decimator::decimateOnPoints: Creating kernel");

	unsigned int ac = 0;	// argument counter
	err |= decimateOnPoint.setArg(ac++, *glVertices);
	err |= decimateOnPoint.setArg(ac++, *glIndices);
	err |= decimateOnPoint.setArg(ac++, *quadrics);
	err |= decimateOnPoint.setArg(ac++, *errorArray);
	err |= decimateOnPoint.setArg(ac++, *independentPoints);
	err |= decimateOnPoint.setArg(ac++, *vertexToIndicesPointers);
	err |= decimateOnPoint.setArg(ac++, *vertexToIndicesData);
	err |= decimateOnPoint.setArg(ac++, maxVertexToIndices);
	err |= decimateOnPoint.setArg(ac++, pointsToDecimate);
	err |= decimateOnPoint.setArg(ac++, *failedAttemptsBuffer);
	clAssert(err, "Decimator::decimateOnPoints: Adding kernel arguments");

	cl_int workgroupSize = getWorkgroupSize(decimateOnPoint, "decimateOnPoint");
	cl_int workSize = getWorkSize(pointsToDecimate ,workgroupSize);

    //std::clog << "Decimator::decimateOnPoints: calling kernel \"decimateOnPoint" << std::endl;
	//std::clog << "DecimateOnPoints: " << pointsToDecimate << "(" << workSize << ")" <<  "/" << pointsFound  <<" points" << std::endl;

	err = queue->enqueueNDRangeKernel(
		decimateOnPoint,
		cl::NullRange,
		cl::NDRange(workSize),
		cl::NullRange,// cl::NDRange(workgroupSize),
		&writeInternalWaitVector,
		&decimateOnPointEvent);

	clAssert(err, "Decimator::decimateOnPoints: Adding kernel to queue");

	debugWait(decimateOnPointEvent);


    std::vector<cl::Event> internalWaitVector(1, decimateOnPointEvent);
	cl_uint failedAttempts = 0;
    err = queue->enqueueReadBuffer(*failedAttemptsBuffer, CL_TRUE, 0, sizeof(cl_uint), &failedAttempts, &internalWaitVector, &readEvent);
	clAssert(err, "Decimator::decimateOnPoints: Error reading failed attempts buffer");

	debugWait(readEvent);

	if(failedAttempts == pointsToDecimate)
	{
		std::cerr << "failedAttempts = " << failedAttempts << " \tpointsToDecimate = " << pointsToDecimate << std::endl;
		throw(std::underflow_error("Decimator::decimateOnPoints: Cuuldn't perform decimation an any one of the selected points"));
	}

	if(verticesToTarget)
	{
		*verticesToTarget -= pointsToDecimate;
		*verticesToTarget += failedAttempts;
	}

	if(returnedEvent)
	{
		*returnedEvent = decimateOnPointEvent;
	}
	else{
		err = decimateOnPointEvent.wait();
	}


	return err;
}

 /**********************************************************
 Collection of the resulting simplified model
 
 We get the triangles and the verices of the model from the 
 OpenCL memoty buffers. 
 The next step is to filter then to remove the vertices and
 triangles that have been marked as deleted.
 The final simplified model is placed in ret.
 **********************************************************/
cl_int Decimator::collectResults(Object &ret, const Object &obj, const std::vector<cl::Event> *const waitVector , cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;
	size_t vertices = obj.vertices.size();
	size_t indices = obj.indices.size();
	cl_float *newVertices = new cl_float[vertices*3];
	unsigned int *relocationData = new unsigned int[vertices];
	cl_uint *newIndices = new cl_uint[indices*3];
	cl::Event readVerticesEvent, readIndicesEvent;

	err = queue->enqueueReadBuffer(*(static_cast<cl::Buffer *>(glIndices)), CL_TRUE, 0, sizeof(cl_uint)*3*indices, newIndices, waitVector, &readIndicesEvent);
	clAssert(err, "Decimator::collectCPUResults: Reading indices from OpenCL");

	err = queue->enqueueReadBuffer(*(static_cast<cl::Buffer *>(glVertices)), CL_TRUE, 0, sizeof(cl_float)*3*vertices, newVertices, waitVector, &readVerticesEvent);
	clAssert(err, "Decimator::collectCPUResults: Reading vertices from OpenCL");



	err = readVerticesEvent.wait();
	clAssert(err, "Decimator::collectCPUResults: Waitting to read indices from OpenCL");

	err = readIndicesEvent.wait();
	clAssert(err, "Decimator::collectCPUResults: Waitting to read vertices from OpenCL");

    //std::clog << "Reconstructing Object" << std::endl;
	unsigned int discardedVertices = 0;
	for(size_t i = 0; i < vertices; ++i){
		if(newVertices[3*i] == CL_MAXFLOAT){
			++discardedVertices;
			relocationData[i] = CL_UINT_MAX;
			continue;
		}

		relocationData[i] = discardedVertices;
		ret.addVertex(point3f( newVertices + 3*i ));
	}

	unsigned int discardedTriangles = 0;
	for(size_t i =0; i < indices; ++i)
	{
		
		if(newIndices[3*i] == CL_UINT_MAX){
			++discardedTriangles;
			continue;
		}

		point3ui newTriangle(newIndices + 3*i);

		newTriangle.x -= relocationData[newTriangle.x];
		newTriangle.y -= relocationData[newTriangle.y];
		newTriangle.z -= relocationData[newTriangle.z];


		ret.addTriangle(newTriangle);
	}

    // std::clog << "discardedVertices: " << discardedVertices << std::endl;
    // std::clog << "skippedTriangles: " << discardedTriangles << std::endl;

	delete[] newVertices;
	delete[] relocationData;
	delete[] newIndices;

	if(returnedEvent)
	{
		*returnedEvent = readIndicesEvent;
	}

	return err;
}

 /**********************************************************
 Frees the allocated resources
 **********************************************************/
cl_int Decimator::cleanup(const std::vector<cl::Event> *const waitVector, cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;
	cl::Event releaseObjectsEvent;

	if(!runOnCPU && glBuffers.size() != 0)
	{
	/**/err = queue->enqueueReleaseGLObjects(&glBuffers, waitVector, &releaseObjectsEvent);
		clAssert(err, "Decimator::cleanup: Releasing GL Objects");

		glBuffers.clear();
	}
	else
	{
		err = cl::Event::waitForEvents(*waitVector);
		clAssert(err, "Decimator::cleanup: Waitting for events");
	}


	if(glIndices)			{	delete glIndices;		glIndices = NULL;			}
	if(glVertices)			{	delete glVertices;		glVertices = NULL;			}
	if(iArray)				{	delete[] iArray;		iArray = NULL;				}
	if(vArray)				{	delete[] vArray;		vArray = NULL;				}
	if(triangleQuadrics)	{	delete triangleQuadrics; triangleQuadrics = NULL;	}
	if(quadrics)			{	delete quadrics;		quadrics = NULL;			}
	if(errorArray)			{	delete errorArray;		errorArray = NULL;			}
	if(usedArray)			{	delete usedArray;		usedArray = NULL;			}

	if(vertexToIndicesData)	{	delete vertexToIndicesData; vertexToIndicesData = NULL;}
	if(vertexToIndicesPointers){delete vertexToIndicesPointers; vertexToIndicesPointers = NULL;	}

	if(independentPoints)	{	delete independentPoints;	independentPoints = NULL;	}
	if(failedAttemptsBuffer){	delete failedAttemptsBuffer; failedAttemptsBuffer = NULL; }


	queue->finish();

	if(returnedEvent){
		*returnedEvent = releaseObjectsEvent;
	}
	else{
		err = releaseObjectsEvent.wait();
	}
	return err;
}

 /**********************************************************
 Helper function that checkes the value which is returned by 
 OpenCL calls.

 In case of an error, an exception is raised with the relevant 
 message
 **********************************************************/
void clAssert(cl_int err, const char *msg)
{
	if(err == CL_SUCCESS)
	{
		return;
	}

	const char *err_str = NULL;
	switch (err) {
#define ERRCASE(x) case x: err_str = #x; break
		ERRCASE(CL_SUCCESS);
		ERRCASE(CL_DEVICE_NOT_FOUND);
		ERRCASE(CL_DEVICE_NOT_AVAILABLE);
		ERRCASE(CL_COMPILER_NOT_AVAILABLE);
		ERRCASE(CL_MEM_OBJECT_ALLOCATION_FAILURE);
		ERRCASE(CL_OUT_OF_RESOURCES);
		ERRCASE(CL_OUT_OF_HOST_MEMORY);
		ERRCASE(CL_PROFILING_INFO_NOT_AVAILABLE);
		ERRCASE(CL_MEM_COPY_OVERLAP);
		ERRCASE(CL_IMAGE_FORMAT_MISMATCH);
		ERRCASE(CL_IMAGE_FORMAT_NOT_SUPPORTED);
		ERRCASE(CL_BUILD_PROGRAM_FAILURE);
		ERRCASE(CL_MAP_FAILURE);
		ERRCASE(CL_INVALID_VALUE);
		ERRCASE(CL_INVALID_DEVICE_TYPE);
		ERRCASE(CL_INVALID_PLATFORM);
		ERRCASE(CL_INVALID_DEVICE);
		ERRCASE(CL_INVALID_CONTEXT);
		ERRCASE(CL_INVALID_QUEUE_PROPERTIES);
		ERRCASE(CL_INVALID_COMMAND_QUEUE);
		ERRCASE(CL_INVALID_HOST_PTR);
		ERRCASE(CL_INVALID_MEM_OBJECT);
		ERRCASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
		ERRCASE(CL_INVALID_IMAGE_SIZE);
		ERRCASE(CL_INVALID_SAMPLER);
		ERRCASE(CL_INVALID_BINARY);
		ERRCASE(CL_INVALID_BUILD_OPTIONS);
		ERRCASE(CL_INVALID_PROGRAM);
		ERRCASE(CL_INVALID_PROGRAM_EXECUTABLE);
		ERRCASE(CL_INVALID_KERNEL_NAME);
		ERRCASE(CL_INVALID_KERNEL_DEFINITION);
		ERRCASE(CL_INVALID_KERNEL);
		ERRCASE(CL_INVALID_ARG_INDEX);
		ERRCASE(CL_INVALID_ARG_VALUE);
		ERRCASE(CL_INVALID_ARG_SIZE);
		ERRCASE(CL_INVALID_KERNEL_ARGS);
		ERRCASE(CL_INVALID_WORK_DIMENSION);
		ERRCASE(CL_INVALID_WORK_GROUP_SIZE);
		ERRCASE(CL_INVALID_WORK_ITEM_SIZE);
		ERRCASE(CL_INVALID_GLOBAL_OFFSET);
		ERRCASE(CL_INVALID_EVENT_WAIT_LIST);
		ERRCASE(CL_INVALID_EVENT);
		ERRCASE(CL_INVALID_OPERATION);
		ERRCASE(CL_INVALID_GL_OBJECT);
		ERRCASE(CL_INVALID_BUFFER_SIZE);
		ERRCASE(CL_INVALID_MIP_LEVEL);
		ERRCASE(CL_INVALID_GLOBAL_WORK_SIZE);
#undef ERRCASE
	default:								err_str =  "Unknown";
	}

	std::stringstream ss;
	ss << (msg == NULL ? "" : msg) << " (" << err << "): " << err_str;
	throw(std::runtime_error(ss.str()));

}

cl_int Decimator::getWorkgroupSize(cl::Kernel & kernel, std::string funcName){
	cl_int err = CL_SUCCESS;
	cl_int workgroupSize = 128;

	workgroupSize = (cl_int) kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);
//	prefSize = (cl_int) kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device, &err);
	//patch for intel opencl implementation
//	workgroupSize = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(&err);
//	workgroupSize = 1024;

	clAssert(err, ("Decimator::" + funcName + ": Getting workgroup info").c_str());

	workgroupSize = std::min(workgroupSize, maxWorkgroupSize);

	//std::cerr << funcName << ": kernel workgroup size: " << workgroupSize << std::endl;

	return workgroupSize;
}

cl_int Decimator::getWorkSize(cl_uint actualSize, cl_int workgroupSize){
	return actualSize + (workgroupSize - actualSize % workgroupSize)%workgroupSize;
}

