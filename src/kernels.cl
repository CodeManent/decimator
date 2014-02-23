//logging constants for debugging

//#define LOGGING 1
//#define LOG_COMPUTE_QUADRICS 1
//#define LOG_COMPUTE_ERROR 1
//#define LOG_SORTING 1
//#define LOG_LOCAL_SORT 1
//#define LOG_INDEPENDENT_POINTS 1
//#define LOG_PACK_LIST_ELEMENTS 1
//#define LOG_PLACEMENT 1
//#define LOG_DECIMATE 1
//__FILE__

#if  defined(LOGGING)
	#if defined(cl_amd_printf)
		#pragma OPENCL EXTENSION cl_amd_printf : enable
	#endif
	#define log_message printf
#else
	//ignore printf commands
	#define dummyPrintf(x, ...)
	#define log_message dummyPrintf
#endif

#ifdef cl_khr_fp64

	#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#else 

	#ifdef cl_amd_fp64
		#pragma OPENCL EXTENSION cl_amd_fp64 : enable
	#else
		#define ONLY_FLOATS 1
	#endif

#endif

#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics: enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics: enable

#ifdef ONLY_FLOATS
//if doubles are not supported convert everything to float
#define double float
#define double4 float4
#define double8 float8
#define double16 float16
#define convert_double4 convert_float4
#define DBL_MAX FLT_MAX

#endif

//sorting constants
#define ASCENDING true
#define DESCENDING false

/*
      / 0  1  2  3 \
q = s.| 4  5  6  7 |
      | 8  9  a  b |
      \ c  d  e  f /
*/
// source: http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche23.html
// http://www.tutorvista.com/math/determinant-of-a-4x4-matrix

#define det2x2(a11, a12, a21, a22) \
	(((a21)*(a12)) - ((a11)*(a22)))

