#include <iostream>
#include <typeinfo>
#include "scene.hpp"
#include <algorithm>

/**********************************************************
 Global var that holds the scene to be visualized
**********************************************************/
Scene scene;

Scene::Scene()
{
}





/**********************************************************
 Desctructor, delets the models in the scene
**********************************************************/
Scene::~Scene(){
	for(unsigned int i=0; i< objects.size(); ++i){
		delete objects[i];
	}
	objects.clear();
}





/**********************************************************
 Adds an object in the scene
**********************************************************/
void Scene::registerObject(Object *obj){
	objects.push_back(obj);
}




/**********************************************************
 Shows information about the scene and the loaded models
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
