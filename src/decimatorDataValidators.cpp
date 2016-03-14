#define NOMINMAX

#include <assert.h>
#include <vector>
#include <iostream>

#include <climits>

#include "decimator.hpp"

void /*_cdecl*/ dataValidator(void *);
void /*_cdecl*/ independentPointsValidator(void *);

 /**********************************************************
 Can be executed only when the exeution device is the CPU

Checks the validity of the data that reside on the buffer objects
 **********************************************************/
cl_int Decimator::validateData(const Object &obj, const std::vector<cl::Event> *const waitVector, cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;

	cl_uint *vertices = new cl_uint((cl_uint) obj.vertices.size());
	cl_uint *indices  = new cl_uint((cl_uint) obj.indices.size());
	cl_uint *validator= new cl_uint(0xdeadface);
	cl::Event validateEvent;

	//size_t size = 0;
	std::vector<void*> arguments;
	//TODO: must add arguments and use from my function

	arguments.push_back(NULL);
	arguments.push_back(NULL);
	arguments.push_back(NULL);
	arguments.push_back(NULL);

	arguments.push_back((void*) vertices);
	arguments.push_back((void*) indices);
	arguments.push_back((void *) validator);





	std::vector<cl::Memory> mem_objects;
	std::vector<const void *> mem_locs;


	mem_objects.push_back(*glVertices);
	mem_objects.push_back(*glIndices);
	mem_objects.push_back(*vertexToIndicesPointers);
	mem_objects.push_back(*vertexToIndicesData);

	mem_locs.push_back(&arguments[0]);
	mem_locs.push_back(&arguments[1]);
	mem_locs.push_back(&arguments[2]);
	mem_locs.push_back(&arguments[3]);





	std::pair<void*, ::size_t> args = std::make_pair(&arguments[0], sizeof(NULL)*arguments.size());

	err = queue->enqueueNativeKernel(dataValidator, args, &mem_objects, &mem_locs, waitVector, &validateEvent);
	//err = queue->enqueueNativeKernel(dataValidator, args, 0, 0, waitVector, &validateEvent);
	assert(err == CL_SUCCESS);

	err = validateEvent.wait();
	assert(err == CL_SUCCESS);

	delete vertices;
	delete indices;
	delete validator;


	if(returnedEvent)
	{
		*returnedEvent = validateEvent;
	}
	else
	{
		validateEvent.wait();
	}

	return err;
}

 /**********************************************************
 Can be executed only when the exeution device is the CPU
 
 The function that is used as a native kernel which validates the
 data in the memory buffers.
 **********************************************************/
void /*_cdecl*/ dataValidator(void *p)
{
	void **args = (void **)p;

	cl_float const * const vertexData = (cl_float *) args[0];
	cl_uint const * const indexData = (cl_uint *) args[1];
	struct arrayInfo const * const vertexToIndicesPointers = (arrayInfo *)args[2];
	cl_uint const * const vertexToIndicesData = (cl_uint *) args[3];

	const cl_uint vertices = *(cl_uint*)(args[4]);
	const cl_uint indices  = *(cl_uint*)(args[5]);
	const cl_uint validity_test = *(cl_uint*)(args[6]);
	assert(validity_test == 0xdeadface);


	unsigned int skippedVertices = 0;

	std::cout << "dataValidator: enter" << std::endl;

	for(unsigned int v = 0; v < vertices; ++v)
	{
		if(vertexData[3*v] == CL_FLT_MAX)
		{
			//vertex no longer used
			++skippedVertices;
			continue;
		}

		for(unsigned int b = v; b != CL_UINT_MAX; )
		{
			const struct arrayInfo blockPointer = vertexToIndicesPointers[b];

			for(unsigned int i = 0; i < blockPointer.size; ++i)
			{
				//entry must point to valid indidces
				const unsigned int triangle = vertexToIndicesData[blockPointer.position + i];

				if(triangle == CL_UINT_MAX || triangle > indices)
				{
					std::cerr << "dataValidator: Invalid vertexToIndex" << std::endl;
					break;//continue;
				}

				unsigned int const * const ind = &indexData[3*triangle];

				if(ind[0] == CL_UINT_MAX)
				{
					std::cerr << "dataValidator: VertexToIndex points to invalidated triangle" << std::endl;
					//std::cerr << '.';
					std::cerr << "triangle: " << triangle << "(" << ind[0] << ", " << ind[1] << ", " << ind[2] << ")" << std::endl;
					break;//continue;
				}

				for(unsigned int j = 0; j < 3; ++j)
				{
					float const*const vert = vertexData + 3*ind[0] + j;

					if(vert[0] == CL_FLT_MAX)
					{
						std::cerr << "dataValidator: Triangle points to invalidated vertex" << std::endl;
						break;
					}
				}

			}

			b = blockPointer.continuesTo;
		}
	}

	std::cout << "dataValidator: skippedVertices = " << skippedVertices << std::endl;
/*
	//dump data
	std::clog << "vertices\n";
	for(unsigned int i = 0; i < vertices; ++i)
	{
		std::clog << i << "\t|\t";
		for(unsigned int j = 0; j < 3 ; ++j)
		{
			const float value = vertexData[3*i+j];
			std::clog << (value == CL_FLT_MAX ? "X" : "_") << "\t";
		}
		std::clog << "\n";
	}
	std::clog << std::endl;

	std::clog << "indices\n";
	for(unsigned int i = 0; i < indices; ++i)
	{
		std::clog << i << "\t|\t";
		for(unsigned int j = 0; j < 3; ++j)
		{
			const unsigned int value = indexData[3*i+j];
			if(value == CL_UINT_MAX)
			{
				std::clog << "___\t";
			}
			else{
				std::clog << value << "\t";
			}
		}
		std::clog << "\n";
	}
//*/



	return;
}

 /**********************************************************
 Can be executed only when the exeution device is the CPU.
 
 Checks the validity of the data in the independent points array.
 **********************************************************/
