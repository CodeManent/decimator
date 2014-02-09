#include <iostream>
#include <typeinfo>
#include "scene.hpp"
#include <algorithm>

/**********************************************************
 �������� ��������� ��� ����� ��� ����� ���� ������������.
**********************************************************/
Scene scene;

Scene::Scene()
{
}





/**********************************************************
 �������� ��� �������� ��� �������� � �����.
**********************************************************/
Scene::~Scene(){
	for(unsigned int i=0; i< objects.size(); ++i){
		delete objects[i];
	}
	objects.clear();
}





/**********************************************************
 �������� ���� ������������ ��� �����
**********************************************************/
void Scene::registerObject(Object *obj){
	objects.push_back(obj);
}

void Scene::display(){
	for(std::vector<Object *>::iterator obj = objects.begin(); obj != objects.end(); ++obj){
		(*obj)->display();
	}
}





/**********************************************************
 ����� ��� ������� ������������� ���� ��� ��������. '����
 �� ������� ������� �� buffer objects ���� ����� ��������.
**********************************************************/
void Scene::initialise(){
	for(std::vector<Object *>::iterator obj = objects.begin(); obj != objects.end(); ++obj){
		(*obj)->initialise();
	}
}





/**********************************************************
 �������� ���� ������� ����������� ��� �� ����� ���
 �� ��������� �������.
**********************************************************/
void Scene::printInfo(){
	//unsigned int totalTriangles = 0, totalVertices = 0, i;
	Object::ind_size_t totalTriangles = 0;
	Object::vert_size_t totalVertices = 0;
	size_t i = 0;

	std::cout << "Camera: " <<  camera.toString() << std::endl << std::endl;


	for(i=0; i< objects.size(); ++i){
		totalTriangles += objects[i]->indices.size();
		totalVertices += objects[i]->vertices.size();

		std::cout << "Object[" << i << "]\t" << typeid(*objects[i]).name() << "\t"
			<< objects[i]->vertices.size()  << " vertices\t"
			<< objects[i]->indices.size() << " triangles"
			<< std::endl;


	}

	std::cout << std::endl << objects.size() << " objects\t" << totalVertices  << " vertices\t" << totalTriangles << " triangles" << std::endl;
}
