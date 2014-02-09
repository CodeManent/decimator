#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include <string>
#include "point3.hpp"

/*
Ορισμός κλάσης για την κάμερα του προγράμματος.
*/
class Camera
{
	point3f position;	// Η θέση της κάμερας
	point3f lookAt;		// Το σημείο στο οποίο είναι στραμμένη
	point3f upVector;	// Η κατεύθυνση που βρίσκεται το πάνω μέρος της κάμερας

	void left(const float d); // Περιστροφή αριστερά-δεξια
	void up(const float d); // Περιστρογή πάνω-κάτω

public:
	Camera(const point3f position = point3f(0.0f, 0.0f, 5.0f), const point3f lookAt = 0.0, const point3f upVector = point3f(0.0f, 1.0f, 0.0f));
	void setTarget(const point3f target); // Ορισμός του στόχου της κάμερας

	void zoom(const float percent); // Ζουμάρισμα κατά το ποσοστό που ορίζεται
	void rotate(const float x, const float y); //Περιστροφή γύρω από το μοντέλο
	void update() const; // Ενημέρωση του πίνακα προβολής της OpenGL

	std::string toString() const;

	~Camera(void);
};

#endif
