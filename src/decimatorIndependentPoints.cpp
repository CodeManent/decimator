#define NOMINMAX

#include <vector>
#include <iostream>
#include <algorithm>

#include "decimator.hpp"

 /**********************************************************
 Μέθοδος επιλογέας του κατάλληλου αλγόριθμου για τον
 υπολογισμό των ανεξάρτητων κορυφών βάση της μεταβλητής
 independentPointsAlgorithm.

 Οι παράμετροι εκτέλεσης προωθούνται στην κατάλληλη μέθοδο
 που υλοποιεί τον αλγόριθμο που επιλέχθηκε.
 **********************************************************/
cl_int Decimator::computeIndependentPoints(const Object &obj, unsigned int remainingVertices, const std::vector<cl::Event> *const waitVector , cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;
	cl::Event independentPointsEvent;

	if(independentPointsAlgorithm == 1)
	{
		err = computeIndependentPoints1(obj, waitVector, &independentPointsEvent);
	}
	else if(independentPointsAlgorithm == 2)
	{
		err = computeIndependentPoints2(obj, waitVector, &independentPointsEvent);

		if(this->pointsFound < 5)
		{
			//std::cout << "inependentPoints1" << std::endl;
			std::vector<cl::Event> internalWaitVector(1, independentPointsEvent);
			err = computeIndependentPoints1(obj, &internalWaitVector, &independentPointsEvent);
		}
	}
	else if(independentPointsAlgorithm == 3)
	{
		err = computeIndependentPoints3(obj, remainingVertices, waitVector, &independentPointsEvent);
	}

	if(returnedEvent)
	{
		*returnedEvent = independentPointsEvent;
	}
	else
	{
		independentPointsEvent.wait();
	}

	return err;
}

 /**********************************************************
 Υπολογίζει τις ανεξάρτητες κορυφές χρησιμοποιόντας την 
 σειριακή υλοποίηση του αλγορίθμου.
 **********************************************************/
cl_int Decimator::computeIndependentPoints1(const Object &obj, const std::vector<cl::Event> *const waitVector , cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;
	const cl_uint vertices = (cl_int) obj.vertices.size();
	//cl_uint workSizePadding = 0;
	cl_uint maxPoints = (cl_uint)(vertices * maxIndependentPointsToVertices);
	cl::Event independentPointsEvent, readEvent;
	std::vector<cl::Event> internalWaitVector;

	cl::Kernel findIndependentPoints(*program, "findIndependentPoints", &err);
	clAssert(err, "Decimator::computeIndependentPoints1: Creating kernel");

	//temporary buffers used only for this kernel
	cl::Buffer used(*context, CL_MEM_READ_WRITE, sizeof(cl_uint) * vertices, 0, &err);
	clAssert(err, "Decimator::computeIndependentPoints1: Creating used buffer");
	cl::Buffer pointsFoundBuffer(*context, CL_MEM_READ_WRITE, sizeof(cl_uint), 0, &err);
	clAssert(err, "Decimator::computeIndependentPoints1: Creating pointsFound buffer");

	unsigned int ac = 0;//argument counter
	err |= findIndependentPoints.setArg(ac++, *glVertices);
	err |= findIndependentPoints.setArg(ac++, *glIndices);
	err |= findIndependentPoints.setArg(ac++, *vertexToIndicesPointers);
	err |= findIndependentPoints.setArg(ac++, *vertexToIndicesData);
	err |= findIndependentPoints.setArg(ac++, *independentPoints);
	err |= findIndependentPoints.setArg(ac++, used);
	err |= findIndependentPoints.setArg(ac++, pointsFoundBuffer);
	err |= findIndependentPoints.setArg(ac++, maxPoints);
	err |= findIndependentPoints.setArg(ac++, vertices);
	clAssert(err, "Decimator::computeIndependentPoints1: Adding kernel arguments");

	err = queue->enqueueTask(findIndependentPoints, waitVector, &independentPointsEvent);
	clAssert(err, "Decimator::computeIndependentPoints1: Adding kernel to queue");


	clAssert(queue->flush(), "flushing queue");
	debugWait(independentPointsEvent);

	internalWaitVector.push_back(independentPointsEvent);
	err = queue->enqueueReadBuffer(pointsFoundBuffer, CL_TRUE, 0, sizeof(cl_uint), &(this->pointsFound), &internalWaitVector, &readEvent);
	clAssert(err, "Decimator::computeIndependentPoints1: Reading the number of points found");

	clAssert(queue->flush(), "flushing queue");
	debugWait(readEvent);
	clAssert(queue->flush(), "flushing queue");
	//std::cout << "computeIndependentPoints: Points found = " << this->pointsFound << std::endl;


	if(returnedEvent)
	{
		*returnedEvent = readEvent;
	}

	return err;
}




 /**********************************************************
 Υπολογίζει τις ανεξάρτητες κορυφές χρησιμοποιόντας την 
 παράλληλη υλοποίση βασισμένη στο σφάλμα της κάθε κορυφής.
 **********************************************************/
