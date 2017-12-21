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

	itemlist = readItems("items.txt");

	//free pattern generation
	//computeBins(length of bin,
	//			  height of the bin,
	//            max number of cutting plans generated(if a cutting plan with the optimal number is produced, the program stops generating),
	//			  list of items,
	//            list of inflection points(trivial for guillotine cuts,
	//			  0 for free pattern generation, 1 for guillotine cuts calculation
	printf("Free pattern generating...\n");
	int num_bins=computeBins(10,10,20, itemlist,inflist, 0);
	printf("Done for free pattern generation.\n\n");
	
	//guillotine pattern calculation, takes much longer time with larger bins
	/*itemlist.clear();
	itemlist.insert(itemlist.end(), *(new item(1, 3, 0)));
	itemlist.insert(itemlist.end(), *(new item(3, 1, 1)));
	itemlist.insert(itemlist.end(), *(new item(2, 3, 2)));
	itemlist.insert(itemlist.end(), *(new item(4, 1, 3)));
	itemlist.insert(itemlist.end(), *(new item(2, 4, 4)));
	itemlist.insert(itemlist.end(), *(new item(3, 2, 5)));
	printf("Guillotine pattern calculating...\n");
	computeBins(4, 3, 2, itemlist, inflist, 1);
	printf("Done for guillotine pattern calculation.\n");*/
	getchar();
}