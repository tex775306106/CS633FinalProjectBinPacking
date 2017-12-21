#ifndef INFLECTION_H  
#define INFLECTION_H 

#include <iostream>  
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
using namespace cv;

class inflection {
public:
	cv::Point p;
	bool candidate;
	inflection(int xcoordinate, int ycoordinate, bool can) {
		p.x = xcoordinate;
		p.y = ycoordinate;
		candidate = can;
	}
};

#endif