cl_int Decimator::computeIndependentPoints2(const Object &obj, const std::vector<cl::Event> *const waitVector, cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;

	debugWait((*waitVector)[0]);
	clAssert(queue->flush(), "flushing queue");

	const cl_uint vertices = (cl_int) obj.vertices.size();
	//cl_uint workSizePadding = vertices % maxWorkgroupSize;

	cl::Event independentPointsEvent, readEvent;
	std::vector<cl::Event> internalWaitVector;

	cl::Kernel findIndependentPoints2(*program, "findIndependentPoints_2", &err);
	clAssert(err, "Decimator::computeIndependentPoints2: Creating kernel");

	cl::Buffer pointsFoundBuffer(*context, CL_MEM_READ_WRITE, sizeof(cl_uint), 0, &err);
	clAssert(err, "Decimator::computeIndependentPoints2: Creating points found buffer");

	unsigned int ac = 0;
	err |= findIndependentPoints2.setArg(ac++, *glVertices);
	err |= findIndependentPoints2.setArg(ac++, *glIndices);
	err |= findIndependentPoints2.setArg(ac++, *vertexToIndicesPointers);
	err |= findIndependentPoints2.setArg(ac++, *vertexToIndicesData);
	err |= findIndependentPoints2.setArg(ac++, *errorArray);
	err |= findIndependentPoints2.setArg(ac++, *independentPoints);
	err |= findIndependentPoints2.setArg(ac++, pointsFoundBuffer);
	err |= findIndependentPoints2.setArg(ac++, vertices);
	clAssert(err, "Decimator::computeIndependentPoints2: Adding kernel parameters");

	//workSizePadding = workSizePadding == 0 ? 0 : maxWorkgroupSize - workSizePadding;
	cl_int workgroupSize = getWorkgroupSize(findIndependentPoints2, "computeIndependentPoints2");
	cl_int workSize = getWorkSize(vertices ,workgroupSize);

	err = queue->enqueueNDRangeKernel(
		findIndependentPoints2,
		cl::NDRange(0),
		cl::NDRange(workSize),
		cl::NDRange(workgroupSize),
		waitVector,
		&independentPointsEvent
		);
	clAssert(err, "Decimator::computeIndependentPoints2: Adding kernel to queue");

	clAssert(queue->flush(), "flushing queue");
	debugWait(independentPointsEvent);

	internalWaitVector.clear();
	internalWaitVector.push_back(independentPointsEvent);
	err = queue->enqueueReadBuffer(pointsFoundBuffer, CL_TRUE, 0, sizeof(cl_uint), &(this->pointsFound), &internalWaitVector, &readEvent);
	clAssert(err, "Decimator::computeIndependentPoints2: Reading the number of points found");

	clAssert(queue->flush(), "flushing queue");
	debugWait(readEvent);

	//std::cout << "findIndependentPoints2: pointsFound = " << this->pointsFound << std::endl;

	if(returnedEvent)
	{
		*returnedEvent = readEvent;
	}

	return err;
}




 /**********************************************************
 Υπολογίζει τις ανεξάρτητες κορυφές χρησιμοποιόντας την 
 παράλληλη υλοποίση βασισμένη μόνο στην συνδεσιμότητα του
 γράφου των που ορίζεται απότο μοντέλο.
 **********************************************************/
