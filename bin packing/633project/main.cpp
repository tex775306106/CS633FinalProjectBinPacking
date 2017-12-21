#include <iostream>  
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/opencv.hpp>  
#include<vector>
#include"functions.h"
#include"inflection.h"
#include"item.h"
using namespace std;

int main() {
	
	vector<inflection> inflist;
	vector<item> itemlist;

/*	itemlist = readItems("items.txt");

	//free pattern generation

	int num_bins=computeBins(10,10,200, itemlist,inflist, 0);
	printf("Done for free pattern generation.\n");
	getchar();
	*/
	//guillotine pattern calculation
	itemlist.clear();
	itemlist.insert(itemlist.end(), *(new item(2, 2, 0)));
	itemlist.insert(itemlist.end(), *(new item(2, 1, 1)));
	computeBins(2, 2, 2, itemlist, inflist, 1);
	printf("Done for guillotine pattern calculation.\n");
	getchar();
}