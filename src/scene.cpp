#include <iostream>
#include <typeinfo>
#include "scene.hpp"
#include <algorithm>

/**********************************************************
 Καθολική μεταβλητή που κρατά την σκηνή προς οπτικοποίηση.
**********************************************************/
Scene scene;

Scene::Scene()
{
}





/**********************************************************
 Διαγραφή των μοντέλων που περιέχει η σκηνή.
**********************************************************/
Scene::~Scene(){
	for(unsigned int i=0; i< objects.size(); ++i){
		delete objects[i];
	}
	objects.clear();
}





/**********************************************************
 Προσθήκη ενός αντικειμένου στη σκηνή
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
 Κλήση της μεθόδου αρχικοποίησης όλων των μοντέλων. 'Ετσι
 τα μοντέλα περνάνε σε buffer objects σητν κάρτα γραφικών.
**********************************************************/
void Scene::initialise(){
	for(std::vector<Object *>::iterator obj = objects.begin(); obj != objects.end(); ++obj){
		(*obj)->initialise();
	}
}





/**********************************************************
 Εμφάνιση στην κονσόλα πληροφορίες για τη σκηνή και
 τα φορτωμένα μοντέλα.
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
