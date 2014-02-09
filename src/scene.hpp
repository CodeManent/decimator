#ifndef __SCENE_HPP__
#define __SCENE_HPP__

#include <vector>

#include "object.hpp"
#include "camera.hpp"

/*
	������� ��� ������ ��� ���������� �� ����� ��� ��������������
	��� �������� ��� ���������
*/
class Scene{
public:

	Camera camera;		// � ������ ��� ���������������
	std::vector<Object *> objects; // �� ������� ��� ����������� ��� �����

	int windowWidth;	// �� ������ ��� ���������
	int windowHeight;	// �� ���� ��� ���������

	void registerObject(Object * obj); // �������� ���� ��������

	void display();		// �������� ������ ��� ��������
	void initialise();	// ������������ ������ ��� ��� �������� ���
	void printInfo();	// �������� ����������� ���� ��������

	Scene();
	~Scene();
};

extern Scene scene;

#endif // __SCENE_HPP__
