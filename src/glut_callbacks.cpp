#include <iostream>
#include <math.h>

#include "glut_callbacks.hpp"
#include "scene.hpp"
#include "decimator.hpp"
#include "plyobject.hpp"
#include "configuration.hpp"


/**********************************************************
	Η θέση (x, y) στην οποία ξεκινά ή μετακίνηση με πατημένο
	το κουμπί του ποντικιού
**********************************************************/
point3f startPosition = 0.0f;

/**********************************************************
	Ορίζει αν η μετακίνηση που εκτελείται χρησιμοποιείται για ζουμάρισμα
**********************************************************/
bool zooming = false;





/**********************************************************
	Αρχικοποίηση του GLUT

	Ορίζονται οι παράμετροι της OpenGL, δημιουργείται το
	παράθυρο του προγράμματος και ορίζονται οι συναρτήσεις
	που θα χειρίζονται τα γεγονότα που έρχονται από το
	παραθυρικό περιβάλλον.
**********************************************************/
void initialiseGlut(int *argcp, char **argv)
{
	//initialise glut
	glutInit(argcp, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1024, 768);

	//create the window
	glutCreateWindow("OpenGL viewer");

	// register callbacks

	// display & resize
	glutDisplayFunc(displayCallback);
	glutReshapeFunc(reshapeCallback);

	// keyboard
	glutKeyboardFunc(keyboardCallback);


	// mouse
	glutMouseFunc(mouseCallback);
	glutMotionFunc(motionCallback);
}





/**********************************************************
	Εμφανίζει τη σκηνή στο παράθυρο του προγράμματος.

	Αρχικά σχεδιάζει τους άξονες στην αρχή των συντεταγμένων
	και μετά εμφανίζει το μοντέλο.
**********************************************************/
void  displayCallback(void){
	static GLenum glerr;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw axes
	glBegin(GL_LINES);
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2i(-5, 0);
		glVertex2i(5, 0);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2i(0, -5);
		glVertex2i(0, 5);

		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3i(0, 0, -5);
		glVertex3i(0, 0, 5);
	glEnd();

	glColor3f(.7f, .7f, .7f);
	glLoadIdentity();
	//glScalef(100.0f, 100.0f, 100.0f);
	scene.display();

	glutSwapBuffers();

	if((glerr = glGetError()) != GL_NO_ERROR){

#define ERRCASE(x) {if(glerr==(x)) {std::cerr << "OpenGL Error: " << (#x) << std::endl;}}

		ERRCASE(GL_NO_ERROR);			// 0
		ERRCASE(GL_INVALID_ENUM);		// 0x500
		ERRCASE(GL_INVALID_VALUE);		// 0x501
		ERRCASE(GL_INVALID_OPERATION);	// 0x502
		ERRCASE(GL_STACK_OVERFLOW);		// 0x503
		ERRCASE(GL_STACK_UNDERFLOW);	// 0x504
		ERRCASE(GL_OUT_OF_MEMORY);		// 0x505
#undef ERRCASE
	}
}





/**********************************************************
	Χειρισμός της αλλαγής διαστάσεων του παραθύρου

	Θέτει τον χώρο που θα εμφανίζονται τα περιεχόμενα από την
	OpenGL να καλύπτουν όλο το παράθυρο και ενημερώνει την
	κάμερα.

	Παράμετροι (δίνονται από το GLUT)
		width	Το νέο πλάτος του παραθύρου
		height	Το νέο ύψος του παραθύρου
**********************************************************/
void  reshapeCallback(int width, int height){
	scene.windowWidth = width;
	scene.windowHeight = height;

	glViewport(0, 0, width, height);
	scene.camera.update();
}





/**********************************************************
	Χειρίζεται το πάτημα ενός κουμιού στο πληκτρολόγιο

	Παράμετροι (δίνονται από το GLUT)
		key		Το κουμί που πατιέται στο πληκτρολόγιο.
		x, y	Οι συντεταγμένες του ποντικιού (αγνοούνται)
**********************************************************/
void  keyboardCallback(unsigned char key, int /*x*/, int /*y*/){
	const char *uiUsageString="\
UI usage\n\
========\n\
h - UI usage help\n\
a - Post redisplay\n\
i - Scene info\n\
d - Decimate loaded model\n\
s - Save model\n\
c - Invert triangle's front face\n\
\n\
Drag with mouse - move camera\n\
Ctrl + drag witn mouse - zoom";

	switch(key){
		case 'h':
		case 'H':
			std::cout << uiUsageString << std::endl;
			break;

		case 'a':
		case 'A':
			glutPostRedisplay();
			break;

		case 'i':
		case 'I':
			scene.printInfo();
			break;

		case 'd':
		case 'D':
		{
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

			glutPostRedisplay();
			break;
		}

		case 's':
		case 'S':
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
			break;

		case 'c':
		case 'C':
			configuration.ccwTriangles = !configuration.ccwTriangles;
			glFrontFace(configuration.ccwTriangles ? GL_CCW : GL_CW);
			glutPostRedisplay();
			break;

		default:
			break;
	}
}





/**********************************************************
	Χειρίζεται τα κουμπιά του ποντικιού

	Παράμετροι (δίνονται από το GLUT)
		button	Το κουμπί που πατήθηκε
		x, y	Οι συντεταγμένες του ποντικιού

	Όταν πατιέται το αριστερό κουμπί καταχωρείται η θέση του
	ποντικιού σαν θέση εκκίνησης της της μετακίνησης και όταν
	αφήνουμε το κουμπί πενεργοποιείται η μετακίνηση.

	’ν στην την αρχή της μετακίνησης είναι πατημένο το [Ctrl]
	τότε ορίζεται ότι η μετακίνηση θα είναι ζουμάρισμα.
**********************************************************/
void  mouseCallback(int button, int state, int x, int y){
	if(button == GLUT_LEFT){
		if(state == GLUT_DOWN){
			startPosition = point3f(x, y, 0);

			if(glutGetModifiers() & GLUT_ACTIVE_CTRL){
				zooming = true;
			}
			else{
				zooming = false;
			}

		}
		else{
			startPosition = 0.0f;

		}
	}

}





/**********************************************************
	Χειρίζεται την μετακίνηση του ποντικιού.

	Υπολογίζεται η απόσταση από το αρχικό σημείο μέχρι το τρέχον
	σημείο που βρίσκεται ο δείκτης του ποντικιού και με βάση
	αυτήν περιστρέφει την κάμερα ή τροποποιεί την απόστασή της.

	Παράμετροι (δίνονται από το GLUT)
		x, y	Οι συντεταγμένες του ποντικιού
**********************************************************/
void  motionCallback(int x, int y){
	if(!(startPosition == 0.0f)){
		point3f newPosition(x, y, 0);
		point3f delta = (newPosition - startPosition)/point3f(scene.windowWidth, scene.windowHeight, 1.0f);

		startPosition = newPosition;

		if(zooming){
			//zoom while ctrl is pressed
			delta = delta * -10.0f;
			scene.camera.zoom(delta.y);
		}
		else{
			// rotate if not
			scene.camera.rotate(delta.x, delta.y);
		}

		scene.camera.update();

		glutPostRedisplay();
	}
}
