#include <cstdlib>
#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>

#include "plyobject.hpp"

#include "decimator.hpp"
#include "configuration.hpp"

int main(int argc, char*argv[]);
Object* go(Object *obj, const float target);
void save(Object *obj, const std::string file, const bool overwrite, const bool ccw);

/**********************************************************
 Entry point of the program.
 **********************************************************/
int main(int argc, char* argv[]){
	try{
		Configuration configuration;

		configuration.getFromCommangLine(argc, argv);
		configuration.validate();

		Decimator decimator;

		//std::clog << "Intiitialising decimator" << std::endl;
		decimator.setRunOnCPU(configuration.runOnCPU);
		decimator.setIndependentPointsPerPassFactor(configuration.pointsPerPassFactor);
		decimator.setKernelFilename(configuration.kernelsFile.c_str());
		decimator.setIndependentPointsAlgorithm(configuration.independentPointsAlgorithm);

		decimator.initialise();

		PLYObject *obj = new PLYObject (configuration.infile);

		if (configuration.decimationTarget > obj->vertices.size())
		{
			throw new std::invalid_argument("Target must be smaller then the object size.");
		}

		// Performs the decimation of the loaded object

		unsigned int computedTarget =
			configuration.decimationTarget < 1 ?
			obj->vertices.size() * configuration.decimationTarget :
			configuration.decimationTarget;

		Object *newObject = new Object();
		decimator.decimate(*obj, *newObject, computedTarget);

		// Saves the model to the specified file
		PLYObject::saveToFile(
			configuration.outfile,
			*obj,
			configuration.overwrite,
			configuration.ccwTriangles);

		std::cout << "Object saved to \"" << configuration.outfile << "\"" << std::endl;
	}
	catch(std::invalid_argument &ia)
	{
		std::cerr << ia.what() << std::endl;
		Configuration::showUsage();
		return EXIT_FAILURE;
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	//run
	return EXIT_SUCCESS;
}
