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
 Μεταφέρει το μοντέλο στην αρχή των αξόνων

 Διατρέχει όλες τις κορυφές και υπολογίζει το κέντρο του
 μοντέλου. Τέλος μεταφέρει το μοντέλο στην αρχή των αξόνων
 διατρέχοντας πάλι όλες τις κορυφές και αφαιρώντας από την
 κκάθε μια την τιμή που υπολογίσθηκε.
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
 Βρίσκει το κέντρο του μοντέλου
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
 Προσθέτει μια κορυφή στο μοντέλο
**********************************************************/
void Object::addVertex(const point3f &vertex){
	vertices.push_back(vertex);
}





/**********************************************************
 Προσθέτει ένα τριγωνο στο μοντέλο.

 Δημιουργεί exception αν κάποιο από τους τρείς δείκτες του
 τριγώνου αναφέρεται σε μη υπάρχουσα κορυφή.
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
