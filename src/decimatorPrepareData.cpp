#define NOMINMAX

#include <vector>
#include <algorithm>
#include <iostream>

#include "decimator.hpp"

 /**********************************************************
 Αρχικοποιεί τα memoty buffers.

 ’ν τρέχει στην CPU δημιουργεί αντίγραφα του μοντέλου σε
 νέους πίνακες αλλίως χργσιμοποιεί τα buffer objects της 
 OpenGL. Δημιουργεί τα buffer objects που θα χρησιμοποιηθούν
 κατά την εκτέλεση του αλγορίθμου απλοποίησης και τα
 αρχικοποιεί καλώντας τον kernel intialiseArrays.
 **********************************************************/
cl_int Decimator::initialiseBuffers(const Object &obj, const std::vector<cl::Event> *const waitVector, cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;
	cl_uint  indices = (cl_int) obj.indices.size();
	cl_uint vertices = (cl_int) obj.vertices.size();
	//cl_uint workSizePadding;

	std::vector<cl::Event> internalWaitVector;

	cl::Event
		acquireObjectsEvent,
		initialiseArraysEvent
		;

	internalWaitVector.clear();

	// fill the buffers with data from the object
	iArray = new cl_uint[indices*3];
	for(size_t i = 0; i< indices; ++i){
		iArray[3*i] = obj.indices[i].x;
		iArray[3*i+1] = obj.indices[i].y;
		iArray[3*i+2] = obj.indices[i].z;
	}

	vArray = new cl_float[vertices*3];
	for(size_t i = 0; i < vertices; ++i){
		vArray[3*i] = obj.vertices[i].x;
		vArray[3*i+1] = obj.vertices[i].y;
		vArray[3*i+2] = obj.vertices[i].z;
	}

	glIndices = new cl::Buffer(*context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_uint)* 3 *indices, iArray, &err);
	clAssert(err, "Decimator::initialiseBuffers: Creating indices buffer");

	glVertices = new cl::Buffer(*context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_float)* 3 *vertices, vArray, &err);
	clAssert(err, "Decimator::initialiseBuffers: Creating vertices buffer");


	triangleQuadrics= new cl::Buffer(*context, CL_MEM_READ_WRITE, sizeof(cl_double)*16*indices, 0, &err);
	clAssert(err, "Decimator::initialiseBuffers: Creating triangle quadrics buffer");

	quadrics = new cl::Buffer(*context, CL_MEM_READ_WRITE , sizeof(cl_double16) * vertices, 0, &err);
	clAssert(err, "Decimator::initialiseBuffers: Creating quadrics buffer");

	errorArray = new cl::Buffer(*context, CL_MEM_READ_WRITE, sizeof(cl_double)*vertices, 0, &err);
	clAssert(err, "Decimator::initialiseBuffers: Creating error array buffer");

	maxVertexToIndices = getMaxVertexToIndices(obj);

	vertexToIndicesPointers = new cl::Buffer(*context, CL_MEM_READ_WRITE, sizeof(arrayInfo)*vertices,0, &err);
	clAssert(err, "Decimator::initialiseBuffers: Creating vertexToIndicesPointers buffer");

	vertexToIndicesData = new cl::Buffer(*context, CL_MEM_READ_WRITE, sizeof(cl_uint)*vertices * maxVertexToIndices, 0, &err);
	clAssert(err, "Decimator::initialiseBuffers: Creating vertexToIndicesData buffer");

	usedArray = new cl::Buffer(*context, CL_MEM_READ_WRITE, sizeof(cl_uint) * vertices, 0, &err);
	clAssert(err, "Decimator::initialiseBuffers: Creating usedArray buffer");

	independentPoints = new cl::Buffer(*context, CL_MEM_READ_WRITE,(size_t)( sizeof(cl_uint) * vertices * maxIndependentPointsToVertices), 0, &err);
	clAssert(err, "Decimator::initialiseBuffers: Creating independentPoints buffer");



	//initialise arrays

	cl::Kernel initialiseArrays(*program, "intialiseArrays", &err);
	clAssert(err, "Decimator::initialiseBuffers: Creating kernel");

	cl_uint ac = 0;
	err  = initialiseArrays.setArg(ac++, *quadrics);
	err |= initialiseArrays.setArg(ac++, *errorArray);
	err |= initialiseArrays.setArg(ac++, *vertexToIndicesPointers);
	err |= initialiseArrays.setArg(ac++, maxVertexToIndices);
	err |= initialiseArrays.setArg(ac++, vertices);
	clAssert(err, "Decimator::initialiseBuffers: Adding parameters to kernel");


	cl_int workgroupSize = (cl_int) initialiseArrays.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);
	clAssert(err, "Decimator::initialiseBuffers: Getting workgroup info");
	workgroupSize = std::min(workgroupSize, maxWorkgroupSize);
	cl_int workSize = vertices + (workgroupSize - vertices % workgroupSize)%workgroupSize;
	

	err = queue->enqueueNDRangeKernel(
		initialiseArrays,
		cl::NullRange,
		cl::NDRange(workSize),
		cl::NDRange(workgroupSize),
		&internalWaitVector,
		&initialiseArraysEvent
		);
	clAssert(err, "Decimator::initialiseBuffers: Adding kernel to queue");

	if(returnedEvent){
		*returnedEvent = initialiseArraysEvent;
	}
	else{
		err = initialiseArraysEvent.wait();
		clAssert(err, "Decimator::computeVertexToIndices: Waitting for the completion event");
	}

	return err;
}




 /**********************************************************
 Διατρέχει το μοντέλο και βρίσκει τον μέγιστο αριθμό τριγώνων
 στα οποία μπορεί να συμμετέχει μια κορυφή.
 **********************************************************/
