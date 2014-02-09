#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_

#pragma once

#include <string>

/*
������� ��� ����� ��� �� ����� ��� ����������� ���
������������ (��� �������� ��� ��� ������ �������).
*/
struct Configuration
{
public:
	//program parameters
	std::string infile;			// �� ������ ��� ��������� �� �������
	std::string outfile;		// �� ������ ��� �� ����������� �� ������ �������
	bool overwrite;				// ������������� ��� ������� ���������� �� ���� �������

	//visual parameters
	bool ccwTriangles;			// � ���� ��� �������� ��� �������� ���������� ����� �������� ��� ���� ��� ������� ��� ��������
	bool antialiasing;			// ������������ ��� ����������� ��� ����� ���� �������� ��� ��������

	//decimation parameters
	bool runOnCPU;				// �������� ��� OpenCL ���� �����������
	std::string kernelsFile;	// �� ������ ��� �������� � ������� ��� ��� OpenCL
	float decimationTarget;		// � ������ ��� ������� ��� ��� ����������� ��� �������� (������ �������)
	float pointsPerPassFactor;	// �� ������� ��� ��������� ������� ��� �� ��������������� ���� ��� ��������� ��� ���� �������
	unsigned int independentPointsAlgorithm;	// � ���������� ����������� ��� �� ��������������

	void getFromCommangLine(int & argc, char *argv[]);	// �������� ��� ����������
	static void showUsage();	// �������� ��������� ������ ��� ������������
	void print() const;			// �������� ���� ������� ��� ����� ��� ���� � ����
	Configuration(void);
	~Configuration(void);
};

extern Configuration configuration;

#endif
