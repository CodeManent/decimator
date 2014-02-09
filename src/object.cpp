#include <cstdlib> //needed for glut at VS because of "error C2381: 'exit' : redefinition; __declspec(noreturn) differs"
#include <GL/glew.h> // must be before glut
#include <GL/glut.h>

#include <vector>
#include <algorithm>
#include <stdexcept>

#include "object.hpp"





/**********************************************************
 Constructor. ����������� �� buffer objects ��� ��
 ���������������.
**********************************************************/
Object::Object(void)
{
	vboID[0] = vboID[1] = 0;
}





/**********************************************************
 Destructor. ��������� �� buffer object �������������� �� �����
 ��� ������������� ���� ����� ��������.
**********************************************************/
Object::~Object(void)
{
	glDeleteBuffersARB(2, vboID);
}





/**********************************************************
 ����� �� ������� ���� OpenGL �������� �� �� buffer objects.

 ���������� ��� buffer objects (������������ ���� ��� �������
 �� ����������) ��� �� ��������� ��� ������� ��� �� ������� ���
 �������� ��� �������.
**********************************************************/
void Object::initialise()
{
	glDeleteBuffersARB(2, vboID);
	vboID[0] = vboID[1] = 0;

	// create the buffers
	glGenBuffersARB(2, vboID);

	//fill a vertices array and pass it to openGL
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboID[0]);
	GLfloat *verticesArray = new GLfloat[vertices.size() * 3];
	GLsizei position = 0;
	for(std::vector<point3f>::const_iterator vertex = vertices.begin(); vertex != vertices.end(); ++vertex)
	{
		verticesArray[position++] = vertex->x;
		verticesArray[position++] = vertex->y;
		verticesArray[position++] = vertex->z;
	}
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(GLfloat)*position, verticesArray, GL_STATIC_DRAW_ARB);
	delete verticesArray;

	//fill an indices array and pass it to openGL
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboID[1]);
	GLuint *indicesArray = new GLuint[indices.size()*3];
	position = 0;
	for(std::vector<point3ui>::const_iterator index = indices.begin(); index != indices.end(); ++index)
	{
		indicesArray[position++] = index->x;
		indicesArray[position++] = index->y;
		indicesArray[position++] = index->z;
	}

	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(GLuint)*position, indicesArray, GL_STATIC_DRAW_ARB);

	delete indicesArray;
}





/**********************************************************
 ��������� �� ������� ���� ���� ��� ������

 ��������� ���� ��� ������� ��� ���������� �� ������ ���
 ��������. ����� ��������� �� ������� ���� ���� ��� ������
 ������������ ���� ���� ��� ������� ��� ���������� ��� ���
 ����� ��� ��� ���� ��� ������������.
**********************************************************/
void Object::bring_to_center(){
	point3f center(0.0f, 0.0f, 0.0f);

	if(vertices.size() == 0){
		return;
	}

	for(std::vector<point3f>::const_iterator i = vertices.begin(); i != vertices.end(); ++i){
		center += *i;
	}

	center /= (float)vertices.size();
	
	for(std::vector<point3f>::iterator i = vertices.begin(); i != vertices.end(); ++i){
		(*i) -= center;
	}

}





/**********************************************************
 ������� �� ������ ��� ��������
**********************************************************/
point3f Object::getCenter()
{
	point3f center(0.0f, 0.0f, 0.0f);
	for(std::vector<point3f>::const_iterator i = vertices.begin(); i != vertices.end(); ++i){
		center += *i;
	}

	center /= (float)vertices.size();
	return center;
}





/**********************************************************
 ��������� �� ������� ���� �����

 ����� �� ������� �� buffer objects ���� ����� ��������
 �� ��� ������� ��� ��� �� ������������ ��� �� �� ���������.
**********************************************************/
void Object::display(){
	if(!vboID[0]){
		// the object hasn't been initialised (registered to a VBO in the graphics card)
		initialise();
	}


	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboID[0]);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	//if indices buffer object is registered
	if(vboID[1]){
		// draw the triangles

		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboID[1]);
		glDrawElements(GL_TRIANGLES, (GLsizei) indices.size()*3, GL_UNSIGNED_INT, NULL);
	}
	else{
		//else draw a "point cloud"

		glDrawArrays(GL_POINTS, 0, (GLsizei) vertices.size()*3);
	}


	glDisableClientState(GL_VERTEX_ARRAY);
}





/**********************************************************
 ��������� ��� ������ ��� �������
**********************************************************/
void Object::addVertex(const point3f &vertex){
	vertices.push_back(vertex);
}





/**********************************************************
 ��������� ��� ������� ��� �������.

 ���������� exception �� ������ ��� ���� ����� ������� ���
 �������� ���������� �� �� ��������� ������.
**********************************************************/
void Object::addTriangle(const point3ui &triangle){
	// if all three indices are smaller than the number of the vertices
	// < overloaded operator for point3ui
	if(triangle < (unsigned int) vertices.size()){
		indices.push_back(triangle);
	}
	else{
		throw(std::out_of_range("Triangle's indices out of range"));
	}
}
