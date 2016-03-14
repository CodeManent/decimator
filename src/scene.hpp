#ifndef __SCENE_HPP__
#define __SCENE_HPP__

#include <vector>

#include "object.hpp"
#include "camera.hpp"

/*
	Represents the scene to be visualized in the window of the program
*/
class Scene{
public:

	Camera camera;		// The camera t be used
	std::vector<Object *> objects; // The models taking part in teh scene

	int windowWidth;	// The idth  of the window
	int windowHeight;	// The height of the window

	void registerObject(Object * obj); // Adding a model

	void printInfo();	// Dump information in the console

	Scene();
	~Scene();
};

extern Scene scene;

#endif // __SCENE_HPP__
