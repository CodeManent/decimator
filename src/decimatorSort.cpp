#define NOMINMAX //for conflict of std::min/max with min/max at WinDef.h (min/max defined as macros)

#include <vector>
#include <iostream>
#include <algorithm>

#include "decimator.hpp"

 /**********************************************************
 Ταξινόμηση των ανεξάρτητων κορυφών βάσει του σφάλματος που
 αντιστοιχεί στην καθεμιά.Ο αλγόριθμος ταξινόμησης που
 υλοποιείται είναι ο bitonic sort.
 **********************************************************/

cl_int Decimator::sortDecimationError(const Object &obj, const std::vector<cl::Event> *const waitVector, cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;
	const cl_uint sortDirection = SORT_ASCENDING;
	cl::Event sortEvent;
	std::vector<cl::Event> internalWaitVector = *waitVector;

	cl_int po2;
	// find power of 2 equal or greater to Vertices
	for(po2 = 1; po2 < pointsFound; po2 <<=1)
	{
	}

//*
	cl::Kernel localBitonicSort(*program, "localBitonicSort", &err);
	clAssert(err, "Decimator::sortDecimationError: Creating kernel");

	cl_int localWorkgroupSize = (cl_int) localBitonicSort.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);
	clAssert(err, "Decimator::sortDecimationError: Getting workgroup info");
	localWorkgroupSize = std::min(localWorkgroupSize, maxWorkgroupSize);

//	cl_int workSize = vertices + (workgroupSize - vertices % workgroupSize)%workgroupSize;

	if(po2 <= localWorkgroupSize*2 && po2 <= ( maxLocalSize / sizeof(cl_uint) ) )
	{
		//std::cout << "local sort" << std::endl;

		cl_uint ac = 0;//argument counter
		err  = localBitonicSort.setArg(ac++, *errorArray);
		err |= localBitonicSort.setArg(ac++, *independentPoints);
		err |= localBitonicSort.setArg(ac++, pointsFound);
		err |= localBitonicSort.setArg(ac++, sortDirection);
		err |= localBitonicSort.setArg(ac++, cl::__local(sizeof(cl_uint) * po2));
		clAssert(err, "Decimator::sortDecimationError: Adding kernel parameters");

		err = queue->enqueueNDRangeKernel(
			localBitonicSort,
			cl::NDRange(0),
			cl::NDRange(po2/2),
			cl::NDRange(po2/2),
			waitVector,
			&sortEvent
			);
		clAssert(err, "Decimator::sortDecimationError: Adding kernel to queue");
	}
	else
