#include "configuration.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

using namespace std;

/**********************************************************
 Καθολική μεταβλητή που κρατά τις παραμέτρους εκτέλεσης
 του προγράμματος.
**********************************************************/
Configuration configuration;





/**********************************************************
 Constructor που βάζει τις προεπιλεγμένες τιμές στη δομή
**********************************************************/
Configuration::Configuration(void):
	infile(),
	outfile(),
	overwrite(false),
	ccwTriangles(true),
	antialiasing(false),
	runOnCPU(false),
	kernelsFile("kernels.cl"),
	decimationTarget(0.5),
	pointsPerPassFactor(0.85f),
	independentPointsAlgorithm(3)
{
}

Configuration::~Configuration(void)
{
}





/**********************************************************
Εμφάνιση των τιμών της δομής στην κονσόλα
**********************************************************/
void Configuration::print() const
{
	cout << "Configuration\n-------------" << endl;
	cout << "infile:\t\t\t" << infile << endl;
	cout << "outfile:\t\t" << outfile << endl;
	cout << "triangle orientation:\t" << (ccwTriangles ? "ccw" : "cw") << endl;
	cout << "antialiasing:\t\t" << (antialiasing ? "yes" : "no") << endl;
	cout << "run on:\t\t\t" << (runOnCPU ? "cpu" : "gpu") << endl;
	cout << "kernels file:\t\t" << kernelsFile << endl;
	cout << "target:\t\t\t" << decimationTarget << endl;
	cout << "points per pass factor:\t" << pointsPerPassFactor << endl;
	cout << "Independent points algorithm:\t" << independentPointsAlgorithm << endl;
}





/**********************************************************
Εμφάνιση στην κονσόλα το μήνυμα χρήσης του προγράμματος
**********************************************************/
void Configuration::showUsage()
{
	char usageString[] = "\n\
decimator inFile [-o outfile] [--overwrite] [--cw] [--ccw] [--antialiasing] [--gpu] [--cpu] [--kernels kernelsFile] [--target numberOfVertices] [--pointsPerPassFactor factor] [--independentPointsAlgorithm (1|2|3)] [--help]\n\
\n\
	-o				The output file\n\
	--overwrite			Overwrites the output file if exists\n\
	--ccw				Counterclockwise triangles in infile\n\
	--cw				clockwise triangles in infile\n\
	--antialiasing			Enable antialiasig in the display\n\
	--gpu				OpenCL run on GPU\n\
	--cpu				openCL runs on cpu\n\
	--kernels			Te location of the kernels file\n\
	--target			The number of vertices for the result of the decimation ( > 0 )\n\
	--pointsPerPassFactor		Percentage of the independent points that are used at every pass of the decimation algorithm\n\
	--independentPointsAlgorithm	The algorithm to be used\n\
	--help				This Message\n\
\n\
Default Values\n\
--------------\n\
run on: gpu\n\
triangle orientation: ccw\n\
kernels: kernels.cl\n\
target 0.5\n\
pointsPerPassFactor: 0.85\n\
independentPointsAlgorithm: 3\n\
";
	cout << usageString << endl;
}





/**********************************************************
 Διαβάζει τις όποιες παραμέτρους δώθηκαν από τον χρήστη στη
 γραμμή εντολών και τις ελέγχει για εγκυρότητα.
**********************************************************/
void Configuration::getFromCommangLine(int &argc, char * argv[])
{
	if(argc < 2)
	{
		throw(invalid_argument("Input file not specified"));
	}
	int pos = 1;

	if(!strcmp(argv[pos], "--help"))
	{
		throw(invalid_argument(""));
	}

	infile = argv[pos++];

	for(; pos < argc; ++pos)
	{
		if(!strcmp(argv[pos], "-o"))
		{
			if(!(pos+1 < argc))
			{
				throw(invalid_argument("Output file not specified"));
			}
			++pos;
			outfile = argv[pos];
		}
		else if(!strcmp(argv[pos], "--overwrite"))
		{
			overwrite = true;
		}
		else if(!strcmp(argv[pos], "--ccw"))
		{
			ccwTriangles = true;
		}
		else if(!strcmp(argv[pos], "--cw"))
		{
			ccwTriangles = false;
		}
		else if(!strcmp(argv[pos], "--antialiasing"))
		{
			antialiasing = true;
		}
		else if(!strcmp(argv[pos], "--gpu"))
		{
			runOnCPU = false;
		}
		else if(!strcmp(argv[pos], "--cpu"))
		{
			runOnCPU = true;
		}
		else if(!strcmp(argv[pos], "--kernels"))
		{
			if(!(pos+1 < argc))
			{
				throw(invalid_argument("Kernels file not specified"));
			}
			pos++;
			kernelsFile = argv[pos];
		}
		else if(!strcmp(argv[pos], "--target"))
		{
			if(!(pos + 1 < argc))
			{
				throw(invalid_argument("Deciamtion target not specified"));
			}
			pos++;
			decimationTarget = (float)atof(argv[pos]);
			if(decimationTarget <= 0.0f)
			{
				throw(invalid_argument("Target must be a positive munber"));
			}
			
		}
		else if(!strcmp(argv[pos], "--pointsPerPassFactor"))
		{
			if(!(pos + 1 < argc))
			{
				throw(invalid_argument("Points per pass factor not specified"));
			}
			pos++;
			pointsPerPassFactor = (float) atof(argv[pos]);
			if(pointsPerPassFactor <= 0.0f || pointsPerPassFactor > 1.0f)
			{
				throw(invalid_argument("The pointsPerPassFactor specified is out of range. Valid range is (0, 1]"));
			}
		}
		else if(!strcmp(argv[pos], "--independentPointsAlgorithm"))
		{
			if(!(pos + 1 < argc))
			{
				throw(invalid_argument("Independent points algorithm specified"));
			}
			pos++;
			independentPointsAlgorithm = atoi(argv[pos]);
			if(independentPointsAlgorithm < 1 || independentPointsAlgorithm > 3)
			{
				throw(invalid_argument("Bad independent points algorithm specified"));
			}
		}
		else if(!strcmp(argv[pos], "--help"))
		{
			throw(invalid_argument(""));
		}
/*
		else if(!strcmp(argv[pos], ""))
		{
		}
*/
		else
		{
			stringstream ss;
			ss << "Bad parameter found \"" << argv[pos] << "\"";
			throw(invalid_argument(ss.str()));
		}
	}
}
