#ifndef __OBJECT_HPP__
#define __OBJECT_HPP__

#include "point3.hpp"

#include <vector>

/*
	Class representing a model
*/
class Object
{
public:

	typedef std::vector<point3f>::size_type vert_size_t;
	typedef std::vector<point3ui>::size_type ind_size_t;

/*
	typedef unsigned int vert_size_t;
	typedef unsigned int ind_size_t;
*/
	std::vector<point3f> vertices;	// The vertices of the model
	std::vector<point3ui> indices;	// The triangles ofthe model

	virtual void bring_to_center();	// Moves the model to the origin
	virtual point3f getCenter();	// Calculates the "center" of the model

	virtual void addVertex(const point3f &vertex); // Adds a vertex
	virtual void addTriangle(const point3ui &triangle); // Adds a triangle

	Object(void);
	virtual ~Object(void);
protected:

	friend class Decimator;
};

#endif // __OBJECT_HPP__
