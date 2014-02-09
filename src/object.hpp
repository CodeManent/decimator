#ifndef __OBJECT_HPP__
#define __OBJECT_HPP__

#include "point3.hpp"

#include <vector>

/*
	K���� ��� ���������� ��� �������.
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
	std::vector<point3f> vertices;	// �� ������� ��� ��������
	std::vector<point3ui> indices;	// �� ������� ��� ��������

	virtual void initialise();		// "�����������" ��� ��������. ����� �� buffer object ���� ����� ��������
	virtual void bring_to_center();	// �������� ��� �������� ���� ���� ��� ������
	virtual point3f getCenter();	// ��������� �� "������" ��� ��������
	virtual void display();			// ����������� ������� ���� �����

	virtual void addVertex(const point3f &vertex); // �������� �������
	virtual void addTriangle(const point3ui &triangle); // �������� ��������

	Object(void);
	virtual ~Object(void);
protected:
	unsigned int vboID[2];

	friend class Decimator;
};

#endif // __OBJECT_HPP__
