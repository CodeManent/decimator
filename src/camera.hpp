#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include <string>
#include "point3.hpp"

/*
Class definition for the camera of the program
*/
class Camera
{
	point3f position;	// The position of the camera
	point3f lookAt;		// The point in space that the camera looks at.
	point3f upVector;	// The direction towards the up side of the camera

	void left(const float d); // left/right rotation
	void up(const float d); // up/down rotation

public:
	Camera(const point3f position = point3f(0.0f, 0.0f, 5.0f), const point3f lookAt = 0.0, const point3f upVector = point3f(0.0f, 1.0f, 0.0f));
	void setTarget(const point3f target); // Sets the target of the camera

	void zoom(const float percent); // Zoomint based on the specified percentile
	void rotate(const float x, const float y); // rotation around the model
	void update() const; // Updates the OpenGL projection matrix

	std::string toString() const;

	~Camera(void);
};

#endif
