#include <GL/glew.h>
#include <cstdlib>
#include <GL/glut.h>
#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>

#include "plyobject.hpp"

#include "scene.hpp"
#include "decimator.hpp"
#include "glut_callbacks.hpp"
#include "configuration.hpp"

int main(int argc, char*argv[]);
void initGL();

 /**********************************************************
 Το σημείο εισόδου στο πρόγραμμα. 

 αρχικοποιεί τη glut, διαβάζει τις παραμέτρους που δίνονται,
 αρχικοποιεί τον decimator και φορτώνει το μοντέλο.
 Τέλος δίνει τον έλεγχο στο glut που αναλαμβάνει τη
 διαχείριση των γεγονότων του παραθυρου.
 **********************************************************/
int main(int argc, char* argv[]){
	try{
		initialiseGlut(&argc, argv);

		configuration.getFromCommangLine(argc, argv);

		initGL();

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

		// initialises the scene
		scene.initialise();
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
	glutMainLoop();
	return EXIT_SUCCESS;
}





 /**********************************************************
 Αρχικοποίηση της OpenGL.

 Αρχικοποίηση της OpenGL και έλεγχος για το αν υποστηρίζονται
 οι κατάλληλες επεκτάσεις της μέσω του GLEW και ορισμός
 παραμέτρων για την εμφάνιση του μοντέλου.
 **********************************************************/
void initGL(){
	std::stringstream ss;
	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		ss << "OpenGL initialisation: Error: " << glewGetErrorString(err);
		throw(std::runtime_error(ss.str()));
	}

	if(GLEW_ARB_vertex_buffer_object)
	{
		//ok
		//std::cout << "Extention supported\n";
	}
	else
	{
		throw(std::runtime_error("Error: Extension \"GL_ARB_vertex_buffer_object\" is not supported"));
	}

	//set Background color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// wireframe mode
	glPolygonMode(GL_FRONT, GL_LINE);
	//glPolygonMode(GL_BACK, GL_LINE);
	//glPolygonMode(GL_FRONT, GL_FILL);

	//correct ordering of the objects via depth test
	//glEnable(GL_DEPTH_TEST);

	// antialiasing
	if(configuration.antialiasing)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
	}

	// don't show the back faces
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	if(!configuration.ccwTriangles)
	{
		glFrontFace(GL_CW);
	}

	//glEnableClientState(GL_VERTEX_ARRAY);
}
