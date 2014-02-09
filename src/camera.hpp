#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include <string>
#include "point3.hpp"

/*
������� ������ ��� ��� ������ ��� ������������.
*/
class Camera
{
	point3f position;	// � ���� ��� �������
	point3f lookAt;		// �� ������ ��� ����� ����� ���������
	point3f upVector;	// � ���������� ��� ��������� �� ���� ����� ��� �������

	void left(const float d); // ���������� ��������-�����
	void up(const float d); // ���������� ����-����

public:
	Camera(const point3f position = point3f(0.0f, 0.0f, 5.0f), const point3f lookAt = 0.0, const point3f upVector = point3f(0.0f, 1.0f, 0.0f));
	void setTarget(const point3f target); // ������� ��� ������ ��� �������

	void zoom(const float percent); // ���������� ���� �� ������� ��� ��������
	void rotate(const float x, const float y); //���������� ���� ��� �� �������
	void update() const; // ��������� ��� ������ �������� ��� OpenGL

	std::string toString() const;

	~Camera(void);
};

#endif
