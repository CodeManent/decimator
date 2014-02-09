#include <cstdlib>
#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>

#include "plyobject.hpp"

#include "scene.hpp"
#include "decimator.hpp"
#include "configuration.hpp"

int main(int argc, char*argv[]);

 /**********************************************************
 Το σημείο εισόδου στο πρόγραμμα. 

 αρχικοποιεί τη glut, διαβάζει τις παραμέτρους που δίνονται,
 αρχικοποιεί τον decimator και φορτώνει το μοντέλο.
 Τέλος δίνει τον έλεγχο στο glut που αναλαμβάνει τη
 διαχείριση των γεγονότων του παραθυρου.
 **********************************************************/
int main(int argc, char* argv[]){
	try{
		configuration.getFromCommangLine(argc, argv);

		//std::clog << "Intiitialising decimator" << std::endl;
		decimator.setRunOnCPU(configuration.runOnCPU);
		decimator.setIndependentPointsPerPassFactor(configuration.pointsPerPassFactor);
		decimator.setKernelFilename(configuration.kernelsFile.c_str());
		decimator.setIndependentPointsAlgorithm(configuration.independentPointsAlgorithm);

		decimator.initialise();

		std::clog << "Trying to load \"" <<configuration.infile << "\"" << std::endl;
		PLYObject *obj = new PLYObject (configuration.infile);

        scene.camera.setTarget(obj->getCenter());
		scene.registerObject(obj);
	}
	catch(std::invalid_argument ia)
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
void go(){
	Object *newObject = new Object();
	try{
		if(configuration.decimationTarget < 1.0f)
		{
			decimator.decimate(*scene.objects[0], *newObject, (unsigned int)(scene.objects[0]->vertices.size()*configuration.decimationTarget));
		}
		else
		{
			if(configuration.decimationTarget < scene.objects[0]->vertices.size())
			{
				decimator.decimate(*scene.objects[0], *newObject, (unsigned int)configuration.decimationTarget);
			}
			else
			{
				std::cout << "Target must be smaller then the object size" << std::endl;
				return;
			}
		}
	}
	catch(std::exception &e)
	{
		delete newObject;
		std::cout << e.what() << std::endl;
		return;
	}

	delete scene.objects[0];
	scene.objects.pop_back();
	scene.objects.push_back(newObject);
}

/******************************************************************************
 Saves the model to the specified file
******************************************************************************/

void save(){
	if(configuration.outfile.size() != 0)
	{
		try{
			PLYObject::saveToFile(configuration.outfile.c_str(), *(scene.objects[0]),configuration.overwrite, configuration.ccwTriangles);
			std::cout << "Object saved to \""<< configuration.outfile <<"\"" << std::endl;
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}
	}
	else
	{
		std::cout << "Outfile not specified" << std::endl;
	}
}

