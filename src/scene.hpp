#ifndef __SCENE_HPP__
#define __SCENE_HPP__

#include <vector>

#include "object.hpp"
#include "camera.hpp"

/*
	Ορισμός της κλάσης που αναπαριστά τη σκηνή που οπτικοποιείται
	στο παράθυρο της εφαρμογής
*/
class Scene{
public:

	Camera camera;		// Η κάμερα που χρησιμοποιείται
	std::vector<Object *> objects; // Τα μοντέλα που συμμετέχουν στη σκηνή

	int windowWidth;	// Το πλάτος του παραθύρου
	int windowHeight;	// Το ύψος του παραθύρου

	void registerObject(Object * obj); // Προσθήκη ενός μοντέλου

	void display();		// εμφάνιση σκηνής στο παράθυρο
	void initialise();	// Αρχικοποίηση σκηνής και των μοντέλων της
	void printInfo();	// Εμφάνιση πληροφοριών στην κοονσόλα

	Scene();
	~Scene();
};

extern Scene scene;

#endif // __SCENE_HPP__
