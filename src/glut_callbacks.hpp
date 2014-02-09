#ifndef __GLUT_CALLBACKS_HPP_
#define __GLUT_CALLBACKS_HPP_

#include <cstdlib>
#include <GL/glut.h>

void initialiseGlut(int *argcp, char **argv); //Αρχικοποίηση της GOUT

void displayCallback(void); // Εμφάνιση του μοντέλου στο παράθυρο

void reshapeCallback(int width, int height); //Χειρισμός της αλλαγής διαστάσεων του παραθύρου

void keyboardCallback(unsigned char key, int x, int y); //Χειριεμός του πληκτρολογίου

void mouseCallback(int button, int state, int x, int y); // Χειρισμός πατήματος κουμπιού του ποντικιού
void motionCallback(int x, int y); // Χειρισμός μετακίνησης του ποντικιού



#endif // __GLUT_CALLBACKS_HPP_