cl_int Decimator::validateIndependentPoints(const Object &obj, const std::vector<cl::Event> *const waitVector, cl::Event *const returnedEvent)
{
	cl_int err = CL_SUCCESS;

	cl_uint *vertices = new cl_uint((cl_uint) obj.vertices.size());
	cl_uint *indices  = new cl_uint((cl_uint) obj.indices.size());
	cl_uint *found    =new cl_uint(this->pointsFound);
	cl_uint *validator= new cl_uint(0xdeadface);
	cl::Event validateEvent;

	//size_t size = 0;
	std::vector<void*> arguments;
	//TODO: must add arguments and use from my function

	arguments.push_back(NULL);
	arguments.push_back(NULL);
	arguments.push_back(NULL);
	arguments.push_back(NULL);
	arguments.push_back(NULL);
	arguments.push_back(NULL);

	arguments.push_back((void*) vertices);
	arguments.push_back((void*) indices);
	arguments.push_back((void*) found);
	arguments.push_back((void *) validator);





	std::vector<cl::Memory> mem_objects;
	std::vector<const void *> mem_locs;


	mem_objects.push_back(*glVertices);
	mem_objects.push_back(*glIndices);
	mem_objects.push_back(*vertexToIndicesPointers);
	mem_objects.push_back(*vertexToIndicesData);
	mem_objects.push_back(*independentPoints);
	mem_objects.push_back(*errorArray);


	mem_locs.push_back(&arguments[0]);
	mem_locs.push_back(&arguments[1]);
	mem_locs.push_back(&arguments[2]);
	mem_locs.push_back(&arguments[3]);
	mem_locs.push_back(&arguments[4]);
	mem_locs.push_back(&arguments[5]);





	std::pair<void*, ::size_t> args = std::make_pair(&arguments[0], sizeof(void *)*arguments.size());

	err = queue->enqueueNativeKernel(independentPointsValidator, args, &mem_objects, &mem_locs, waitVector, &validateEvent);
	//err = queue->enqueueNativeKernel(dataValidator, args, 0, 0, waitVector, &validateEvent);
	assert(err == CL_SUCCESS);

	err = validateEvent.wait();
	assert(err == CL_SUCCESS);
	
	delete vertices;
	delete indices;
	delete found;
	delete validator;

	if(returnedEvent)
	{
		*returnedEvent = validateEvent;
	}
	else
	{
		validateEvent.wait();
	}

	return err;
}

 /**********************************************************
 Can be executed only when the exeution device is the CPU.


 The function that is used as a native kernel which validates the
 data in the independent points array.
 **********************************************************/
