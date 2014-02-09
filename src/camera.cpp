#include <cstdlib>
#include <GL/glut.h>
#include <sstream>

#include "camera.hpp"
#include "scene.hpp"

Camera::Camera(const point3f position, const point3f lookAt, const point3f upVector):
	position(position),
	lookAt(lookAt),
	upVector(upVector)
{
}

Camera::~Camera(void){
}





/**********************************************************
	Ορίζει τον στόχο της κάμερας
**********************************************************/
void Camera::setTarget(const point3f target)
{
	this->lookAt = target;
}





/**********************************************************
	Μειώνει η αυξάνει την απόσταση της κάμερας από το σημείο
	στόχου ανάλογα με το ποσοστό που δίνεται.
**********************************************************/
void Camera::zoom(const float percent){
	// bring to center
	point3f p = position - lookAt;

	// change length
	p = p + p*percent;

	if(p.length() < 0.001f){
		return;
	}

	// get it back
	position = p + lookAt;
}





/**********************************************************
	Περιστρέφει την κάμερα πρώτα στον οριζόντιο και ύστερα
	στον κατακόρυφο άξονα
**********************************************************/
void Camera::rotate(const float x, const float y){
	point3f d(x, y);
	d = d*position.distance(lookAt)*5.0f;

	left(d.x);
	up(d.y);
}





/**********************************************************
	Περιστρέφει την κάμερα στον οριζόντιο άξονα γύρω από το
	σημείο που στοχεύει διατηρώντας την απόσταση από αυτό.

	Αρχικά γίνεται μετακίνηση του διανύσματος ώστε το σημείο
	στόχου να σημπίπτει με την αρχή των αξόνων. Εκτελεί την
	περιστροφή διατηρώντας την απόσταση από την αρχή των
	αξόνων και τέλος με μια αντίστροφη μετακίνηση από την αρχική
	τοποθετείται στην τελική του θέση.
**********************************************************/
void Camera::left(const float d){
	point3f p = position - lookAt;
	const float length = p.length();
	point3f left = p.cross(upVector);

	left.normalise();
	left = left*d;
	p += left;
	p.normalise();
	p=p*length;
	position = p + lookAt;

}





/**********************************************************
	Περιστρέφει την κάμερα στον κατακόρυφο άξονα γύρω από το
	σημείο που στοχεύει διατηρώντας την απόσταση από αυτό.

	Αρχικά γίνεται μετακίνηση του διανύσματος ώστε το σημείο
	στόχου να σημπίπτει με την αρχή των αξόνων. Εκτελεί την
	περιστροφή διατηρώντας την απόσταση από την αρχή των
	αξόνων και τέλος με μια αντίστροφη μετακίνηση από την αρχική
	τοποθετείται στην τελική του θέση.
**********************************************************/
void Camera::up(const float d){
	point3f p = position - lookAt;
	const float length = p.length();
	point3f left = p.cross(upVector);

	upVector.normalise();
	upVector = upVector * d;
	p += upVector;
	p.normalise();
	p = p*length;
	position = p+lookAt;
	upVector = left.cross(p).normalise();
}





/**********************************************************
	Ενημερώνει τον πίνακα προβολής (projection matrix) της
	OpenGL.
**********************************************************/
void Camera::update() const{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (GLdouble)scene.windowWidth/scene.windowHeight, 0.01, 10000);
	gluLookAt(
		position.x, position.y, position.z, //camera position
		lookAt.x, lookAt.y, lookAt.z, //look at
		//0.0f, 1.0f, 0.0f
		upVector.x, upVector.y, upVector.z
		);

	glMatrixMode(GL_MODELVIEW);
}





/**********************************************************
	Επιστρέφει ένα string με τις τρέχουσες τιμές των μεταβλητών
	που υπάρχουν στο αντικείμενο της κάμερας.
**********************************************************/
std::string Camera::toString() const{
	std::stringstream ss;

	ss << "position = (" << position.x << ", " << position.y << ", " << position.z << ") "
		<< "lookAt = (" << lookAt.x << ", " << lookAt.y << ", " << lookAt.z << ") "
		<< "upVector = (" << upVector.x << ", " << upVector.y << ", " << upVector.z << ")"
		<< std::ends;

	return ss.str();
}
