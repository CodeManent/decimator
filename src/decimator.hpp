#ifndef __DECIMATOR_HPP_
#define __DECIMATOR_HPP_

#include <string>
#include <vector>

#include "cl.hpp"

#include "object.hpp"
/*
	���� ��� ��������������� ��� ��� OpenCL ��� ���� �������
	��� ��� ������� ���� �� �������
*/
struct arrayInfo{
	cl_uint position;
	cl_uint size;
	cl_uint continuesTo;
};

/*
	����� ��� ����������� ��� ���������� ��� ��������
*/
class Decimator{
	enum {SORT_ASCENDING = CL_TRUE, SORT_DESCENDING = CL_FALSE};

	bool runOnCPU;					// �������� ��� OpenCL ���� ������������

	std::string kernelFilename;		// �� ������ ��� ���������� �� kernels
	cl::Context *context;			// To context ��� OpenCL
	cl::CommandQueue *queue;		// � ���� ��������� ��� ���������������.
	cl::Device device;				// � ������� ���� ����� ���������� � �������
	cl::Program *program;			// ������� ��� �� ��������������� ��������� ��� �� ������� ��� �� ��������������

	cl_int maxWorkgroupSize;		// � �������� ������� workgroup ��� ����������� � �������
	cl_uint maxComputeUnits;		// � ������� ��� ������� ������������ ��� ��������
	cl_uint maxVertexToIndices;		// � �������� ������� �������� ��� ���������� ��� ������
	cl_ulong maxLocalSize;			// �� ������� ��� ������� ������ ��� ������� ��� �������

	cl_uint *iArray;	// ������� �������� ��� ���������� ���� �����������
	cl_float *vArray;	// ������� ������� ��� ���������� ���� �����������

	std::vector<cl::Memory> &glBuffers; // �� buffer objects ��� �� ��������������� �� �� ������� ��������� ���� ����� ��������.

	cl::Memory *glIndices;			// Memory buffer (��� OpenCL) ��� �� ������� ��� ��������
	cl::Memory *glVertices;			// Memory buffer ��� ��� ������� ��� ��������

	cl::Buffer *triangleQuadrics;	// Memory buffer ��� �� "���������" quadrrics ��� ��������
	cl::Buffer *quadrics;			// Memory buffer ��� �� ������ quadrics ��� �������
	cl::Buffer *errorArray;			// Memory buffer �� �� ������
	cl::Buffer *usedArray;			// Memory buffer ��� ��������������� ���� ��� ������ ��������� �������

	cl::Buffer *vertexToIndicesPointers; // ��������� memory buffer ��� �� ������� �� ���� ������ ��� ������ ��� ������� ������ �� ����������� ��� ���� ������
	cl::Buffer *vertexToIndicesData; // Memory buffe �� ���� ������� ��� ��� ������� ���� ��� �������

	double maxIndependentPointsToVertices; // � �������� ������� ��� ����������� ������� �� ����� �� �� ������ ��� �������
	float independentPointsPerPassFactor; // �� ������� ��� ����������� ������� ��� �� ��������������� �� ���� �������
    int independentPointsAlgorithm;	// � ���������� ������� ����������� ������� ��� �� ��������������
	cl_int pointsFound;				// �� ������ ��� ����������� ������� ��� ��������
	cl::Buffer *independentPoints;	// Memory buffer �� ��� ����������� �������

	//������������ ��� Memory buffers
	cl_int initialiseBuffers		(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	//����������� ��� ������� ��� ��� ������� ���� �� �������
	cl_int computeVertexToIndices	(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	//����������� ��� ������������ �������
	cl_int computeIndependentPoints	(const Object &obj, unsigned int remainingVertices, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// � ������� ��������� ���������� ������� ������������ �������
	cl_int computeIndependentPoints1(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// � ���������� ������� ����������� ������� ���������� ��� ������
	cl_int computeIndependentPoints2(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// � ������� ���������� ������� ����������� �������
	cl_int computeIndependentPoints3(const Object &obj, unsigned int remainingVertices, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// ����������� ��� quadrics ��� ��������
	cl_int computeTriangleQuadrics	(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// ����������� ��� quadrics ��� �������
	cl_int computeFinalQuadrics		(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// ����������� ��� ��������� ��� ���� ������
	cl_int computeDecimationError	(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// ���������� ��� ����������� ������� ����� ��� ��������� ��� ����������� ��� ������
	cl_int sortDecimationError		(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// ���������� ��� �������� ��� ���������� ������ ��� ��������
	cl_int decimateOnPoints			(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0, unsigned int * const verticesToTarget = 0);

	//������� ������� ��� ��������� (��� ���� ���������� ���� �����������)
	cl_int validateData				(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);
	cl_int validateIndependentPoints(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// ������� ������� �������� ��� ��� OpenCL
	cl_int collectResults			(Object &ret, const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);
	cl_int cleanup					(					const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// ������ ��� �������� ������� �������� ��� ���������� ��� ������
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
};

/*
	��������� ������� ��� ����� ���������� ��� ��� ������� ��� OpenCL
*/
void clAssert(cl_int err, const char * msg = NULL);

#define debugWait(x) //clAssert(queue->finish(), "finish queue: " #x); clAssert(x.wait(), "waitting: " #x)

extern Decimator decimator;

#endif

