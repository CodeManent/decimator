#ifndef __PLYOBJECT_HPP__
#define __PLYOBJECT_HPP__

#include "object.hpp"

#include <string>

/*
	Class representing the model that will be loaded/saved in a .ply file
*/
class PLYObject: public Object{
public:
	PLYObject(void);
	PLYObject(std::string filename);
	virtual ~PLYObject(void);

	void loadFromFile(std::string path);
	static void saveToFile(std::string path, Object &obj, bool overwrite = false, bool ccwTriangles = true);
};

#endif
