#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_

#pragma once

#include <string>

/*
The struct that holds the parameters of the program
that are given from the command line
*/
struct Configuration
{
public:
	//program parameters
	std::string infile;			// The file of the model
	std::string outfile;		// The file where the produced model will be saves
	bool overwrite;				// Flag to replace the target file if it exists

	//visual parameters
	bool ccwTriangles;			// The kind of the trinagles of the destination model
	bool antialiasing;
	
	//decimation parameters
	bool runOnCPU;
	std::string kernelsFile;
	float decimationTarget;		// In number of vertices
	float pointsPerPassFactor;	// The percent of the independent points that will be used on each pass
	unsigned int independentPointsAlgorithm;	// The simplificaion algorithm that will be used

	void getFromCommangLine(int & argc, char *argv[]);	// Gets the parametera
	static void showUsage();	// Prints on the console a description of how to use the program
	void print() const;			// Prints on the console the values of the structure
	Configuration(void);
	~Configuration(void);
};

extern Configuration configuration;

#endif