cl_int Decimator::getMaxVertexToIndices(const Object &obj)
{
	int *const accumulator = new int[obj.vertices.size()];
	int max = 0;

	for(size_t i = 0; i < obj.vertices.size(); ++i)
	{
		accumulator[i] = 0;
	}

	for(size_t i = 0; i < obj.indices.size(); ++i)
	{
		accumulator[obj.indices[i].x] += 1;
		accumulator[obj.indices[i].y] += 1;
		accumulator[obj.indices[i].z] += 1;
	}

	for(size_t i = 0; i < obj.vertices.size(); ++i)
	{
		max = accumulator[i] > max ? accumulator[i] : max;
	}

	delete accumulator;

	return max;
}





 /**********************************************************
 Υπολογίζει τα δεδομένα της δομής που περιέχει δείκτες από
 τις κορυφές προς τα τρίγωνα.
 **********************************************************/
cl_int Decimator::computeVertexToIndices(const Object &obj, const std::vector<cl::Event> *const waitVector, cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;
	cl_uint indices = (cl_int) obj.indices.size();
	cl_uint workSizePadding = 0;
	cl::Event computeVTIEvent;


	cl::Kernel computeVTI (*program, "computeVertexToIndices", &err);
	clAssert(err, "Decimator::computeVertexToIndices: Creating kernel");

	err =  computeVTI.setArg(0, *glIndices);
	err |= computeVTI.setArg(1, *vertexToIndicesPointers);
	err |= computeVTI.setArg(2, *vertexToIndicesData);
	err |= computeVTI.setArg(3, indices);
	clAssert(err, "Decimator::computeVertexToIndices: Adding parameters to kernel");


	workSizePadding = indices % maxWorkgroupSize;
	workSizePadding = workSizePadding == 0 ? 0 : maxWorkgroupSize - workSizePadding;

	err = queue->enqueueNDRangeKernel(
		computeVTI,
		cl::NullRange,
		cl::NDRange(indices + workSizePadding),
		cl::NullRange,
		waitVector,
		&computeVTIEvent);
	clAssert(err, "Decimator::computeVertexToIndices: Adding kernel to queue");


	if(returnedEvent)
	{
		*returnedEvent = computeVTIEvent;
	}
	else
	{
		err = computeVTIEvent.wait();
		clAssert(err, "Decimator::computeVertexToIndices: Waitting for the completion event");
	}

	return err;
}




 /**********************************************************
 Υπολογίζει τα θεμελιώδη quadrics των τριγώνων τα οποία
 ύστερα θα χρησιμοποιηθούν για τον υπολογισμό των quadrics
 των κορυφών.

 Τα αποτελέσματα μπαίνουν στο buffer object triangleQuadrics
 **********************************************************/
cl_int Decimator::computeTriangleQuadrics(const Object &obj, const std::vector<cl::Event> *const waitVector,cl::Event *const returnedEvent)
{
	std::cout << "Waiting for prev events to finish" << std::endl;
	for(size_t i = 0; i < waitVector->size(); ++i){
		std::cout << "Waiting for event " << i << std::endl;
		waitVector->at(i).wait();
	}


	cl_int err = CL_SUCCESS;
	cl_uint indices = (cl_int) obj.indices.size();
	cl::Event computeQuadricsEvent;

/**/cl::Kernel computeQuadrics(*program, "computeTriangleQuadrics", &err);
	clAssert(err, "Decimator::computeTriangleQuadrics: Creating kernel");

	err =  computeQuadrics.setArg(0, *glIndices);
	err |= computeQuadrics.setArg(1, *glVertices);
	err |= computeQuadrics.setArg(2, *triangleQuadrics);
	err |= computeQuadrics.setArg(3, indices);
	clAssert(err, "Decimator::computeTriangleQuadrics: Adding kernel parameters");

	cl_int workgroupSize = (cl_int) computeQuadrics.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);
	clAssert(err, "Decimator::computeTriangleQuadrics: Getting workgroup info");
	//patch for intel opencl implementation
	workgroupSize = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(&err);
	clAssert(err, "Decimator::computeTriangleQuadrics: Getting workgroup info");
	workgroupSize = std::min(workgroupSize, maxWorkgroupSize);
	
	// intel's OpenCL implementation (for the CPU) segfaults with the current (big) workgroup size
	workgroupSize = 64;
	cl_int workSize = indices + (workgroupSize - indices % workgroupSize)%workgroupSize;
	
	std::cerr << "TriangleQuadrics: before enqueue" << std::endl;
	std::cout << "TriangleQuadrics: workgroupSize: "<< workgroupSize << "\tworkSize: " << workSize << "\tindices: << " << indices << std::endl;
