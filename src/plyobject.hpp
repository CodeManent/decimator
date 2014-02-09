#ifndef __PLYOBJECT_HPP__
#define __PLYOBJECT_HPP__

#include "object.hpp"

#include <string>

/*
	Κλάση που αναπαριστά το μοντέλο που θα φορτωθεί ή θα αποθηκευθεί
	σε αρχείο με δομή .ply
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