#define vdet2x2(vec, v0, v1, v2, v3) \
	det2x2(vec.s##v0, vec.s##v1, vec.s##v2, vec.s##v3)

#define det3x3(a11, a12, a13, a21, a22, a23, a31, a32, a33) \
	((a11)*(a22)*(a33) + (a21)*(a32)*(a13) + (a31)*(a12)*(a23) - (a11)*(a32)*(a23) - (a31)*(a22)*(a13) - (a21)*(a12)*(a33))

#define vdet3x3(vec, v0, v1, v2, v3, v4, v5, v6, v7, v8) \
	det3x3(vec.s##v0, vec.s##v1, vec.s##v2, vec.s##v3, vec.s##v4, vec.s##v5, vec.s##v6, vec.s##v7, vec.s##v8)

struct arrayInfo
{
	unsigned int position;
	unsigned int size;
	unsigned int continuesTo;
};

 /**********************************************************
 Initialise arrays
 **********************************************************/
__kernel void intialiseArrays(
							  __global double16 *const quadrics, 
							  __global double *const errorArray, 
							  __global struct arrayInfo *const vertexToIndicesPointers, 
							  unsigned int maxVertexToIndices, 
							  unsigned int vertices
							  );

 /**********************************************************
 Vertex to indices pointers
 **********************************************************/
__kernel void computeVertexToIndices(
									 __global unsigned int const *const indices, 
									 __global struct arrayInfo *const vertexToIndicesPointers, 
									 __global unsigned int *const vertexToIndicesData, 
									 unsigned int indicesSize
									 );

 /**********************************************************
 Independent points
 **********************************************************/
__kernel void findIndependentPoints(
									__global float const * const vertices,
									__global unsigned int const * const indices,
									__global struct arrayInfo const * const vertexToIndicesPointers,
									__global unsigned int const * const vertexToIndicesData,
									__global unsigned int *const independentPoints,
									__global unsigned int *const used,
									__global unsigned int *const pointsFound,
									const unsigned int maxPoints,
									const unsigned int verticesSize
									);

__kernel void findIndependentPoints_2(
									  __global float const * const vertices,
									  __global unsigned int const * const indices,
									  __global struct arrayInfo const * const vertexToIndicesPointers,
									  __global unsigned int const * const vertexToIndicesData,
									  __global double const * const errorArray,
									  __global unsigned int * const independentPoints,
									  __global unsigned int * pointsFound,
									  unsigned int workSize
									  );

__kernel void resetUsed(
						__global unsigned int * const used
						, unsigned int size, 
						__global unsigned int * const pointsFound
						);
__kernel void markDependentPoints(
								  __global float const * const vertices,
								  __global unsigned int const * const indices,
								  __global struct arrayInfo const * const vertexToIndicesPointers,
								  __global unsigned int const * const vertexToIndicesData,
								  __global unsigned int * const used,
								  const unsigned int verticesSize
								  );
__kernel void sweepIndependentPoints(
									 __global unsigned int const * const indices,
									 __global struct arrayInfo const * const vertexToIndicesPointers,
									 __global unsigned int const * const vertexToIndicesData,
									 __global double const * const errorArray,
									 __global unsigned int const * const used,
									 const unsigned int usedSize,
									 __global unsigned int * const independentPoints,
									 const unsigned int independentPointsSize,
									 __global unsigned int * pointsFound
									 );

 /**********************************************************
 Compute quadrics
 **********************************************************/
__kernel void computeTriangleQuadrics(
									  __global unsigned int const *const triangles, 
									  __global float const * const points, 
									  __global double16 * const triangleQuadrics, 
									  unsigned int indices
									  );
__kernel void computeFinalQuadrics(
								   __global double16 const *const triangleQuadrics, 
								   __global double16 *const quadrics, 
								   __global struct arrayInfo const *const vertexToIndicesPointers,
								   __global unsigned int const *const vertexToIndicesData,
								   const unsigned int vertices
								   );

/*Helper functions*/
uint4 getTriangle(__global unsigned int const * const triangles, size_t index);
float4 getVertex(__global float const * const vertices, size_t index);
double4 triangleNormal(float4 p1, float4 p2, float4 p3);
double4 trianglePlane(float4 p1, float4 p2, float4 p3);
double computeFaceArea(float4 p1, float4 p2, float4 p3);
double16 constructQuadric(double4 plane, double area);

 /**********************************************************
 Decimation Error
 **********************************************************/

__kernel void computeDecimationError(
									 __global float *points,  
									 __global double16 *quadrics, 
									 __global double *errorArray, 
									 unsigned int vertices
									 );
double decimationError(const float4 point, const double16 quadric);
void printQuadric(const double16 q);

 /**********************************************************
 Sorting
 **********************************************************/
void localSwapValues(__global double const *const val, __local uint  * const key, const uint left, const uint right, const uint direction, const uint masterDirection);
void localSwapValues2(__local double *const val, __local uint  * const key ,const uint left, const uint right ,const uint direction);
void globalSwapValues(__global double const *const val, __global uint  * const key, const uint left, const uint right, const uint direction);

__kernel void localBitonicSort(
							   __global double const * const errorArray,
							   __global unsigned int * const independentPoints,
							   const unsigned int pointsFound,
							   const unsigned int sortDirection,
							   __local unsigned int * const localMemory
							   );
__kernel void globalBitonicSort(
								__global double const * const val,
								__global uint  * const key,
								const uint stage, 
								const uint passOfStage,
								const uint arrayLength,
								const uint direction
								);
__kernel void globalBitonicMerge(
								 __global double const * const val,
								 __global uint * const key,
								 const uint stage,
								 const uint passOfStage,
								 const uint arrayLength,
								 const uint direction
								 );
__kernel void globalBitonicSortARB(
								   __global double const * const values,
								   __global unsigned int * const keys,
								   const int arrayLength,
								   const int sortDirection,
								   const int stage,
								   const int passOfStage
								   );
__kernel void localBitonicPreSortARB(
									 __global double const * const values,
									 __global unsigned int * const keys,
									 const int arrayLength,
									 const int sortDirection,
									 const int stages,
									 __local double * const localValues,
									 __local unsigned int * const localKeys
									 );
__kernel void localBitonicMergeARB(
								   __global double * const values,
								   __global unsigned int * const keys,
								   const int arrayLength,
								   const int sortDirection,
								   __local double * const localValues,
								   __local unsigned int * const localKeys,
								   const int stage,
								   const int startPassOfStage
								   );

 /**********************************************************
 Decimation
 **********************************************************/
uint2 getCommonTriangles(__global struct arrayInfo const * const vertexToIndicesPointers,__global unsigned int const *const vertexToIndicesData,const unsigned int firstVertex,const unsigned int secondVertex);
void rewireTriangles(__global unsigned int *const indices,__global struct arrayInfo const * const vertexToIndicesPointers,__global unsigned int const * const vertexToIndicesData,const unsigned int firstVertex,const unsigned int secondVertex);
double det4x4(const double16 q);
double16 invert4x4(const double16 q, const double det);
double16 invert3x3(const double16 q, const double det);
double4 multiplyMatrixVector(const double16 m, const double4 v);
float4 findOptimalPlacement(__global float const *const vertices,__global double16 const *const quadrics,const unsigned int firstVertex,const unsigned int secondVertex);
bool foldOverOccurs(const uint v1, const uint v2, const float4 vfinal, __global const float * const vertices,__global const uint *const indices,__global const struct arrayInfo * const vertexToIndicesPointers, __global const uint * const vertexToIndicesData);
bool validEdgeContraction(__global float const * const vertices,__global unsigned int const * const indices,__global struct arrayInfo const * const vertexToIndicesPointers,__global unsigned int const * const vertexToIndicesData,__global double16 const * const quadrics,const unsigned int v0,const unsigned int v1);
unsigned int getBestNeighbor(__global float const * const vertices,__global unsigned int const * const indices,__global struct arrayInfo const * const vertexToIndicesPointers,__global unsigned int const * const vertexToIndicesData,__global double16 const * const quadrics,__global double const * const errorArray,const unsigned int centralVertex);
void mergeVertexToIndicesLists(__global struct arrayInfo *const vertexToIndicesPointers,__global unsigned int *const vertexToIndicesData,const unsigned int base,const unsigned int toAppend);
void packListElements(__global struct arrayInfo *const vertexToIndicesPointers,__global unsigned int * const vertexToIndicesData,const unsigned int maxVertexToIndices,const unsigned int base,const unsigned int firstTriangleToRemove,const unsigned int secondTriangleToRemove);
uint2 getTopBottom(__global unsigned int const * const indices,const unsigned int v1,const unsigned int v2,uint2 commonTriangles);
void connectLists(__global struct arrayInfo *const vertexToIndicesPointers,const unsigned int base,const unsigned int toAppend);
void printList(__global struct arrayInfo const*const vertexToIndicesPointers,__global unsigned int const * const vertexToIndicesData,const unsigned int base);
int validateList(__global struct arrayInfo const*const vertexToIndicesPointers,__global unsigned int const * const vertexToIndicesData,__global unsigned int const * const indices,const unsigned int base);

__kernel void decimateOnPoint(
		__global float *const vertices,
		__global unsigned int *const indices,
		__global double16 *const quadrics,
		__global double *const errorArray,
		__global unsigned int const *const independentPoints,
		__global struct arrayInfo *const vertexToIndicesPointers,
		__global unsigned int *const vertexToIndicesData,
		const unsigned int maxVertexToIndices,
		const unsigned int independentPointsNumber,
		__global unsigned int * failedAttempts
		);











/***********************************************************
| Iinitialises the memory for the computations
*/
__kernel void intialiseArrays(__global double16 *const quadrics, __global double *const errorArray, __global struct arrayInfo *const vertexToIndicesPointers, unsigned int maxVertexToIndices, unsigned int vertices)
{
	size_t tid = get_global_id(0);

	if(!(tid < vertices)){
		return;
	}

	quadrics[tid] = (double16)(0.0);
	errorArray[tid] = (double)(0.0);

	vertexToIndicesPointers[tid].size = 0;
	vertexToIndicesPointers[tid].position = tid*maxVertexToIndices;
	vertexToIndicesPointers[tid].continuesTo = UINT_MAX;
}





/***********************************************************
| Initialises the VertexToIndices data structure
*/
__kernel void computeVertexToIndices(
		__global unsigned int const *const indices, 
		__global struct arrayInfo *const vertexToIndicesPointers, 
		__global unsigned int *const vertexToIndicesData, 
		unsigned int indicesSize
		)
{
	size_t tid = get_global_id(0);

	if(!(tid < indicesSize))
	{
		return;
	}

	const unsigned int v0 = indices[3*tid    ];
	const unsigned int v1 = indices[3*tid + 1];
	const unsigned int v2 = indices[3*tid + 2];

	const unsigned int p0 = atom_inc(&(vertexToIndicesPointers[v0].size));
	vertexToIndicesData[vertexToIndicesPointers[v0].position + p0] = tid;

	const unsigned int p1 = atom_inc(&(vertexToIndicesPointers[v1].size));
	vertexToIndicesData[vertexToIndicesPointers[v1].position + p1] = tid;

	const unsigned int p2 = atom_inc(&(vertexToIndicesPointers[v2].size));
	vertexToIndicesData[vertexToIndicesPointers[v2].position + p2] = tid;

	return;
}




/***********************************************************
| The first serial algorithm for the independent points.
| Uses only one processing unit
*/
__kernel void findIndependentPoints(
		__global float const * const vertices,
		__global unsigned int const * const indices,
		__global struct arrayInfo const * const vertexToIndicesPointers,
		__global unsigned int const * const vertexToIndicesData,
		__global unsigned int *const independentPoints,
		__global unsigned int *const used,
		__global unsigned int *const pointsFound,
		const unsigned int maxPoints,
		const unsigned int verticesSize
		)
{
	*pointsFound = 0;
	unsigned int localPointsFound = 0;

	//initialise used array
	for(unsigned int i=0; i< verticesSize; ++i)
	{
		used[i] = 0;
	}

	for(unsigned int i = 0; (i < verticesSize) && (localPointsFound < maxPoints); ++i)
	{
		if(vertices[3*i] == FLT_MAX || used[i] == 1)
		{
			continue; //to next vertex
		}

		bool succeded = true;

		for(unsigned int b = i; b != UINT_MAX; )
		{
			const struct arrayInfo ai = vertexToIndicesPointers[b];

			if(ai.size < 1)
			{
				//doesn't have neighbors
				succeded = false;
				break;
			}

			for(unsigned int j = 0; j < ai.size; ++j)
			{
				const unsigned int triangle = vertexToIndicesData[ai.position + j];

				const unsigned int v0 = indices[3*triangle    ];
				const unsigned int v1 = indices[3*triangle + 1];
				const unsigned int v2 = indices[3*triangle + 2];


				if((used[v0] == 1) || (used[v1] == 1) || (used[v2] == 1))
				{
					//vertices already used
					//log_message("fip: already used ");
					succeded = false;
					break;
				}
			}

			if(succeded == false)
			{
				break;
			}

			b = ai.continuesTo;

		}

		if(succeded == false)
		{
			continue;
		}

		//succeded, mark vertices as used
		for(unsigned int b = i; b != UINT_MAX; )
		{
			const struct arrayInfo ai = vertexToIndicesPointers[b];

			for(unsigned int j = 0; j < ai.size; ++j)
			{
				const unsigned int triangle = vertexToIndicesData[ai.position + j];

				const unsigned int v0 = indices[3*triangle    ];
				const unsigned int v1 = indices[3*triangle + 1];
				const unsigned int v2 = indices[3*triangle + 2];

				used[v0] = 1;
				used[v1] = 1;
				used[v2] = 1;
			}

			b = ai.continuesTo;
		}

		independentPoints[localPointsFound] = i;
		++localPointsFound;
	}
/*
	for(unsigned int i = 0; i < localPointsFound; ++i)
	{
		if(validateList(vertexToIndicesPointers, vertexToIndicesData, indices, independentPoints[i]) == -1)
		{
			log_message("invalid list for independent point (%d)->%d\n", i, independentPoints[i]);
			printList(vertexToIndicesPointers, vertexToIndicesData, independentPoints[i]);
		}
	}
*/

	*pointsFound = localPointsFound;
}






/***********************************************************
| The parallel independent points algorithm based on the 
| decimation error of the points.
*/
__kernel void findIndependentPoints_2(
		__global float const * const vertices,
		__global unsigned int const * const indices,
		__global struct arrayInfo const * const vertexToIndicesPointers,
		__global unsigned int const * const vertexToIndicesData,
		__global double const * const errorArray,
		__global unsigned int * const independentPoints,
		__global unsigned int * pointsFound,
		unsigned int workSize
		)
{
	unsigned int tid = get_global_id(0);

	if(tid >= workSize)
	{
		return;
	}

	if(tid == 0){
		*pointsFound = 0;
	}
	write_mem_fence(CLK_GLOBAL_MEM_FENCE);

	tid = workSize - tid-1;

	if(vertices[3*tid] == FLT_MAX){
		return;
	}

	const double bestError = errorArray[tid];

	for(unsigned int b = tid; b != UINT_MAX; ){
		const struct arrayInfo ai = vertexToIndicesPointers[b];
		if(ai.size < 1 && b == tid){
			return;
		}

		for(unsigned int i = 0; i < ai.size; ++i)
		{
			const unsigned int index = vertexToIndicesData[ai.position + i];
			for(unsigned int j = 0; j < 3; ++j)
			{
				unsigned int tid2 = indices[3*index + j];
				if(tid == tid2){
					continue;
				}

				const double error2 = errorArray[tid2];
				if((error2 < bestError))
				{
					return;
				}

				for(unsigned int b2 = tid2; b2 != UINT_MAX; )
				{
					const struct arrayInfo ai2 = vertexToIndicesPointers[b2];

					for(unsigned int k = 0; k < ai2.size; ++k)
					{
						const unsigned int index2 = vertexToIndicesData[ai2.position + k];
						for(unsigned int m = 0; m < 3; ++m)
						{
							unsigned int tid3 = indices[3*index2 + m];
							if(tid3 == tid || tid3 == tid2){
								continue;
							}

							double error3 = errorArray[tid3];

							if(((error3 < bestError) /*&& (tid3 > tid)*/) || (error3 == bestError && tid3 > tid))
							{
								return;
							}
						}
					}
					b2 = ai2.continuesTo;
				}

			}
		}
		b = ai.continuesTo;
	}

	//passed the checks and is the best among its neighbors
	//add it to the list
	unsigned int pos = atom_inc(pointsFound);
	independentPoints[pos] = tid;
}





/***********************************************************
| Initialises the used buffer for the parallel independent
| points algorithn
*/
__kernel void resetUsed(__global unsigned int * const used, unsigned int size, __global unsigned int * const pointsFound)
{
	const size_t tid = get_global_id(0);
	if(tid == 0)
	{
		*pointsFound = 0;
#if defined(LOGGING) && defined(LOG_INDEPENDENT_POINTS)
		log_message("resetUsed: set pointsFound = 0\n");
#endif
	}
	if(tid < size)
	{
		used[tid] = 0;
	}
}






/***********************************************************
| The main part of the parallel independent points algorithn
*/
__kernel void markDependentPoints(__global float const * const vertices,
								  __global unsigned int const * const indices,
								  __global struct arrayInfo const * const vertexToIndicesPointers,
								  __global unsigned int const * const vertexToIndicesData,
								  __global unsigned int * const used,
								  const unsigned int verticesSize
								  )
{
	const size_t tid = get_global_id(0);
	const size_t threads = get_global_size(0);
	const unsigned int blockSize = verticesSize / threads;
	const unsigned int startVertex = blockSize * tid;
	const unsigned int lastVertex = blockSize*(tid+1);
	const unsigned int endVertex = lastVertex < verticesSize ? lastVertex : verticesSize;

	for(unsigned int vertex = startVertex; vertex < endVertex; ++vertex)
	{
		if( (vertices[3*vertex] == FLT_MAX) || (used[vertex] != 0) )
		{
			continue;
		}
		/*
		const struct arrayInfo testAi  = vertexToIndicesPointers[vertex];
		if(testAi.size < 4)
		{
			continue;
		}
		*/

		bool succeded = true;
		for(unsigned int b = vertex; b != UINT_MAX;)
		{
			const struct arrayInfo ai = vertexToIndicesPointers[b];

			if(ai.size < 1 && b == vertex)
			{
				succeded = false;
				break;// to next vertex
			}

			for(unsigned int index = 0; index < ai.size; ++index)
			{
				const unsigned int triangle = vertexToIndicesData[ai.position + index];
				for(unsigned int i = 0; i < 3; ++i)
				{
					const unsigned int triangleVertex = indices[3*triangle + i];
					if(triangleVertex == vertex)
					{
						continue;
					}
					for(unsigned int b2 = triangleVertex; b2 != UINT_MAX;)
					{
						const struct arrayInfo ai2 = vertexToIndicesPointers[b2];
						for(unsigned int index2 = 0; index2 < ai2.size; ++index2)
						{
							const unsigned int triangle2 = vertexToIndicesData[ai2.position + index2];
							const unsigned int v0 = indices[3*triangle2    ];
							const unsigned int v1 = indices[3*triangle2 + 1];
							const unsigned int v2 = indices[3*triangle2 + 2];
							//if(used[v0] != 0 || used[v1] != 0 || used[v2] != 0)
							if(used[v0] == 1 || used[v1] == 1 || used[v2] == 1)
							{
								succeded = false;
								break; // next vertex
							}
						}
						if(!succeded)
						{
							break;
						}
						b2 = ai2.continuesTo;
					}
					if(!succeded)
					{
						break;
					}
				}
				if(!succeded)
				{
					break;
				}

			}
			if(!succeded)
			{
				break; // to next vertex
			}
			b = ai.continuesTo;
		}
		if(!succeded)
		{
			continue; //to next vertex
		}

		//log_message("|");
/*
		//succeded, mark vertices as used
		for(unsigned int b = vertex; b != UINT_MAX;)
		{
			const struct arrayInfo ai = vertexToIndicesPointers[b];
			for(unsigned int index = 0; index < ai.size; ++index)
			{
				const unsigned int triangle = vertexToIndicesData[ai.position + index];
				for(unsigned int i = 0; i < 3; ++i)
				{
					const unsigned int triangleVertex = indices[3*triangle + i];
					if(triangleVertex == vertex)
					{
						continue;
					}
					for(unsigned int b2 = triangleVertex; b2 != UINT_MAX; )
					{
						const struct arrayInfo ai2 = vertexToIndicesPointers[b2];
						for(unsigned int index2 = 0; index2 < ai2.size; ++index2)
						{
							const unsigned int triangle2 = vertexToIndicesData[ai2.position + index2];
							const unsigned int v0 = indices[3*triangle2    ];
							const unsigned int v1 = indices[3*triangle2 + 1];
							const unsigned int v2 = indices[3*triangle2 + 2];

//							used[v0] = 2;
//							used[v1] = 2;
//							used[v2] = 2;


//							if(vertex != v0)
//								atom_max(&used[v0], 2); //set used verttices
//							if(vertex != v1)
//								atom_max(&used[v1], 2);
//							if(vertex != v2)
//								atom_max(&used[v2], 2);

							//log_message("m");
							//log_message("(%d)", triangle2);

							atom_xchg(used + v0, tid + 2);
							atom_xchg(used + v1, tid + 2);
							atom_xchg(used + v2, tid + 2);

						}
						b2 = ai2.continuesTo;
					}
				}

			}
			b = ai.continuesTo;
		}
*/
		//used[vertex] = 2;
		//atom_max(&used[vertex], 1); //set central vertex if unused, if used, ignore it
		//atom_cmpxchg(used + vertex, tid + 2, 1);
		used[vertex] = 1;
	}
	//log_message("markIndependentPoints3: end %d\n", tid);
}






/***********************************************************
| The last part of the parallel independent points algorithm
*/
__kernel void sweepIndependentPoints(
		__global unsigned int const * const indices,
		__global struct arrayInfo const * const vertexToIndicesPointers,
		__global unsigned int const * const vertexToIndicesData,
		__global double const * const errorArray,
		__global unsigned int const * const used,
		const unsigned int usedSize,
		__global unsigned int * const independentPoints,
		const unsigned int independentPointsSize,
		__global unsigned int * pointsFound
		)
{
	if(*pointsFound >= independentPointsSize){
		return;
	}

	bool successfulFind = true;

	const size_t tid = get_global_id(0);
	if(tid < usedSize && used[tid] == 1 )
	{
#if defined(LOGGING) && defined(LOG_INDEPENDENT_POINTS)
		//log_message("*%d - %d\n", tid, used[tid]);
		log_message(".");
#endif
		if(validateList(vertexToIndicesPointers, vertexToIndicesData, indices, tid) == -1){
			return;
		}

		for(unsigned int b = tid; b != UINT_MAX; )
		{
			struct arrayInfo ai = vertexToIndicesPointers[b];
			for(unsigned int index = 0; index < ai.size; ++index)
			{
				const unsigned int triangle = vertexToIndicesData[ai.position + index];
				for(unsigned int i = 0; i < 3; ++i)
				{
					const unsigned int triangleVertex = indices[3*triangle +i];
					if(triangleVertex == tid)
					{
						continue;
					}
					if(validateList(vertexToIndicesPointers, vertexToIndicesData, indices, triangleVertex) == -1){
						continue;
					}
					for(unsigned int b2 = triangleVertex; b2 != UINT_MAX; )
					{
						const struct arrayInfo ai2 = vertexToIndicesPointers[b2];
						for(unsigned int index2 = 0; index2 < ai2.size; ++index2)
						{
							const unsigned int triangle2 = vertexToIndicesData[ai2.position + index2];
							for(unsigned int i2 = 0; i2 < 3; ++i2)
							{
								unsigned int finalVertex = indices[3*triangle2 + i2];
								if(finalVertex == tid)
								{
									continue;
								}
								if(used[finalVertex] == 1 && errorArray[finalVertex] > errorArray[tid])//finalVertex > tid)
								{
#if defined(LOGGING) && defined(LOG_INDEPENDENT_POINTS)
									log_message("%d - %d\n", finalVertex, used[finalVertex]);
#endif
									//return;
									successfulFind = false;
									break;
								}
							}
							if(!successfulFind) break;
						}
						if(!successfulFind) return;
						b2 = ai2.continuesTo;
					}
					if(!successfulFind) break;
				}
				if(!successfulFind) break;
			}
			if(!successfulFind) break;
			b = ai.continuesTo;
		}

		//log_message(".");
		if(successfulFind)
		{
			const unsigned int pos = atom_inc(pointsFound);
			if(independentPointsSize > pos){
				independentPoints[pos] = tid;
			}
		}
	}
}






/***********************************************************
| The kernel that computes the quadrics for every triangle
*/
__kernel void computeTriangleQuadrics(
						__global unsigned int const *const triangles,
						__global float const * const points,
						__global double16 * const triangleQuadrics,
						unsigned int indices)
{
	size_t tid = get_global_id(0);

	if(!(tid < indices)){
		log_message("+");
		return ;
	}

	uint4 index = getTriangle(triangles, tid);

	float4 p1 = getVertex(points, index.x);
	float4 p2 = getVertex(points, index.y);
	float4 p3 = getVertex(points, index.z);

	double4 plane = trianglePlane(p1, p2, p3);
	//double area = computeFaceArea(p1, p2, p3);

	//area *= 10000000;
	//quadric: 4x4
	double16 Q = constructQuadric(plane, 1/* area*/);

	triangleQuadrics[tid] = Q;


#if defined(LOGGING) &&defined (LOG_COMPUTE_QUADRICS)
	if(tid == 0){
		log_message("computeTriangleQuadrics tid = %d\n", tid);
		log_message("------------------------\n");
		log_message("indices = %d\n", indices);
		log_message("p1 = (%f, %f, %f, %f)\n", p1.x, p1.y, p1.z, p1.w);
		log_message("plane = (%f, %f, %f, %f)\n", plane.x, plane.y, plane.z, plane.w);
		//log_message("area = %f (==0 (%d))\n", area, area == 0);
		printQuadric(Q);
		log_message("\n");
	}
#endif
}






__kernel void computeFinalQuadrics(
					__global double16 const *const triangleQuadrics, 
					__global double16 *const quadrics, 
					__global struct arrayInfo const *const vertexToIndicesPointers, 
					__global unsigned int const *const vertexToIndicesData,
					const unsigned int vertices)
{
	size_t tid = get_global_id(0);

	if(!(tid < vertices))
	{
		return;
	}
	
	struct arrayInfo vip = vertexToIndicesPointers[tid];
	size_t i;

	for(i=0; i< vip.size; ++i)
	{
		size_t currentTriangle = vertexToIndicesData[vip.position + i];
		
		/*
		double16 Q = quadrics[tid];
		Q += triangleQuadrics[currentTriangle];
		quadrics[tid] = Q;
		*/
		quadrics[tid] += triangleQuadrics[currentTriangle];
	}
#if defined(LOGGING) && defined(LOG_COMPUTE_QUADRICS)
	if(get_global_id(0) == 0)
	{
		log_message("computeFinalQuadrics\n");
		log_message("----------------\n");
		printQuadric(quadrics[tid]);
	}
#endif

}





/***********************************************************
| Helper function to access the indices array
*/
uint4 getTriangle(__global unsigned int const * const triangles, size_t index)
{
	return (uint4)(triangles[3*index], triangles[3*index+1], triangles[3*index+2], 0);
}






/***********************************************************
| Helper function to access the vertex array
*/
float4 getVertex(__global float const * const vertices, size_t index)
{
	return (float4)(vertices[3*index], vertices[3*index+1], vertices[3*index+2], 0.0f);
}


/***********************************************************
| Given the triangle's vertices (in counterclockwise order)
| it computes the normal vector
*/
double4 triangleNormal(float4 p1, float4 p2, float4 p3)
{
	double4 v1, v2;

	v1 = convert_double4(p2 - p1);
	v2 = convert_double4(p3 - p1);

	return normalize(cross(v1, v2));

}

/***********************************************************
| Given the points it calculates the parameters a, b, c, d 
| for the plane equation a*x + b*y + c*z + d = 0
|
| returns (double4)(a,b,c,d)
*/
double4 trianglePlane(float4 p1, float4 p2, float4 p3)
{
	double4 dp1 = convert_double4(p1);

	double4 normal = triangleNormal(p1, p2, p3);
	return (double4)(normal.x, normal.y, normal.z, -(dot(normal, dp1)));
}


/***********************************************************
| Helper function for the quadrics computation
*/
double computeFaceArea(float4 p1, float4 p2, float4 p3)
{

	double4 v1, v2;
	double4 n;

	v1 = convert_double4(p2 - p1);
	v2 = convert_double4(p3 - p1);

	n = cross(v1, v2);

#if defined(LOGGING) && defined (COMPUTE_QUADRICS)
	if(get_global_id(0) == 0)
	{

		log_message("computeFaceArea\n");
		log_message("p1 = (%f, %f, %f, %f)\n", p1.x, p1.y, p1.z, p1.w);
		log_message("p2 = (%f, %f, %f, %f)\n", p2.x, p2.y, p2.z, p2.w);
		log_message("p3 = (%f, %f, %f, %f)\n", p3.x, p3.y, p3.z, p3.w);
		log_message("v1 = (%f, %f, %f, %f)\n", v1.x, v1.y, v1.z, v1.w);
		log_message("v2 = (%f, %f, %f, %f)\n", v2.x, v2.y, v2.z, v2.w);
		log_message("length = %f\n", n);
	}
#endif
	return (double)0.5*sqrt(dot(n, n));
}

/***********************************************************
| A function that manages the creation of the quadrics
*/
double16 constructQuadric(double4 plane, double area)
{
	const double
		a = plane.x,
		b = plane.y,
		c = plane.z,
		d = plane.w;

	const double16 q = (double16)(
		(a*a), (a*b), (a*c), (a*d),
		(b*a), (b*b), (b*c), (b*d),
		(c*a), (c*b), (c*c), (c*d),
		(d*a), (d*b), (d*c), (d*d)
		);

/*
	double16 q;

	q.s0 = a*a;
	q.s1 = q.s4 = a*b;
	q.s2 = q.s8 = a*c;
	q.s3 = q.sc = a*d;
	q.s5 = b*b;
	q.s6 = q.s9 = b*c;
	q.s7 = q.sd = b*d;
	q.sa = c*c;
	q.sb = q.se = c*d;
	q.sf = d*d;
*/

#if defined(LOGGING) && defined(LOG_COMPUTE_QUADRICS)
	if(get_global_id(0) == 0)
	{
		log_message("constructQuadric\n");
		log_message("----------------\n");
		log_message("(a, b, c, d) = (%f, %f, %f, %f)\n", a, b, c, d);
		log_message("area = %f\n", area);
		printQuadric(q);
	}
#endif

	return q * area;
}











/***********************************************************
| A kernel that handles the computation of the error/weigth
*/
__kernel void computeDecimationError(__global float *points,  __global double16 *quadrics, __global double *errorArray, unsigned int vertices)
{
	size_t tid = get_global_id(0);
	if(!(tid < vertices)){
		return;
	}
	float4 v = getVertex(points, tid);
	errorArray[tid] = decimationError(v, quadrics[tid]);

#if defined(LOGGING) && defined(LOG_COMPUTE_ERROR)
	//if(tid == 0)
	{
		log_message("computeDecimationError tid = %d\n", tid);
		log_message("-------------------------------\n");
		log_message("v = (%f, %f, %f, %f)\n", v.x, v.y, v.z, v.w);
		printQuadric(quadrics[tid]);
		log_message("\n");
	}
	//if(tid < 10)
	{
		log_message("errorArray[%d] = %f\n", tid, errorArray[tid]);
	}
#endif
}






/***********************************************************
| compute the deciamtion error based on the quadrics using 
| the equation below
| Ä(v) = vT * Q * v
*/
double decimationError(const float4 point, const double16 quadric)
{
	const double4 dp = convert_double4(point);

	const double4 tmp = (double4)(
		dot(dp, quadric.lo.lo),
		dot(dp, quadric.lo.hi),
		dot(dp, quadric.hi.lo),
		dot(dp, quadric.hi.hi)
		);

#if defined(LOGGING) && defined(LOG_COMPUTE_ERROR)
	log_message("point = (%f, %f, %f, %f)\n", point.x, point.y, point.z, point.w);
	log_message("dp = (%f, %f, %f, %f)\n", dp.x, dp.y, dp.z, dp.w);
	printQuadric(quadric);
	log_message("tmp = (%f, %f, %f, %f)\n", tmp.x, tmp.y, tmp.z, tmp.w);
#endif

	return dot(tmp, dp);
}






/***********************************************************
| Misc helper functions
*/
void printQuadric(const double16 q)
{
#if defined(LOGGING)
	log_message("    / %f %f %f %f\\\n", q.s0, q.s1, q.s2, q.s3);
	log_message("Q = | %f %f %f %f|\n",  q.s4, q.s5, q.s6, q.s7);
	log_message("    | %f %f %f %f|\n",  q.s8, q.s9, q.sa, q.sb);
	log_message("    \\ %f %f %f %f/\n", q.sc, q.sd, q.se, q.sf);
#endif
}






/***********************************************************
| Bitonic Sort. 
*/

void localSwapValues(
			__global double const *const val,
			__local uint  * const key,
			const uint left,
			const uint right,
			const uint direction,
			const uint masterDirection
			)
{
	const uint  leftKey  = key[left ];
	const uint  rightKey = key[right];

	//const double leftVal  = val[leftKey ];
	//const double rightVal = val[rightKey];

	const double leftVal  = (leftKey  == UINT_MAX ? (masterDirection == ASCENDING ? DBL_MAX : -DBL_MAX) : val[leftKey ]);
	const double rightVal = (rightKey == UINT_MAX ? (masterDirection == ASCENDING ? DBL_MAX : -DBL_MAX) : val[rightKey]);

	const uint greaterKey = (leftVal > rightVal ? leftKey : rightKey);
	const uint lesserKey  = (leftVal > rightVal ? rightKey : leftKey);

	//log_message("--%d\t%d\t%d\t%d\t%f\t%f\n",left, right, leftKey, rightKey, leftVal, rightVal);

	key[left ] = (direction == ASCENDING ? lesserKey : greaterKey);
	key[right] = (direction == ASCENDING ? greaterKey : lesserKey);
}






void localSwapValues2(
			__local double *const val,
			__local uint  * const key,
			const uint left,
			const uint right,
			const uint direction
			)
{
	const uint  leftKey  = key[left ];
	const uint  rightKey = key[right];

	const double leftVal  = val[left ];
	const double rightVal = val[right];

	const uint greaterKey = (leftVal > rightVal ? leftKey : rightKey);
	const uint lesserKey  = (leftVal > rightVal ? rightKey : leftKey);

	const double greaterVal = max(leftVal, rightVal);
	const double lesserVal  = min(leftVal, rightVal);


	//log_message("--%d\t%d\t%d\t%d\t%f\t%f\n",left, right, leftKey, rightKey, leftVal, rightVal);

	key[left ] = (direction == ASCENDING ? lesserKey : greaterKey);
	key[right] = (direction == ASCENDING ? greaterKey : lesserKey);

	val[left ] = (direction == ASCENDING ? lesserVal : greaterVal);
	val[right] = (direction == ASCENDING ? greaterVal : lesserVal);
}




void globalSwapValues(
			__global double const *const val,
			__global uint  * const key,
			const uint left,
			const uint right,
			const uint direction
			)
{
	const uint  leftKey  = key[left ];
	const uint  rightKey = key[right];

	const double leftVal  = val[leftKey ];
	const double rightVal = val[rightKey];

	uint  greaterKey, lesserKey;


	if(leftVal > rightVal){
		greaterKey = leftKey;
		lesserKey = rightKey;
	}
	else
	{
		greaterKey = rightKey;
		lesserKey = leftKey;
	}

	key[left ] = direction ? lesserKey : greaterKey;

	key[right]  = direction ? greaterKey : lesserKey;
}





__kernel void localBitonicSort(
							   __global double const * const errorArray,
							   __global unsigned int * const independentPoints,
							   const unsigned int pointsFound,
							   const unsigned int sortDirection,
							   __local unsigned int * const localKeys
							   )
{
	size_t tid = get_global_id(0);
	size_t lid = get_local_id(0);
	//size_t gid = get_group_id(0);

	// find the power of 2 equal or greater to pointsFound
	uint po2;
	for(po2 = 1; po2 < pointsFound; po2 <<= 1)
	{
	}

	localKeys[lid] = independentPoints[lid];
#if defined(LOGGING) && defined(LOG_LOCAL_SORT)
	log_message("move to local memory %d\n", lid);
#endif

	if(lid + po2/2 < pointsFound )
	{
#if defined(LOGGING) && defined(LOG_LOCAL_SORT)
		log_message("move to local memory %d\n", lid + po2/2);
#endif
		localKeys[lid + po2/2] = independentPoints[lid + po2/2];
	}
	else
	{
		localKeys[lid + po2/2] = UINT_MAX;
	}

	for(uint size = 2; size <= po2; size <<= 1)
	{
		//Bitonic Merge
		uint dir;
		if(sortDirection == DESCENDING)
		{
			dir = ((lid & (size/2)) != 0);
		}
		else
		{
			dir = ((lid & (size/2)) == 0);
		}
#if defined(LOGGING) && defined(LOG_LOCAL_SORT)
		log_message("lid(%d) size(%d) dir(%d)\n",lid, size,  dir);
#endif
		for(uint stride = size/2; stride > 0; stride >>= 1)
		{
			barrier(CLK_LOCAL_MEM_FENCE);
			const uint pos = 2*lid - (lid & (stride -1));
			//const uint pos2 = pos + 2*stride -1 - 2*(lid%stride);
			const uint pos2 = pos + stride;

			//if(pos2 < pointsFound)
			{
#if defined(LOGGING) && defined(LOG_LOCAL_SORT)
				log_message("lid(%d): size(%d) dir(%c) comparing  stride(%d)\t%d\t%d\n", lid,size, (dir == true ? 't' : 'f'), stride, pos, pos2);
#endif
				localSwapValues(errorArray, localKeys, pos, pos2, dir, sortDirection);
			}
		}
	}

	//dir = sortdir for the last bitonic merge step
	for(uint stride = po2/2; stride > 0; stride >>= 1)
	{
		barrier(CLK_LOCAL_MEM_FENCE);
		uint pos = 2*lid - (lid & (stride -1));
		//const uint pos2 = pos + 2*stride -1 - 2*(lid%stride);
		const uint pos2 = pos + stride;
		//if(pos2 < pointsFound)
		{
#if defined(LOGGING) && defined(LOG_LOCAL_SORT)
			//log_message("lid(%d): final comparing pos(%d) stride(%d)\n", lid, pos, stride);
			log_message("lid(%d): size(%d) dir(%c) comparing  stride(%d)\t%d\t%d\n", lid, po2, (sortDirection == true ? 't' : 'f'), stride, pos, pos2);
#endif
			localSwapValues(errorArray, localKeys, pos, pos2, sortDirection, sortDirection);
		}
	}
#if defined(LOGGING) && defined(LOG_LOCAL_SORT)
	log_message("move to global memory %d = %d\n", lid, localMemory[lid]);
#endif
	independentPoints[lid] = localKeys[lid];

	if(lid + po2/2 < pointsFound )
	{
#if defined(LOGGING) && defined(LOG_LOCAL_SORT)
		log_message("move to global memory %d = %d\n", lid + po2/2, localMemory[lid + po2/2]);
#endif
		independentPoints[lid + (po2/2) ] = localKeys[lid + po2/2];
	}
	return;
}





__kernel 
void globalBitonicSort(
			__global double const * const val,
			__global uint  * const key,
            const uint stage, 
            const uint passOfStage,
			const uint arrayLength,
            const uint direction
			)
{
    uint sortIncreasing = direction;
    uint threadId = get_global_id(0);

    uint pairDistance = 1 << (stage - passOfStage);
    uint blockWidth   = 2 * pairDistance;

    uint leftId = (threadId % pairDistance) 
                   + (threadId / pairDistance) * blockWidth;

    uint rightId = leftId + pairDistance;

	//*
	if(
		//(threadId >= arrayLength/2) ||
		(leftId >= arrayLength) || 
		(rightId >= arrayLength))
	{
#if defined(LOGGING) && defined(LOG_SORTING)
		log_message("- tid = %d\tleft = %d\trigth = %d\n", threadId, leftId, rightId);
#endif
		return;
	}
#if defined(LOGGING) && defined(LOG_SORTING)
	else{
		log_message("+ tid = %d\tleft = %d\trigth = %d\n", threadId, leftId, rightId);
	}
#endif
	//*/

	//log_message("comparing tid = %d\tleft = %d\trigth = %d\n", threadId, leftId, rightId);
    
    uint sameDirectionBlockWidth = 1 << stage;
    
    sortIncreasing = ( threadId / sameDirectionBlockWidth ) % 2 == 1 ?
        1 - sortIncreasing : sortIncreasing;

	globalSwapValues(val, key, leftId, rightId, sortIncreasing);
}






/***********************************************************
| Bitonic Merge. 
*/
__kernel
void globalBitonicMerge(
			__global double const * const val,
			__global uint * const key,
			const uint stage,
			const uint passOfStage,
			const uint arrayLength,
			const uint direction
			)
{

	const size_t tid = get_global_id(0);

	const uint blockSize = 2 << (stage - passOfStage);
	const uint blockStart = (tid >> (stage - passOfStage))*blockSize;
	const uint blockLeftId = tid % (blockSize/2);
	const uint blockRightId = blockSize - blockLeftId - 1;

	const uint leftId  = blockStart + blockLeftId ;
	const uint rightId = blockStart + blockRightId;

	if(
		(leftId  >= arrayLength) ||
		(rightId >= arrayLength)
		)
	{
#if defined(LOGGING) && defined(LOG_SORTING)
		log_message("- tid = %d\tleft = %d\tright = %d\n", tid, leftId, rightId);
#endif
		return;
	}
#if defined(LOGGING) && defined(LOG_SORTING)
	else
	{
		log_message("+ tid = %d\tleft = %d\tright = %d\n", tid, leftId, rightId);
	}
#endif

	globalSwapValues(val, key, leftId, rightId, direction);
	return;
}




__kernel void globalBitonicSortARB(
							  __global double const * const values,
							  __global unsigned int * const keys,
							  const int arrayLength,
							  const int sortDirection,
							  const int stage,
							  const int passOfStage
							  )
{
	const size_t tid =  get_global_id(0);

	const int pairDistance = 1 << (stage - passOfStage);
	const int subBlockSize = 2*pairDistance;
	const int subBlockStart = (tid / pairDistance) *subBlockSize;
	const int upPos   = (tid % pairDistance) + subBlockStart;
	const int downPos = upPos + pairDistance;

	const int blockSize = 2 << stage;
	const int lastBlock = (arrayLength-1) / blockSize;
	const int blockId = 2* tid / blockSize;
	const int lastBlockDirection = sortDirection;
	const int otherBlockDirection = sortDirection == ASCENDING ? DESCENDING : ASCENDING;

	const int blockDirection = (blockId%2 == lastBlock%2 ? lastBlockDirection : otherBlockDirection);

	//if(!tid) log_message("blockSize = %d\n", blockSize);

	//log_message("last block = %d\n", lastBlock);
	if(downPos < arrayLength)
	{
		//log_message("++++global sort arb: tid(%d) lblock(%d) block(%d): %d\t%d\t%c\n", tid, lastBlock, blockId, upPos, downPos, (blockDirection == ASCENDING ? 'a' : 'd') );
		globalSwapValues(values, keys, upPos, downPos, blockDirection);
	}
	else
	{
		//log_message("----global sort arb: tid(%d) lblock(%d) block(%d): %d\t%d\t%c\n", tid, lastBlock, blockId, upPos, downPos, (blockDirection == ASCENDING ? 'a' : 'd') );
	}
	return;
}






__kernel void localBitonicPreSortARB(
									 __global double const * const values,
									 __global unsigned int * const keys,
									 const int arrayLength,
									 const int sortDirection,
									 const int stages,
									 __local double * const localValues,
									 __local unsigned int * const localKeys
									 )
{
	const size_t tid = get_global_id(0);
	const size_t lid = get_local_id(0);
	const size_t gid = get_group_id(0);
	const size_t groups = get_num_groups(0);
	const size_t lsize = get_local_size(0);

	//read
	const size_t read_pos1 = gid * 2*lsize + lid;
	const size_t read_pos2 = gid * 2*lsize + lid + lsize;

	if(read_pos1 < arrayLength)
	{
		//if(!gid) log_message("pos1 localKeys[%d] = keys[%d]\n", lid, read_pos1);
		localKeys[lid] = keys[read_pos1];
		localValues[lid] = values[localKeys[lid]];
		if(read_pos2 < arrayLength)
		{
			//if(!gid) log_message("pos2 localKeys[%d] = keys[%d]\n", lid, read_pos1);
			localKeys[lid + lsize] = keys[read_pos2];
			localValues[lid+lsize] = values[localKeys[lid + lsize]];
		}
	}

	//sort

	for(int stage = 0; stage < stages; ++stage)
	{
		const int blockSize = 2 << stage;
		const int globalBlockId = (2*lid+gid*lsize*2) / blockSize;
		const int lastBlockId = (arrayLength-1)/blockSize;
		const int lastBlockDirection = sortDirection;
		const int otherBlockDirection = (lastBlockDirection == ASCENDING ? DESCENDING : ASCENDING);
		const int blockDirection = (globalBlockId %2 == lastBlockId%2 ? lastBlockDirection : otherBlockDirection);

		for(int passOfStage = 0; passOfStage <= stage; ++passOfStage)
		{
			const int pairDistance = 1 << (stage - passOfStage);
			const int subBlockSize = 2 * pairDistance;
			const int subBlockStart = (lid / pairDistance) * subBlockSize;
			const int upPos = (lid % pairDistance) + subBlockStart;
			const int downPos = upPos + pairDistance;

			barrier(CLK_LOCAL_MEM_FENCE);

			if(downPos + gid * lsize*2 < arrayLength)
			{
				localSwapValues2(localValues, localKeys, upPos, downPos, blockDirection);
			}
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	//write back
	if(read_pos1 < arrayLength)
	{
		keys[read_pos1] = localKeys[lid];
		if(read_pos2 < arrayLength)
		{
			keys[read_pos2] = localKeys[lid + lsize];
		}
	}
	return;
}






__kernel void localBitonicMergeARB(
								   __global double * const values,
								   __global unsigned int * const keys,
								   const int arrayLength,
								   const int sortDirection,
								   __local double * const localValues,
								   __local unsigned int * const localKeys,
								   const int stage,
								   const int startPassOfStage
								   )
{
	//get contxt
	const size_t tid = get_global_id(0);
	const size_t lid = get_local_id(0);
	const size_t gid = get_group_id(0);
	const size_t groups = get_num_groups(0);
	const size_t lsize = get_local_size(0);

	//read values
	const size_t read_pos1 = gid * 2*lsize + lid;
	const size_t read_pos2 = gid * 2*lsize + lid + lsize;

	if(read_pos1 < arrayLength)
	{
		//if(!gid) log_message("pos1 localKeys[%d] = keys[%d]\n", lid, read_pos1);
		localKeys[lid] = keys[read_pos1];
		localValues[lid] = values[localKeys[lid]];
		if(read_pos2 < arrayLength)
		{
			//if(!gid) log_message("pos2 localKeys[%d] = keys[%d]\n", lid, read_pos1);
			localKeys[lid + lsize] = keys[read_pos2];
			localValues[lid+lsize] = values[localKeys[lid + lsize]];
		}
	}
	//process stages/passes
	const int blockSize = 2 << stage;
	const int globalBlockId = (2*lid+gid*lsize*2) / blockSize;
	const int lastBlockId = (arrayLength-1)/blockSize;
	const int lastBlockDirection = sortDirection;
	const int otherBlockDirection = (lastBlockDirection == ASCENDING ? DESCENDING : ASCENDING);
	const int blockDirection = (globalBlockId %2 == lastBlockId%2 ? lastBlockDirection : otherBlockDirection);

	for(int passOfStage = startPassOfStage; passOfStage <= stage; ++passOfStage)
	{
		const int pairDistance = 1 << (stage - passOfStage);
		const int subBlockSize = 2 * pairDistance;
		const int subBlockStart = (lid / pairDistance) * subBlockSize;
		const int upPos = (lid % pairDistance) + subBlockStart;
		const int downPos = upPos + pairDistance;

		barrier(CLK_LOCAL_MEM_FENCE);

		if(downPos + gid * lsize*2 < arrayLength)
		{
			localSwapValues2(localValues, localKeys, upPos, downPos, blockDirection);
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	//write back
	if(read_pos1 < arrayLength)
	{
		keys[read_pos1] = localKeys[lid];
		if(read_pos2 < arrayLength)
		{
			keys[read_pos2] = localKeys[lid + lsize];
		}
	}
	return;
}







/****************************/
/*        DECIMATION        */
/****************************/




uint2 getCommonTriangles(	
		__global struct arrayInfo const * const vertexToIndicesPointers,
		__global unsigned int const *const vertexToIndicesData,
		const unsigned int firstVertex,
		const unsigned int secondVertex
		)
{
	struct arrayInfo firstInfo;
	struct arrayInfo secondInfo;
	uint2 retVal = (uint2)(UINT_MAX, UINT_MAX);

	unsigned int trianglesFound = 0;

	for(unsigned int firsBlock = firstVertex; firsBlock != UINT_MAX; firsBlock = firstInfo.continuesTo)
	{
		firstInfo = vertexToIndicesPointers[firsBlock];
		for(unsigned int i=0; i< firstInfo.size; ++i)
		{
			const unsigned int firstTriangle  = vertexToIndicesData[firstInfo.position + i ];

			for(unsigned int secondBlock = secondVertex; secondBlock != UINT_MAX; secondBlock = secondInfo.continuesTo)
			{
				secondInfo = vertexToIndicesPointers[secondBlock];

				for(unsigned int j=0; j< secondInfo.size; ++j)
				{
					const unsigned int secondTriangle = vertexToIndicesData[secondInfo.position + j];

					if(firstTriangle == secondTriangle)
					{
						++trianglesFound;
						
						if(retVal.s0 == UINT_MAX)
						{
							retVal.s0 = firstTriangle;
						}
						else
						{
							retVal.s1 = firstTriangle;
							return retVal;
						}
					}
					if(trianglesFound == 2) break;
				}
				if(trianglesFound == 2) break;
			}
			if(trianglesFound == 2) break;
		}
		if(trianglesFound == 2) break;
	}
	return retVal;
}






/***********************************************************
| Make all triangles that point to the central (first) vertex
| to point to the final (second) vertex.
*/
void rewireTriangles(	
		__global unsigned int *const indices,
		__global struct arrayInfo const * const vertexToIndicesPointers,
		__global unsigned int const * const vertexToIndicesData,
		const unsigned int firstVertex,
		const unsigned int secondVertex)
{
	struct arrayInfo firstInfo;

	// for blocks
	for(unsigned int block = firstVertex; block != UINT_MAX; block = firstInfo.continuesTo)
	{
		firstInfo = vertexToIndicesPointers[block];
//		firstInfo = vertexToIndicesPointers[firstVertex];


		//for triangles in block
		for(unsigned int i=0; i< firstInfo.size; ++i)
		{
			const unsigned int triangle = vertexToIndicesData[firstInfo.position + i ];

			//for vertices int triangle
			for(unsigned int j=0; j < 3; ++j)
			{
				if(indices[triangle*3 + j] == firstVertex)
				{
					indices[triangle*3 + j] = secondVertex;
				}
				/*
				maybe equevalnet to ....
				atom_cmpxchg(indices + triangle*3 + i, firstVertex, secondVertex);
				but must be faster without the atomic operation
				*/
			}
		}
	}
}






/***********************************************************
| Computes the determinant of a 4x4 matrix stored in a double16
*/
double det4x4(const double16 q)
{
	const double det =
		+ q.s0 * vdet3x3(q,	5, 6, 7,
							9, a, b,
							d, e, f)

		- q.s1 * vdet3x3(q,	4, 6, 7,
							8, a, b,
							c, e, f)

		+ q.s2 * vdet3x3(q,	4, 5, 7,
							8, 9, b,
							c, d, f)

		- q.s3 * vdet3x3(q,	4, 5, 6,
							8, 9, a,
							c, d, e);

	return det;

}





/***********************************************************
| Inverts a a 4x4 matrix stored in a double16
*/
double16 invert4x4(const double16 q, const double det)
{
	double16 ret;

	// ret = cofactor matrix of q
	ret.s0 = +vdet3x3(q, 5, 6, 7, 9, a, b, d, e, f);
	ret.s1 = -vdet3x3(q, 4, 6, 7, 8, a, b, c, e, f);
	ret.s2 = +vdet3x3(q, 4, 5, 7, 8, 9, b, c, d, f);
	ret.s3 = -vdet3x3(q, 4, 5, 6, 8, 9, a, c, d, e);

	ret.s4 = -vdet3x3(q, 1, 2, 3, 9, a, b, d, e, f);
	ret.s5 = +vdet3x3(q, 0, 2, 3, 8, a, b, c, e, f);
	ret.s6 = -vdet3x3(q, 0, 1, 3, 8, 9, b, c, d, f);
	ret.s7 = +vdet3x3(q, 0, 1, 2, 8, 9, a, c, d, e);

	ret.s8 = +vdet3x3(q, 1, 2, 3, 5, 6, 7, d, e, f);
	ret.s9 = -vdet3x3(q, 0, 2, 3, 4, 6, 7, c, e, f);
	ret.sa = +vdet3x3(q, 0, 1, 3, 4, 5, 7, c, d, f);
	ret.sb = -vdet3x3(q, 0, 1, 2, 4, 5, 6, c, d, e);

	ret.sc = -vdet3x3(q, 1, 2, 3, 5, 6, 7, 9, a, b);
	ret.sd = +vdet3x3(q, 0, 2, 3, 4, 6, 7, 8, a, b);
	ret.se = -vdet3x3(q, 0, 1, 3, 4, 5, 7, 8, 9, b);
	ret.sf = +vdet3x3(q, 0, 1, 2, 4, 5, 6, 8, 9, a);

	// ret = adjugate matrix of q, adj(q)
	ret = ret.s048c159d26ae37bf;

	//ret = inverce matrix of q
	ret  = ret/det;

	return ret;
}





/***********************************************************
| Inverts a a 3x3 matrix stored in a double16
*/
double16 invert3x3(const double16 q, const double det)
{
	double16 ret;
	
	ret.s0 = +vdet2x2(q, 5, 6, 9, a);
	ret.s1 = -vdet2x2(q, 4, 6, 8, a);
	ret.s2 = +vdet2x2(q, 4, 5, 8, 9);
	ret.s3 = 0;
	ret.s4 = -vdet2x2(q, 1, 2, 9, a);
	ret.s5 = +vdet2x2(q, 0, 2, 8, a);
	ret.s6 = -vdet2x2(q, 0, 1, 8, 9);
	ret.s7 = 0;
	ret.s8 = +vdet2x2(q, 1, 2, 5, 6);
	ret.s9 = -vdet2x2(q, 0, 2, 4, 6);
	ret.sa = +vdet2x2(q, 0, 1, 4, 5);
	ret.sb = 0;
	ret.sc = 0;
	ret.sd = 0;
	ret.se = 0;
	ret.sf = 0;

	ret = ret / det;

	return ret;
}




/***********************************************************
| Multiplies a 4x4 matrix vith a 4x1 vector
*/
double4 multiplyMatrixVector(const double16 m, const double4 v)
{
	const double4 ret = (double4)(
		dot(m.lo.lo, v),
		dot(m.lo.hi, v),
		dot(m.hi.lo, v),
		dot(m.hi.hi, v)
	);

	return ret;
}





/***********************************************************
| Computes the optimal placement of the vertex based on the
| Quadrics and sets the decimation error of the final point 
*/
float4 findOptimalPlacement(
		__global float const *const vertices,
		__global double16 const *const quadrics,
		const unsigned int firstVertex,
		const unsigned int secondVertex
		)
{

	float4 finalPos = (float4)(0.0f, 0.0f, 0.0f, 0.0f);

	const double16 Q = quadrics[firstVertex] + quadrics[secondVertex];
	const double det = det4x4(Q);

#if defined(LOGGING) && defined(LOG_PLACEMENT)
	if(get_global_id(0) == 0)
	{
		log_message("findAndSetOptimalPlacement\n");
		log_message("--------------------------\n");
		printQuadric(quadrics[firstVertex]);
		log_message("det = %f\n", det4x4(quadrics[firstVertex]));
		log_message("err = %f\n\n", errorArray[firstVertex]);
		printQuadric(quadrics[secondVertex]);
		log_message("det = %f\n", det4x4(quadrics[secondVertex]));
		log_message("err = %f\n\n", errorArray[secondVertex]);
		printQuadric(Q);
	}
#endif

//	if(1e-12 < det)
	if(DBL_EPSILON < det)
//	if(0 != det)
	{
        //log_message("-");
		const double det2 = vdet3x3(Q, 0, 1, 2, 4, 5, 6, 8, 9, a);
		const double16 tmp = invert3x3(Q, det2);
		const double4 v = multiplyMatrixVector(tmp, Q.s37bf);

		finalPos = convert_float4(v);
	}
	else

	{
        //log_message("|");
		//special case
		finalPos.x = vertices[firstVertex*3    ] + vertices[secondVertex*3    ];
		finalPos.y = vertices[firstVertex*3 + 1] + vertices[secondVertex*3 + 1];
		finalPos.z = vertices[firstVertex*3 + 2] + vertices[secondVertex*3 + 2];

		finalPos /= 2.0f;
		finalPos.w = 1.0f;
	}
/*
	quadrics[secondVertex] = Q;
	errorArray[secondVertex] = decimationError(finalPos, Q);

	vertices[3*secondVertex    ] = finalPos.x;
	vertices[3*secondVertex + 1] = finalPos.y;
	vertices[3*secondVertex + 2] = finalPos.z;
*/
	return finalPos;
}





bool foldOverOccurs(
					const uint v1, 
					const uint v2, 
					const float4 vfinal, 
					__global const float * const vertices,
					__global const uint *const indices,
					__global const struct arrayInfo * const vertexToIndicesPointers, 
					__global const uint * const vertexToIndicesData
					)
{
	for(uint b = v1; b != UINT_MAX; )
	{
		const struct arrayInfo ai = vertexToIndicesPointers[b];
		for(uint t = 0; t < ai.size; ++t)
		{
			const uint triangleID = vertexToIndicesData[ai.position + t];
			const uint4 triangle = getTriangle(indices, triangleID);
			if(triangle.x == v2 || triangle.y == v2 || triangle.z == v2)
			{
				//this trialgle will be removed from the model at a next step of the algorithm
				continue;
			}

			const float4 vertex1 = getVertex(vertices, triangle.x);
			const float4 vertex2 = getVertex(vertices, triangle.y);
			const float4 vertex3 = getVertex(vertices, triangle.z);
			const double4 normal1 = triangleNormal(vertex1, vertex2, vertex3);

			const float4 vertex4 = triangle.x == v1 ? vfinal : vertex1;
			const float4 vertex5 = triangle.y == v1 ? vfinal : vertex2;
			const float4 vertex6 = triangle.z == v1 ? vfinal : vertex3;
			const double4 normal2 = triangleNormal(vertex4, vertex5, vertex6);

			if(dot(normal1, normal2) <= 0)
			{
				//log_message("dot = %f\n",dot(normal1, normal2));
				return true;
			}
		}
		b = ai.continuesTo;
	}

	return false;
}





bool validEdgeContraction(
		__global float const * const vertices,
		__global unsigned int const * const indices,
		__global struct arrayInfo const * const vertexToIndicesPointers,
		__global unsigned int const * const vertexToIndicesData,
		__global double16 const * const quadrics,
		const unsigned int v0,
		const unsigned int v1
		)
{
	unsigned int commonNeighbors = 0;

	for(unsigned int b0 = v0; b0 != UINT_MAX; )
	{
		const struct arrayInfo ai0 = vertexToIndicesPointers[b0];
		for(unsigned int i0 = 0; i0 < ai0.size; ++i0)
		{
			const unsigned int testV0 = vertexToIndicesData[ai0.position + i0];

			for(unsigned int b1 = v1; b1 != UINT_MAX; )
			{
				const struct arrayInfo ai1 = vertexToIndicesPointers[b1];
				for(unsigned int i1 = 0; i1 < ai1.size; ++i1)
				{
					const unsigned int testV1 = vertexToIndicesData[ai1.position + i1];
					if(testV0 == testV1)
					{
						++commonNeighbors;
					}
				}
				b1 = ai1.continuesTo;
			}
		}
		b0 = ai0.continuesTo;
	}

	if(commonNeighbors > 2)
	{
		return false;
	}

	//perform contraction

	const float4 finalPos = findOptimalPlacement(vertices, quadrics, v0, v1);

	if(
		foldOverOccurs(v0, v1, finalPos, vertices, indices, vertexToIndicesPointers, vertexToIndicesData)
		||
		foldOverOccurs(v1, v0, finalPos, vertices, indices, vertexToIndicesPointers, vertexToIndicesData)
		)
	{
		return false;
	}

	return true;
}

/***********************************************************
* Scan through the surrounding triangles to find the best 
* neighbor vertex (lowest error associated)
*/
unsigned int getBestNeighbor(
		 __global float const * const vertices,
		__global unsigned int const * const indices,
		__global struct arrayInfo const * const vertexToIndicesPointers,
		__global unsigned int const * const vertexToIndicesData,
		__global double16 const * const quadrics,
		__global double const * const errorArray,
		const unsigned int centralVertex
		)
{
	struct arrayInfo vtip;
	__global unsigned int const * vtid;
	unsigned int bestMatch = UINT_MAX;
	unsigned int bestError = UINT_MAX;
/*
	//verify that has neighbors
	bool hasNeighbors = false;

	for(unsigned int b = centralVertex; b != UINT_MAX; b = vtip.continuesTo)
	{
		vtip = vertexToIndicesPointers[b];
		vtid = vertexToIndicesData + vtip.position;

		for(unsigned int i = 0; i < vtip.size; ++i)
		{
			const unsigned int triangle = 3*vtid[i];
			for(unsigned int j = 0; j < 3; ++j)
			{
				const unsigned int tmpVertex = indices[triangle + j];
				if(tmpVertex == centralVertex)
				{
					continue;
				}

				hasNeighbors = true;
			}
		}
	}

//#if defined(LOGGING)
	if(!hasNeighbors)
	{
		log_message("getBestNeighbor: %d doesn't have neighbors\n", centralVertex);
		return UINT_MAX;
	}
//#endif
*/
	for(unsigned int b = centralVertex; b != UINT_MAX; b = vtip.continuesTo)
	{
		vtip = vertexToIndicesPointers[b];
//		vtip = vertexToIndicesPointers[centralVertex];
		vtid = vertexToIndicesData + vtip.position;

		for(unsigned int i = 0; i < vtip.size; ++i)
		{
			const unsigned int triangle = vtid[i];
			for(unsigned int j=0; j < 3; ++j)
			{
				const unsigned int tmpVertex = indices[triangle*3 + j];

				if( (tmpVertex == UINT_MAX) || (tmpVertex == centralVertex) || (tmpVertex == bestMatch) )
				{
					continue;
				}

				//const unsigned int tmpError = errorArray[tmpVertex];
				const double tmpError = errorArray[tmpVertex];
				if( (tmpError < bestError) && validEdgeContraction(vertices, indices, vertexToIndicesPointers, vertexToIndicesData, quadrics, centralVertex, tmpVertex) )
				{
					bestError = tmpError;
					bestMatch = tmpVertex;
				}
			}
		}
	}

	return bestMatch;
}



/***********************************************************
* Removes the indices from the second list that exists in the
* first list and expands the first list to the second.
*/
//if we can assure that the arrays are sorted it can be faster!!!
void mergeVertexToIndicesLists(
		__global struct arrayInfo *const vertexToIndicesPointers,
		__global unsigned int *const vertexToIndicesData,
		const unsigned int base,
		const unsigned int toAppend
		)
{
	struct arrayInfo baseAI;
	struct arrayInfo toAppendAI;
	unsigned int finalBaseBlock = 0;

	//remove duplicate values
	// baseBlocks * elementsPerBlock * toAppendBlocks * elementsPerBlock = O(m*n)
	for(unsigned int baseBlock = base; baseBlock != UINT_MAX; baseBlock = baseAI.continuesTo)
	{
		baseAI = vertexToIndicesPointers[baseBlock];
		for(unsigned int baseEnumerator = 0; baseEnumerator < baseAI.size; ++baseEnumerator)
		{
			const unsigned int baseElement = vertexToIndicesData[baseAI.position + baseEnumerator];
			for(unsigned int toAppendBlock = toAppend; toAppendBlock != UINT_MAX; toAppendBlock = toAppendAI.continuesTo)
			{
				toAppendAI = vertexToIndicesPointers[toAppendBlock];
				for(unsigned int toAppendEnumerator = 0; toAppendEnumerator < toAppendAI.size; ++toAppendEnumerator)
				{
					__global unsigned int *const toAppendElement = vertexToIndicesData + toAppendAI.position + toAppendEnumerator;
					if(baseElement == *toAppendElement)
					{
						*toAppendElement = UINT_MAX;
					}
				}
			}
		}
		finalBaseBlock = baseBlock;
	}

	//connect the lists
	vertexToIndicesPointers[finalBaseBlock].continuesTo = toAppend;
}





/***********************************************************
* Removes the emty elements of the list and the triangles to
* be invalidated and packs the list so there are no holes
*/
void packListElements(
		__global struct arrayInfo *const vertexToIndicesPointers,
		__global unsigned int * const vertexToIndicesData,
		const unsigned int maxVertexToIndices,
		const unsigned int base,
		const unsigned int firstTriangleToRemove,
		const unsigned int secondTriangleToRemove
		)
{
	__global struct arrayInfo * writeAI = vertexToIndicesPointers + base;
	unsigned int writeSize = 0;

	for(unsigned int readBlock = base; readBlock != UINT_MAX; )
	{
		const struct arrayInfo readAI = vertexToIndicesPointers[readBlock];

		for(unsigned int i = 0;  i < readAI.size; ++i)
		{
			const unsigned int value = vertexToIndicesData[readAI.position + i];

			if((value == firstTriangleToRemove) || (value == secondTriangleToRemove) )
			{
				//skip the current triangle
			}
			else
			{
				//write the current triangle
				vertexToIndicesData[writeAI->position + writeSize] = value;

				++writeSize;

				if(writeSize == maxVertexToIndices)
				{
					writeAI->size = maxVertexToIndices;
					writeAI = vertexToIndicesPointers + writeAI->continuesTo;
					writeSize = 0;
				}
			}
		}

		readBlock = readAI.continuesTo;
	}
	
	writeAI->size = writeSize;
	writeAI->continuesTo = UINT_MAX;

}





uint2 getTopBottom(
		__global unsigned int const * const indices,
		const unsigned int v1,
		const unsigned int v2,
		uint2 commonTriangles
		)
{
	uint2 result = (uint2)(UINT_MAX, UINT_MAX);

	const uint4 t1 = getTriangle(indices, commonTriangles.s0);

	if(t1.s0 != v1 && t1.s0 != v2)
	{
		result.s0 = t1.s0;
	}
	else if( t1.s1 != v1 && t1.s1 != v2)
	{
		result.s0 = t1.s1;
	}
	else
	{
		result.s0 = t1.s2;
	}

	if(commonTriangles.s1 != UINT_MAX)
	{
		const uint4 t2 = getTriangle(indices, commonTriangles.s1);
		if(t2.s0 != v1 && t2.s0 != v2)
		{
			result.s1 = t2.s0;
		}
		else if( t2.s1 != v1 && t2.s1 != v2)
		{
			result.s1 = t2.s1;
		}
		else
		{
			result.s1 = t2.s2;
		}
	}

	return result;
}






void connectLists(
	__global struct arrayInfo *const vertexToIndicesPointers,
	const unsigned int base,
	const unsigned int toAppend
	)
{
	__global struct arrayInfo * vtip = vertexToIndicesPointers + base;

	for(unsigned int b = base; b != UINT_MAX; )
	{
		vtip = vertexToIndicesPointers + b;
		b = vtip->continuesTo;
	}

	vtip->continuesTo = toAppend;
}





void printList(
		__global struct arrayInfo const*const vertexToIndicesPointers,
		__global unsigned int const * const vertexToIndicesData,
		const unsigned int base
		)
{
	log_message("List for vertex %d\n", base);
	for(unsigned int b = base; b != UINT_MAX;)
	{
		struct arrayInfo ai = vertexToIndicesPointers[b];
		for(unsigned int i = 0; i < ai.size; ++i)
		{
			unsigned int value = vertexToIndicesData[ai.position + i];
			if(value == UINT_MAX)
			{
				log_message("\tUINT_MAX");
			}
			else{
				log_message("\t%d", value);
			}
		}
		log_message("\n");
		b = ai.continuesTo;
	}
	log_message("\n");
}





int validateList(
		__global struct arrayInfo const*const vertexToIndicesPointers,
		__global unsigned int const * const vertexToIndicesData,
		__global unsigned int const * const indices,
		const unsigned int base
		)
{

	for(unsigned int b = base; b != UINT_MAX; )
	{
		struct arrayInfo ai = vertexToIndicesPointers[b];
		for(unsigned int i = 0; i < ai.size; ++i)
		{
			unsigned int ind = vertexToIndicesData[ai.position + i];

			if(ind == UINT_MAX)
			{
				log_message("validate List(%d): invalid list (bad entry)\n", base);
				return -1;
			}

			if(indices[ind*3] == UINT_MAX)
			{
				log_message("validate List (%d): invalid list (bad triangle ind = %d)\n", base, ind);
				return -1;
			}
		}
		b = ai.continuesTo;
	}
	return 0;
}





__kernel void decimateOnPoint(
		__global float *const vertices,
		__global unsigned int *const indices,
		__global double16 *const quadrics,
		__global double *const errorArray,
		__global unsigned int const *const independentPoints,
		__global struct arrayInfo *const vertexToIndicesPointers,
		__global unsigned int *const vertexToIndicesData,
		const unsigned int maxVertexToIndices,
		const unsigned int independentPointsNumber,
		__global unsigned int * failedAttempts
		)
{
	const size_t tid = get_global_id(0);

	if(tid >= independentPointsNumber)
	{
		//out of range
		return;
	}
	/*
	else if(tid == 0)
	{
		*failedAttempts = 0;
	}
	*/

	// get vertex
	const unsigned int vertexId = independentPoints[tid];
	const unsigned int bestNeighbor = getBestNeighbor(vertices, indices, vertexToIndicesPointers, vertexToIndicesData, quadrics, errorArray, vertexId);

	if(vertexId == UINT_MAX || bestNeighbor == UINT_MAX)
	{
#if defined(LOGGING) && defined(LOG_DECIMATE)
		log_message("bestNeighbor: UINT_MAX\n\n");
#endif
		atom_inc(failedAttempts);
		return;
	}

	const uint2 commonTriangles = getCommonTriangles(vertexToIndicesPointers, vertexToIndicesData, vertexId, bestNeighbor);

	if(commonTriangles.s0 == UINT_MAX)// || (commonTriangles.s1 == UINT_MAX) || (commonTriangles.s0 == commonTriangles.s1))
	{
#if defined(LOGGING) && defined(LOG_DECIMATE)
		log_message("decimateOnPoints failed (tid = %d)\n", tid);
		log_message("firstVertex = %d\tsecondVertex = %d\tfirstTri = %d\tsecondTri = %d\n\n", vertexId, bestNeighbor, commonTriangles.s0, commonTriangles.s1);
#endif
		atom_inc(failedAttempts);
		return; //ERROR
	}

	const float4 finalPos = findOptimalPlacement(vertices, quadrics, vertexId, bestNeighbor);

	rewireTriangles(indices, vertexToIndicesPointers, vertexToIndicesData, vertexId, bestNeighbor);
	connectLists(vertexToIndicesPointers, bestNeighbor, vertexId);
	packListElements(vertexToIndicesPointers, vertexToIndicesData, maxVertexToIndices, bestNeighbor, commonTriangles.s0, commonTriangles.s1);
	
	const uint2 topBottom = getTopBottom(indices, vertexId, bestNeighbor, commonTriangles);

	packListElements(vertexToIndicesPointers, vertexToIndicesData, maxVertexToIndices, topBottom.s0, commonTriangles.s0, commonTriangles.s1);
	if(topBottom.s1 != UINT_MAX)
	{
		//if there are two triangles around
		packListElements(vertexToIndicesPointers, vertexToIndicesData, maxVertexToIndices, topBottom.s1, commonTriangles.s0, commonTriangles.s1);
	}

	//set final placement
	{
		const double16 Q1 = quadrics[vertexId];
		const double16 Q2 = quadrics[bestNeighbor];

		const double16 Q = Q1 + Q2;
		quadrics[bestNeighbor] = Q;
		errorArray[bestNeighbor] = decimationError(finalPos, Q);

		vertices[3*bestNeighbor    ] = finalPos.x;
		vertices[3*bestNeighbor + 1] = finalPos.y;
		vertices[3*bestNeighbor + 2] = finalPos.z;
	}

	// discard up/down Triangles
	indices[commonTriangles.s0*3    ] = UINT_MAX;
	//indices[commonTriangles.s0*3 + 1] = UINT_MAX;
	//indices[commonTriangles.s0*3 + 2] = UINT_MAX;

	if(commonTriangles.s1 != UINT_MAX)
	{
		indices[commonTriangles.s1*3   ] = UINT_MAX;
		//indices[commonTriangles.s1*3 +1] = UINT_MAX;
		//indices[commonTriangles.s1*3 +2] = UINT_MAX;
	}


	// discard vertex
	vertices[3*vertexId    ] = FLT_MAX;
	//vertices[3*vertexId + 1] = FLT_MAX;
	//vertices[3*vertexId + 2] = FLT_MAX;

#if defined(LOGGING) && defined(LOG_DECIMATE)
	if(validateList(vertexToIndicesPointers, vertexToIndicesData, indices, bestNeighbor) != 0)
	{
		log_message("dop: invalid list after decimation for %d\n", bestNeighbor);
		printList(vertexToIndicesPointers, vertexToIndicesData, bestNeighbor);
	}
#endif
	
	return;

}