/**/err = queue->enqueueNDRangeKernel(
		computeQuadrics,
		cl::NullRange,
		cl::NDRange(workSize),
		cl::NDRange(workgroupSize),
		waitVector,
		&computeQuadricsEvent
		);
	clAssert(err, "Decimator::computeTriangleQuadrics: Adding kernel to queue");
	std::cout << "TriangleQuadrics: after enqueue, err = " << err << std::endl;
	computeQuadricsEvent.wait();
	std::cout << "TriangleQuadrics: after wait()" << std::endl;

	if(returnedEvent){
		*returnedEvent = computeQuadricsEvent;
	}
	else{
		err = computeQuadricsEvent.wait();
		clAssert(err, "Decimator::computeTriangleQuadrics: Waitting for the completion event");
	}

	return err;
}

 /**********************************************************
 Υπολογίζει τα quadrics κορυφών.
 **********************************************************/
cl_int Decimator::computeFinalQuadrics(const Object &obj, const std::vector<cl::Event> *const waitVector,cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;
	cl_uint vertices = (cl_int) obj.vertices.size();
	cl::Event computeQuadricsEvent;

/**/cl::Kernel finalQuadrics(*program, "computeFinalQuadrics", &err);
	clAssert(err, "Decimator::computeFinalQuadrics: Creating kernel");

	err  = finalQuadrics.setArg(0, *triangleQuadrics);
	err |= finalQuadrics.setArg(1, *quadrics);
	err |= finalQuadrics.setArg(2, *vertexToIndicesPointers);
	err |= finalQuadrics.setArg(3, *vertexToIndicesData);
	err |= finalQuadrics.setArg(4, vertices);
	clAssert(err, "Decimator::computeFinalQuadrics: Adding kernel parameters");

	cl_int workgroupSize = (cl_int) finalQuadrics.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);
	//patch for intel opencl implementation
	workgroupSize = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(&err);
	clAssert(err, "Decimator::computeFinalQuadrics: Getting workgroup info");
	workgroupSize = std::min(workgroupSize, maxWorkgroupSize);
	cl_int workSize = vertices + (workgroupSize - vertices % workgroupSize)%workgroupSize;

	std::cout << "computeFinalQuadrics: wgs:" << workgroupSize << std::endl;

/**/err = queue->enqueueNDRangeKernel(
		finalQuadrics,
		cl::NullRange,
		cl::NDRange(workSize),
		cl::NDRange(workgroupSize),
		waitVector,
		&computeQuadricsEvent
		);
	clAssert(err, "Decimator::computeFinalQuadrics: Adding kenrel to queue");

	if(returnedEvent){
		*returnedEvent = computeQuadricsEvent;
	}
	else{
		err = computeQuadricsEvent.wait();
		clAssert(err, "Decimator::computeFinalQuadrics: Waitting for the completion event");
	}

	return err;
}


 /**********************************************************
 Υπολογίζει το σφάλμα βασισμένο στα quadrics.
 **********************************************************/
cl_int Decimator::computeDecimationError(const Object &obj, const std::vector<cl::Event> *const waitVector, cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;
	cl_uint vertices = (cl_int) obj.vertices.size();
	cl::Event computeDecimationErrorEvent;

	cl::Kernel computeDecimationError(*program, "computeDecimationError", &err);
	clAssert(err, "Decimator::computeDecimationError: Creating kernel");

	err  = computeDecimationError.setArg(0, *glVertices);
	err |= computeDecimationError.setArg(1, *quadrics);
	err |= computeDecimationError.setArg(2, *errorArray);
	err |= computeDecimationError.setArg(3, vertices);
	clAssert(err, "Decimator::computeDecimationError: Adding kernel parameters");

	cl_int workgroupSize = (cl_int) computeDecimationError.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);
	clAssert(err, "Decimator::computeDecimationError: Getting workgroup info");
	workgroupSize = std::min(workgroupSize, maxWorkgroupSize);
	cl_int workSize = vertices + (workgroupSize - vertices % workgroupSize)%workgroupSize;

	err = queue->enqueueNDRangeKernel(
		computeDecimationError,
		cl::NullRange,
		cl::NDRange(workSize),
		cl::NDRange(workgroupSize),
		waitVector,
		&computeDecimationErrorEvent
		);
	clAssert(err, "Decimator::computeDecimationError: Adding kernel to queue");

	if(returnedEvent){
		*returnedEvent = computeDecimationErrorEvent;
	}
	else
	{
		err = computeDecimationErrorEvent.wait();
		clAssert(err, "Decimator::computeDecimationError: Waitting for the completion event");
	}

	return err;

}