cl_int Decimator::computeIndependentPoints3(const Object &obj, unsigned int remainingVertices, const std::vector<cl::Event> *const waitVector, cl::Event *const returnedEvent)
{

	//std::cout << "computeIndependentPoints3: enter" << std::endl;
	cl_int err = CL_SUCCESS;
	std::vector<cl::Event> internalWaitVector;

	const cl_uint vertices = (cl_uint) obj.vertices.size();
	cl_int workgroupSize = 1;
	cl_int workSize;

	cl::Event resetUsedEvent, markDependentPointsEvent,sweepIndependentPointsEvent, readEvent;

	cl::Kernel resetUsed(*program, "resetUsed", &err);
	clAssert(err, "Decimator::computeIndependentPoints3: Creating kernel");
	cl::Kernel markDependentPoints(*program, "markDependentPoints", &err);
	clAssert(err, "Decimator::computeIndependentPoints3: Creating kernel");
	cl::Kernel sweepIndependentPoints(*program, "sweepIndependentPoints", &err);
	clAssert(err, "Decimator::computeIndependentPoints3: Creating kernel");

	cl::Buffer pointsFoundBuffer(*context, CL_MEM_READ_WRITE, sizeof(cl_uint), 0, &err);
	clAssert(err, "Decimator::computeIndependentPoints3: Creating points found buffer");

	unsigned int ac = 0;
	err |= resetUsed.setArg(ac++, *usedArray);
	err |= resetUsed.setArg(ac++, vertices);
	err |= resetUsed.setArg(ac++, pointsFoundBuffer);
	clAssert(err, "Decimator::computeIndependentPoints3: Adding kernel paramemets");

	workgroupSize = getWorkgroupSize(resetUsed, "computeIndependentPoints3");
	workSize = getWorkSize(vertices ,workgroupSize);

	err = queue->enqueueNDRangeKernel(
		resetUsed,
		cl::NDRange(0),
		cl::NDRange(workSize),
		cl::NDRange(workgroupSize),
		waitVector,
		&resetUsedEvent);
	clAssert(err, "Decimator::computeIndependentPoints3: Addig kernel to queue");

	debugWait(resetUsedEvent);

	ac = 0;
	err |= markDependentPoints.setArg(ac++, *glVertices);
	err |= markDependentPoints.setArg(ac++, *glIndices);
	err |= markDependentPoints.setArg(ac++, *vertexToIndicesPointers);
	err |= markDependentPoints.setArg(ac++, *vertexToIndicesData);
	err |= markDependentPoints.setArg(ac++, *usedArray);
	err |= markDependentPoints.setArg(ac++, vertices);
	clAssert(err, "Decimator::computeIndependentPoints3: Adding kernel parameters");

	workSize = std::min(std::max((unsigned int)1,remainingVertices / 100), maxComputeUnits *64);
	//workSize = 1;
	//std::cout << "Decimator::computeIndependentPoints3: " << workSize << " slices " << std::endl;
	//workSize = maxComputeUnits * 32;

	internalWaitVector.push_back(resetUsedEvent);
	err = queue->enqueueNDRangeKernel(
		markDependentPoints,
		cl::NDRange(0),
		cl::NDRange(workSize),
		cl::NDRange(1),
		&internalWaitVector,
		&markDependentPointsEvent);
	clAssert(err, "Decimator::computeIndependentPoints3: Adding kernel to queue");

	debugWait(markDependentPointsEvent);

	ac = 0;
	err |= sweepIndependentPoints.setArg(ac++, *glIndices);
	err |= sweepIndependentPoints.setArg(ac++, *vertexToIndicesPointers);
	err |= sweepIndependentPoints.setArg(ac++, *vertexToIndicesData);
	err |= sweepIndependentPoints.setArg(ac++, *errorArray);
	err |= sweepIndependentPoints.setArg(ac++, *usedArray);
	err |= sweepIndependentPoints.setArg(ac++, vertices);
	err |= sweepIndependentPoints.setArg(ac++, *independentPoints);
	err |= sweepIndependentPoints.setArg(ac++, pointsFoundBuffer);
	clAssert(err, "Decimator::computeIndependentPoints3: Adding kernel parameters");

	workgroupSize = getWorkgroupSize(sweepIndependentPoints, "computeIndependentPoints3");
	workSize = getWorkSize(vertices ,workgroupSize);

	internalWaitVector.clear();
	internalWaitVector.push_back(markDependentPointsEvent);
	err = queue->enqueueNDRangeKernel(
		sweepIndependentPoints,
		cl::NDRange(0),
		cl::NDRange(workSize),
		cl::NDRange(workgroupSize),
		&internalWaitVector,
		&sweepIndependentPointsEvent);
	clAssert(err, "Decimator::computeIndependentPoints3: Adding kernel to queue");

	debugWait(sweepIndependentPointsEvent);

	internalWaitVector.clear();
	internalWaitVector.push_back(sweepIndependentPointsEvent);
	err = queue->enqueueReadBuffer(pointsFoundBuffer, CL_TRUE, 0, sizeof(cl_uint), &(this->pointsFound), &internalWaitVector , &readEvent);
	clAssert(err, "Decimator::computeIndependentPoints3: Reading the number of points found");
	
	debugWait(readEvent);

	//std::cout << "findIndependentPoints3: pointsFound = " << this->pointsFound << std::endl;

	if(returnedEvent)
	{
		*returnedEvent = readEvent;
	}
	return err;
}

