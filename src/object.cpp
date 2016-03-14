#include <vector>
#include <algorithm>
#include <stdexcept>

#include "object.hpp"





/**********************************************************
 Constructor.
**********************************************************/
Object::Object(void)
{
}





/**********************************************************
 Destructor. 
**********************************************************/
Object::~Object(void)
{
}







/**********************************************************
 Moves the model to the origin of the axis
 
 Runs through all the vertices and computes the center of the 
 model. Then Moves the model to the origin by going again
 through each vertex and subtracting the comuted value.
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
 Finds the 'center' of the model.
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
 Adds a vertex to the model
**********************************************************/
void Object::addVertex(const point3f &vertex){
	vertices.push_back(vertex);
}





/**********************************************************
 Adds a triangle to the model
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
