#ifndef __OBJECT_HPP__
#define __OBJECT_HPP__

#include "point3.hpp"

#include <vector>

/*
	Kλάση που αναπαριστά ένα μοντέλο.
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
	std::vector<point3f> vertices;	// ΟΙ κορυφές του μοντέλου
	std::vector<point3ui> indices;	// Τα τρίγωνα του μοντέλου

	virtual void bring_to_center();	// Μεταφορά του μοντέλου στην αρχή των αξόνων
	virtual point3f getCenter();	// Παίρνουμε το "κέντρο" του μοντέλου

	virtual void addVertex(const point3f &vertex); // Προσθήκη κορυφής
	virtual void addTriangle(const point3ui &triangle); // Προσθήκη τριγώνου

	Object(void);
	virtual ~Object(void);
protected:

	friend class Decimator;
};

#endif // __OBJECT_HPP__
