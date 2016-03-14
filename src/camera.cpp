#include <cstdlib>
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
	Sets the target of the camera
**********************************************************/
void Camera::setTarget(const point3f target)
{
	this->lookAt = target;
}





/**********************************************************
	Reduces/increases the distance of the camera from the
	target based on the given percentile 
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





/**********************************************************.
	Rotates the camera around the horizontal ant then the 
	vertical axis
**********************************************************/
void Camera::rotate(const float x, const float y){
	point3f d(x, y);
	d = d*position.distance(lookAt)*5.0f;

	left(d.x);
	up(d.y);
}





/**********************************************************
	Rotates the camera around the horizontal axis preserving the
	distance to the target
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
	Rotates the camera around the vertical axis preserving the
	distance to the target.
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
	Updateh teh projection matrix
**********************************************************/
void Camera::update() const{
	// code removed, to clear OpenGL dependency
}





/**********************************************************
	Returns a string description of the object.
**********************************************************/
std::string Camera::toString() const{
	std::stringstream ss;

	ss << "position = (" << position.x << ", " << position.y << ", " << position.z << ") "
		<< "lookAt = (" << lookAt.x << ", " << lookAt.y << ", " << lookAt.z << ") "
		<< "upVector = (" << upVector.x << ", " << upVector.y << ", " << upVector.z << ")"
		<< std::ends;

	return ss.str();
}