//*/
	{
		cl_int ac = 0;
		//local presort
		cl::Kernel localPreSortARB(*program, "localBitonicPreSortARB", &err);
		clAssert(err, "Decimator::sortDecimationError: Creating \"localBitonicPreSortARB\" kernel");
		
		cl_int workgroupSize  = (cl_int)localPreSortARB.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);
		clAssert(err, "Decimator::sortDecimationError: Getting workgroup size");
		workgroupSize = std::min(workgroupSize, maxWorkgroupSize);
		workgroupSize = std::min(workgroupSize, (cl_int)(maxLocalSize/(2*(sizeof(cl_uint)+sizeof(cl_double)))));
		cl_int localPresortStages = 1;
		for(cl_int localPresortPo2 = 1; localPresortPo2 < workgroupSize; localPresortPo2 <<=1)
		{
			++localPresortStages;
		}

		ac = 0;
		err  = localPreSortARB.setArg(ac++, *errorArray);
		err |= localPreSortARB.setArg(ac++, *independentPoints);
		err |= localPreSortARB.setArg(ac++, pointsFound);
		err |= localPreSortARB.setArg(ac++, sortDirection);
		err |= localPreSortARB.setArg(ac++, localPresortStages);
		err |= localPreSortARB.setArg(ac++, cl::__local(sizeof(cl_double)*(1<<localPresortStages)));
		err |= localPreSortARB.setArg(ac++, cl::__local(sizeof(cl_uint)*(1<<localPresortStages)));
		clAssert(err, "Decimator::sortDecimationError: Setting \"localPreSortARB\" parameters");

		cl::Event localPreSortEvent;
		cl_int workSize = pointsFound/2 + (workgroupSize - (pointsFound/2)%workgroupSize)%workgroupSize;
		err = queue->enqueueNDRangeKernel(
			localPreSortARB,
			cl::NDRange(0),
			cl::NDRange(workSize),
			cl::NDRange(workgroupSize),
			&internalWaitVector,
			&localPreSortEvent
			);
		clAssert(err, "Decimator::sortDecimationError: Enqueue \"localPreSortARB\" kernel");
		internalWaitVector.clear();
		internalWaitVector.push_back(localPreSortEvent);

		//global sort
		cl::Kernel bitonicSortARB(*program, "globalBitonicSortARB", &err);
		clAssert(err, "Decimator::sortDecimationError: Creating kernel");

		ac = 0;
		err  = bitonicSortARB.setArg(ac++, *errorArray);
		err |= bitonicSortARB.setArg(ac++, *independentPoints);
		err |= bitonicSortARB.setArg(ac++, pointsFound);
		err |= bitonicSortARB.setArg(ac++, sortDirection);
		clAssert(err, "Decimator::sortDecimationError: Adding kernel parameters");

		workgroupSize = (cl_int) bitonicSortARB.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);
		clAssert(err, "Decimator::sortDecimationError: Getting workgroup info");
		workgroupSize = std::min(maxWorkgroupSize, workgroupSize);
		workSize = pointsFound + ((workgroupSize - pointsFound%workgroupSize) % workgroupSize);

		cl_int stages = 0;
		//stages = log2(po2);
		for(cl_uint tmp = po2; tmp > 1; tmp>>=1)
		{
			++stages;
		}

		cl::Kernel bitonicMergeARB(*program, "localBitonicMergeARB", &err);
		clAssert(err, "Decimator::sortDecimationError: Creating \"bitoincMergeARB\" kernel");
		cl_int mergeWorkgroupSize = (cl_int)bitonicMergeARB.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);
		clAssert(err, "Decimator::sortDecimationError: Getting \"bitoincMergeARB\"s workgroup size");
		mergeWorkgroupSize = std::min(mergeWorkgroupSize, maxWorkgroupSize);
		mergeWorkgroupSize = std::min(mergeWorkgroupSize, (cl_int)(maxLocalSize/2*(sizeof(cl_uint) + sizeof(cl_double))));
		cl_int mergeWorkSize = pointsFound/2 + (mergeWorkgroupSize - (pointsFound/2)%mergeWorkgroupSize)%mergeWorkgroupSize;

		cl_int mergePasses = 1;
		for(cl_int mergePo2 = 1; mergePo2 < mergeWorkgroupSize; mergePo2 <<= 1)
		{
			++mergePasses;
		}
		ac = 0;
		err  = bitonicMergeARB.setArg(ac++, *errorArray);
		err |= bitonicMergeARB.setArg(ac++, *independentPoints);
		err |= bitonicMergeARB.setArg(ac++, pointsFound);
		err |= bitonicMergeARB.setArg(ac++, sortDirection);
		err |= bitonicMergeARB.setArg(ac++, cl::__local(sizeof(cl_double)*(1<<mergePasses)));
		err |= bitonicMergeARB.setArg(ac++, cl::__local(sizeof(cl_uint)*(1<<mergePasses)));
		clAssert(err, "Decimator::sortDecimationError: Setting merge kernel parameters");

		for(cl_int stage = localPresortStages; stage < stages; ++stage)
		{
            for(cl_int passOfStage = 0; passOfStage <= stage; ++passOfStage)
			{
				if(stage-passOfStage > mergePasses-1)
				{
					err |= bitonicSortARB.setArg(4, stage);
					err |= bitonicSortARB.setArg(5, passOfStage);
					clAssert(err, "Decimator::sortDecimationError: Adding kernel parameters");

					err = queue->enqueueNDRangeKernel(
						bitonicSortARB,
						cl::NDRange(0),
						cl::NDRange(workSize),
						cl::NDRange(workgroupSize),
						&internalWaitVector,
						&sortEvent);
					clAssert(err, "Decimator::sortDecimationError: Adding kernel to queue");

					internalWaitVector.clear();
					internalWaitVector.push_back(sortEvent);
				}
				else
				{
					err  = bitonicMergeARB.setArg(6, stage);
					err |= bitonicMergeARB.setArg(7, passOfStage);
					clAssert(err, "Decimator::sortDecimationError: Adding merge kernel parameters");

					err = queue->enqueueNDRangeKernel(
						bitonicMergeARB,
						cl::NDRange(0),
						cl::NDRange(mergeWorkSize),
						cl::NDRange(mergeWorkgroupSize),
						&internalWaitVector,
						&sortEvent
						);
					clAssert(err, "Decimator::sortDecimationError: Adding merge kernel to queue");

					internalWaitVector.clear();
					internalWaitVector.push_back(sortEvent);

					break;
				}
			}
		}
	}
	if(returnedEvent){
		*returnedEvent = sortEvent;
	}
	else
	{
		err = sortEvent.wait();
		clAssert(err, "Decimator::sortDecimationError: Waitting for the completion event");
	}
	return err;
}
