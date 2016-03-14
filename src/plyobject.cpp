#include "plyobject.hpp"
#include "rply.h"

#include <fstream>
#include <vector>
#include <iostream>
#include <stdexcept>

using namespace std;





PLYObject::PLYObject(void)
{
}





/**********************************************************
	Constructor that creates a model from a file
**********************************************************/
PLYObject::PLYObject(string filename)
{
	loadFromFile(filename);
}





PLYObject::~PLYObject(void)
{
}





/**********************************************************
	Callback that is used by rply to read the vertices of the model
	from the file.
	
	After we read three numbers, we add them in the
	model as a vertex.
**********************************************************/
static int vertexCallback(p_ply_argument argument){
	static unsigned int pos = 0;
	static float data[3];

	data[pos] = (float) ply_get_argument_value(argument);
	
	if(pos == 2){
		void *voidp;
		PLYObject *thisp;

		ply_get_argument_user_data(argument, &voidp, NULL);
		thisp = reinterpret_cast<PLYObject *>(voidp);
		if(thisp == NULL){
			return 0;
		}

		thisp->addVertex(point3f(data)); // may throw out of bounds exception

		pos = 0;
	}
	else{
		++pos;
	}
	
	return 1;
}





/**********************************************************
	Callback that is used by rply to read the triangles of the
	model the file.

	For each three numbers that we read, we add them in the
	model as a vertex.
	
	After we read three numbers, we add them in the
	model as a triangle.
**********************************************************/
static int faceCallback(p_ply_argument argument){
	long length, value_index;
	static unsigned int data[3], pos = 0;

	ply_get_argument_property(argument, NULL, &length, &value_index);

	if(length != 3){
		cerr << "A face with " << length << "vertices has been found" << endl;
		return 0;
	}

	if(value_index<0 || value_index > 2){
		return 1;
	}

	data[pos] = (unsigned int) ply_get_argument_value(argument);

	if(pos == 2){
		void *voidp;
		PLYObject *thisp;
		
		ply_get_argument_user_data(argument, &voidp, NULL);
		thisp = reinterpret_cast<PLYObject *>(voidp);
		if(thisp == NULL){
			return 0;
		}

		thisp->addTriangle(point3ui(data));

		pos = 0;
	}
	else{
		++pos;
	}

	return 1;
}





/**********************************************************
	Loads a model from the specified .ply file by using rply.
**********************************************************/
void PLYObject::loadFromFile(string path){
	long nvertices, ntriangles;
	p_ply ply;

    if(!(ply = ply_open(path.c_str(), NULL, 0, NULL)))
    {
		throw(runtime_error("PLYObject::loadFromFile: Couldn't open the file"));
    }
    if(!ply_read_header(ply))
    {
		ply_close(ply);
		throw(runtime_error("PLYObject::loadFromFile: Couldn't read the file's header"));
    }
	
	nvertices = ply_set_read_cb(ply, "vertex", "x", vertexCallback, this, sizeof(this));
	ply_set_read_cb(ply, "vertex", "y", vertexCallback, this, sizeof(this));
	ply_set_read_cb(ply, "vertex", "z", vertexCallback, this, sizeof(this));

	ntriangles = ply_set_read_cb(ply, "face", "vertex_indices", faceCallback, this, sizeof(this));

	this->indices.reserve(ntriangles);
	this->vertices.reserve(nvertices);

	if(!ply_read(ply)){
		ply_close(ply);
		throw(runtime_error("PLYObject::loadFromFile: Error while reading the file"));
	}

	ply_close(ply);
}





/**********************************************************
	Saves the model to the specified file.

	Parameters:
		path			The destination file.
		obj				The model that will be used
		overwrite		Overwrite the file if it already exists
		ccwTriangles	Winding of the triangles.
**********************************************************/
void PLYObject::saveToFile(string path, Object &obj, bool overwrite, bool ccwTriangles)
{
	if(!overwrite)
	{
		ifstream testfile(path.c_str());
		if(testfile.is_open())
		{
			//file already exists;
			testfile.close();
			throw runtime_error("PLYObject::saveToFile: File already exists");
		}
	}

	ofstream outfile(path.c_str(), ios::out | ios::binary);

	if(!outfile)
	{
		throw(runtime_error("PLYObject::saveToFile: Couldn't open outfile"));
	}

	outfile
		<< "ply\n"
		<< "format ascii 1.0\n"
		<< "element vertex " << obj.vertices.size() << '\n'
		<< "property float32 x\n"
		<< "property float32 y\n"
		<< "property float32 z\n"
		<< "element face " << obj.indices.size() << '\n'
		<< "property list uint8 int32 vertex_indices\n"
		<< "end_header" << endl;

	for(vector<point3f>::const_iterator v = obj.vertices.begin(); v != obj.vertices.end(); ++v)
	{
		if(!outfile)
		{
			throw(runtime_error("Outfile is in a bad state"));
		}
		outfile << v->x << " " << v->y << " " << v->z << '\n';
	}

	for(vector<point3ui>::const_iterator i = obj.indices.begin(); i != obj.indices.end(); ++i)
	{
		if(!outfile)
		{
			throw(runtime_error("Outfile is in a bad state"));
		}

		if(ccwTriangles)
		{
			outfile << "3 " << i->x << " " << i->y << " " << i->z << " " << '\n';
		}
		else
		{
			//exchange 2nd and 3rd indices of the triangle to invert its orientation
			outfile << "3 " << i->x << " " << i->z << " " << i->y << " " << '\n';
		}
	}
	outfile.close();
}