void /*_cdecl*/ independentPointsValidator(void *p)
{
	void **args = (void **)p;

	int ac = 0;
	cl_float const * const vertexData						= (cl_float *)		args[ac++];
	cl_uint const * const indexData							= (cl_uint *)		args[ac++];
	struct arrayInfo const * const vertexToIndicesPointers	= (arrayInfo *)		args[ac++];
	cl_uint const * const vertexToIndicesData				= (cl_uint *)		args[ac++];
	cl_uint const*const independentPoints					= (cl_uint *)		args[ac++];
	cl_double const*const errorArray						= (cl_double *)	args[ac++];

	const cl_uint vertices									= *(cl_uint*) args[ac++];
	const cl_uint indices									= *(cl_uint*) args[ac++];
	const cl_uint pointsFound								= *(cl_uint*) args[ac++];
	const cl_uint validity_test								= *(cl_uint*) args[ac++];
	assert(validity_test == 0xdeadface);

	//bool *const used = new bool[vertices];
	bool *const used = new bool[vertices];

	//std::cout << "ip Validator: enter" << std::endl;

	for(unsigned int i = 0; i < vertices; ++i)
	{
		used[i] = false;
	}


	for(unsigned int i = 0; i < pointsFound; ++i)
	{
		//get vertex number
		const unsigned int v = independentPoints[i];

		if(v >= vertices)
		{
			std::cout << "ip Validator: Invalid Vertex number " << v << std::endl;
			continue;
		}

		//is vertex valid?
		if(vertexData[3*v] == CL_FLT_MAX)
		{
			std::cout << "ip Validator: Invalid vertex " << v << std::endl;
			continue;
		}

		//has valid queue

		bool succeded = true;

		//is really independent?
		for(unsigned int b = v; b != UINT_MAX; )
		{
			const arrayInfo ai = vertexToIndicesPointers[b];

			for(unsigned int entry = 0; entry < ai.size; ++entry)
			{
				const unsigned int triangle = vertexToIndicesData[ai.position + entry];

				if(triangle == UINT_MAX)
				{
					std::cerr << "ip Validator: Bad entry";
					succeded = false;
					break;
				}

				const unsigned int v0 = indexData[3*triangle    ];
				const unsigned int v1 = indexData[3*triangle + 1];
				const unsigned int v2 = indexData[3*triangle + 2];

				if(v0 == CL_UINT_MAX)
				{
					std::cerr << "ip Validator: bad Triangle " << triangle;
					succeded = false;
					break;
				}

				if((vertexData[3*v0] == CL_FLT_MAX) || (vertexData[3*v1] == CL_FLT_MAX) || (vertexData[3*v2] == CL_FLT_MAX))
				{
					std::cerr << "ip Validator: Bad vertex " << v0 << " " << v1 << " " << v2 << "(triangle " << triangle << ")";
					succeded = false;
					break;
				}

				if((used[v0] == true) || (used[v1] == true) || (used[v2] == true))
				{
					std::cerr << "ip Validator: Vertex already used";
					succeded = false;
					break;
				}

			}

			if(!succeded)
			{
				break;
			}

			b = ai.continuesTo;
		}

		if(!succeded)
		{
			std::clog << " Continuing to next point" << std::endl;
			continue;//continue to nect vertex
		}

		//succeded, mark used array
		for(unsigned int b = v; b != UINT_MAX; )
		{
			const arrayInfo ai = vertexToIndicesPointers[b];

			for(unsigned int entry = 0; entry < ai.size; ++entry)
			{
				const unsigned int triangle = vertexToIndicesData[ai.position + entry];

				const unsigned int v0 = indexData[3*triangle    ];
				const unsigned int v1 = indexData[3*triangle + 1];
				const unsigned int v2 = indexData[3*triangle + 2];

				used[v0] = used[v1] = used[v2] = true;
			}

			b = ai.continuesTo;
		}
	}
//*
	//check that points are sorted
    for( int i = 0; i < (int) pointsFound-1; ++i)
	{
		if(errorArray[independentPoints[i]] > errorArray[independentPoints[i + 1]])
		{
			std::cout << "ip Validator: Bad sorting at point " << i  << " from " << pointsFound << std::endl;
            for(int j = std::max(i-2, 0); j < std::min(i+10, (int)pointsFound); ++j)
			{
                std::cout << "errorArray[independentPoints[" << j << "]] = " << errorArray[independentPoints[j]] << std::endl;
			}
            assert(false);
		}
	}
//*/
	delete[] used;

}
