#ifndef __DECIMATOR_HPP_
#define __DECIMATOR_HPP_

#include <string>
#include <vector>

#include "cl.hpp"

#include "object.hpp"
/*
	Δομή που χρησιμοποιείται από την OpenCL για τους δείκτες
	από τις κορυφές προς τα τρίγωνα
*/
struct arrayInfo{
	cl_uint position;
	cl_uint size;
	cl_uint continuesTo;
};

/*
	Κλαση που αναλαμβάνει την απλοποίηση του μοντέλου
*/
class Decimator{
	enum {SORT_ASCENDING = CL_TRUE, SORT_DESCENDING = CL_FALSE};

	bool runOnCPU;					// Εκτέλεση της OpenCL στον επεξεραγαστή

	std::string kernelFilename;		// Το αρχείο που βρίσκονται οι kernels
	cl::Context *context;			// To context τηε OpenCL
	cl::CommandQueue *queue;		// Η ουρά εκτέλεσης που χρησιμοποιείται.
	cl::Device device;				// Η συσκευή στην οποία εκτελείται ο κώδικας
	cl::Program *program;			// Δείκτης για το μεταγλωττισμένο πρόγραμμα για τη συσκευή που θα χρησιμοποιηθεί

	cl_int maxWorkgroupSize;		// Ο μέγιστος αριθμός workgroup που υποστηρίζει η συσκευή
	cl_uint maxComputeUnits;		// Ο αριθμός των μονάδων επεξεργασίας της συσκευής
	cl_uint maxVertexToIndices;		// Ο μέγιστος αριθμός τριγώνων που συμμετέχει μια κορυφή
	cl_ulong maxLocalSize;			// Το μέγεθος της τοπικής μνήμης που υπάρχει στη συσκευή

	cl_uint *iArray;	// Πίνακας τριγώνων για απλοποίηση στον επεξεργαστή
	cl_float *vArray;	// Πίνακας κορυφών για απλοποίηση στον επεξεργαστή

	std::vector<cl::Memory> &glBuffers; // Τα buffer objects που θα χρησιμοποιηθούν αν το μοντέλο βρίσκεται στην κάρτα γραφικών.

	cl::Memory *glIndices;			// Memory buffer (της OpenCL) για τα τρίγωνα του μοντέλου
	cl::Memory *glVertices;			// Memory buffer για τις κορυφές του μοντέλου

	cl::Buffer *triangleQuadrics;	// Memory buffer για τα "ενδιάμεσα" quadrrics των τριγώνων
	cl::Buffer *quadrics;			// Memory buffer για τα τελικά quadrics των κορυφών
	cl::Buffer *errorArray;			// Memory buffer με το σφάλμα
	cl::Buffer *usedArray;			// Memory buffer που χρησιμοποιείται κατά την εύρεση αυτόνομων κορυφών

	cl::Buffer *vertexToIndicesPointers; // Βοηθητικό memory buffer για να ξέρουμε σε ποιό σημείο του πίνακα των δεικτών πρέπει να ξεκινήσουμε για κάθε κορυφή
	cl::Buffer *vertexToIndicesData; // Memory buffe με τους δείκτες από τις κορυφές πίσω στα τρίγωνα

	double maxIndependentPointsToVertices; // Ο μέγιστος αριθμός των ανεξάρτητων κορυφών σε σχέση με το σύνολο των κορυφών
	float independentPointsPerPassFactor; // Το ποσοστό των ανεξάρτητων κορυφών που θα χρησιμοποιηθούν σε κάθε πέρασμα
    int independentPointsAlgorithm;	// Ο αλγόριθμος εύρεσης ανεξάρτητων κορυφών που θα χρησιμοποιηθεί
	cl_int pointsFound;				// Το πλήθος των ανεξάρτητων κορυφών που βρέθηκαν
	cl::Buffer *independentPoints;	// Memory buffer με τις ανεξάρτητες κορυφές
	cl::Buffer *failedAttemptsBuffer;

	//Αρχικοποίηση των Memory buffers
	cl_int initialiseBuffers		(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	//Υπολοφισμός των δεικτών από τις κορυφές τρος τα τρίγωνα
	cl_int computeVertexToIndices	(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	//Υπολογισμός των ανεξάρτητηων αημείων
	cl_int computeIndependentPoints	(const Object &obj, unsigned int remainingVertices, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Ο Αρχικός σειριακός αλγόριθμος εύρεσης ανεξάρτητηων σημείων
	cl_int computeIndependentPoints1(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Ο αλγόριθμος εύρεσης ανεξάρτητων σημείων βασισμένος στο σφάλμα
	cl_int computeIndependentPoints2(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Ο Τελικός αλγόριθμος εύρεσης ανεξάρτητων σημείων
	cl_int computeIndependentPoints3(const Object &obj, unsigned int remainingVertices, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Υπολογισμός των quadrics των τριγώνων
	cl_int computeTriangleQuadrics	(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Υπολογισμός των quadrics των κορυφών
	cl_int computeFinalQuadrics		(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Υπολογισμός του σφάλματος για κάθε κορυφή
	cl_int computeDecimationError	(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Ταξινόμηση των ανεξάρτητων κορυφών βάσει του σφάλματος που αντιστοιχεί στο καθένα
	cl_int sortDecimationError		(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Απλοποίηση του μοντέλου στα ανεξάρτητα σημεία που βρέθηκαν
	cl_int decimateOnPoints			(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0, unsigned int * const verticesToTarget = 0);

	//Μέθοδοι ελέγχου των δεδομένων (για όταν εκτελείται στον επεξεργαστή)
	cl_int validateData				(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);
	cl_int validateIndependentPoints(const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Συλλογή τελικού μοντέλου από την OpenCL
	cl_int collectResults			(Object &ret, const Object &obj, const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);
	cl_int cleanup					(					const std::vector<cl::Event> *const waitVector = 0, cl::Event *const returnedEvent = 0);

	// Έυρεση του μέγιστου αριθμού τριγώνων που συμμετέχει μια κορυφή
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
	Συνάρτηση ελέγχου της τιμής επιστροφής από τις εντολές της OpenCL
*/
void clAssert(cl_int err, const char * msg = NULL);

#define debugWait(x) clAssert(queue->finish(), "finish queue: " #x); clAssert(x.wait(), "waitting: " #x)

extern Decimator decimator;

#endif

