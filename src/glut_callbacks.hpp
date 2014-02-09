#ifndef __GLUT_CALLBACKS_HPP_
#define __GLUT_CALLBACKS_HPP_

#include <cstdlib>
#include <GL/glut.h>

void initialiseGlut(int *argcp, char **argv); //������������ ��� GOUT

void displayCallback(void); // �������� ��� �������� ��� ��������

void reshapeCallback(int width, int height); //��������� ��� ������� ���������� ��� ���������

void keyboardCallback(unsigned char key, int x, int y); //��������� ��� �������������

void mouseCallback(int button, int state, int x, int y); // ��������� ��������� �������� ��� ���������
void motionCallback(int x, int y); // ��������� ����������� ��� ���������



#endif // __GLUT_CALLBACKS_HPP_
