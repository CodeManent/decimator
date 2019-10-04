#include <cstdlib>
#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>

#include "plyobject.hpp"

#include "decimator.hpp"
#include "configuration.hpp"

int main(int argc, char*argv[]);
Object* go(Object *obj);
void save(Object *obj);

/**********************************************************
 Entry point of the program.
 **********************************************************/
int main(int argc, char* argv[]){
	try{
		configuration.getFromCommangLine(argc, argv);

		if(configuration.outfile == "") {
			throw std::invalid_argument("No outfile is specified");
		}

		//std::clog << "Intiitialising decimator" << std::endl;
		decimator.setRunOnCPU(configuration.runOnCPU);
		decimator.setIndependentPointsPerPassFactor(configuration.pointsPerPassFactor);
		decimator.setKernelFilename(configuration.kernelsFile.c_str());
		decimator.setIndependentPointsAlgorithm(configuration.independentPointsAlgorithm);

		decimator.initialise();

		PLYObject *obj = new PLYObject (configuration.infile);

		go(obj);

		// and save it if appropriate argument is specified
		save(obj);
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



/******************************************************************************
 Performs the decimation of the loaded object
******************************************************************************/
Object* go(Object *obj)
{
	Object *newObject = new Object();
	try{
		if(configuration.decimationTarget < 1.0f)
		{
			decimator.decimate(*obj, *newObject, (unsigned int)(obj->vertices.size()*configuration.decimationTarget));
		}
		else
		{
			if(configuration.decimationTarget < obj->vertices.size())
			{
				decimator.decimate(*obj, *newObject, (unsigned int)configuration.decimationTarget);
			}
			else
			{
				std::cout << "Target must be smaller then the object size" << std::endl;
				return nullptr;
			}
		}
	}
	catch(std::exception &e)
	{
		delete newObject;
		std::cout << e.what() << std::endl;
		return nullptr;
	}

	return newObject;
}

/******************************************************************************
 Saves the model to the specified file
******************************************************************************/

void save(Object *obj){
	PLYObject::saveToFile(configuration.outfile.c_str(), *obj, configuration.overwrite, configuration.ccwTriangles);
	std::cout << "Object saved to \""<< configuration.outfile <<"\"" << std::endl;
}

