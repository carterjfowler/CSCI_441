#include <iostream>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

int main () {
	//I am reviewing my knowledge of C++, but this is my best attempt at a solution for this

	string fileName;
	cout << "\nPlease enter the file name:";
	cin >> fileName;

	ifstream input;
	input.open(fileName);

	vector<vector<double>> points;

	if (input.fail()) {
		cerr << "Error opening the input file" << endl;
		exit(1);
	}

	int pointsSize = 0;
	input.get(pointsSize);

	string tempPoint;
	vector<string> tempVec;
	vector<double> tempPointVec;
	while ( !input.eof()) {
		input.get(tempPoint);
		tempVec = tempPoint.split(',');
		tempPointVec = null;
		tempPointVec.add(tempVec[0]);
		tempPointVec.add(tempVec[1]);
		points.add(tempPointVec);
	}

	input.close();

	int x = 0;
	int y = 0;
	for (int i = 0; i < pointsSize; ++i) {
		x += points.get(i).get(0);
		y += points.get(i).get(1);
	}

	int midpointX = x/pointsSize;
	int midpointY = y/pointsSize;

	cout << "\nThe midpoint of the inputed points is (" + midpointX + ", " + midpointY + ")";

	return 0;
}