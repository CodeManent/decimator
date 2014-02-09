#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_

#pragma once

#include <string>

/*
Ορισμός της δομής που θα κρατά τις παραμέτρους του
προγράμματος (που δίνονται από την γραμμή εντολών).
*/
struct Configuration
{
public:
	//program parameters
	std::string infile;			// Το αρχείο που βρίσκεται το μοντέλο
	std::string outfile;		// Το αρχείο που θα αποθηκευθεί το τελικό μοντέλο
	bool overwrite;				// Αντικατάσταση του αρχείου προορισμού αν αυτό υπάρχει

	//visual parameters
	bool ccwTriangles;			// Η φορά των τριγώνων του μοντέλου προορισμού είναι αντίθετη από αυτή των δεικτών του ρολογιού
	bool antialiasing;			// Ενεργοποίηση τηε εξομάλυνσης των ακμών στην εμφάνιση του μοντέλου

	//decimation parameters
	bool runOnCPU;				// Εκτέλεση της OpenCL στον επεξεργαστή
	std::string kernelsFile;	// Το αρχείο που βρίσκετα ο κώδικας για την OpenCL
	float decimationTarget;		// Ο στόχος που θέτουμε για την απλοποίσηση του μοντέλου (πλήθος σημείων)
	float pointsPerPassFactor;	// Το ποσοστό των αυτόνομων σημείων που θα χρησιμοποιηθούν κατά την απλοποίση ησε κάθε πέρασμα
	unsigned int independentPointsAlgorithm;	// Ο αλγόριθμος απλοποίησης που θα χρησιμοποιηθεί

	void getFromCommangLine(int & argc, char *argv[]);	// Διάβασμα των παραμέτρων
	static void showUsage();	// Εμφάνιση μηνύματος χρήσης του προγράμματος
	void print() const;			// Εμφάνιση στην κονσόλα των τιμών που έχει η δομή
	Configuration(void);
	~Configuration(void);
};

extern Configuration configuration;

#endif
