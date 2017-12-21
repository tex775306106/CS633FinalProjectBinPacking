#ifndef ITEM_H  
#define ITEM_H 
class item {
public:
	int l, h;  //height and length of the item
	double value = l*h;   //the value of the item, initially it is the size of it
	bool rotated = false;    //Be true if the item is rotated
	int r = 0;			//color of the edges of this traingle
	int g =0;
	int bb = 1;
	int id;
	item(){}
	item(int length, int height,int num) {
		l = length;
		h = height;
		value = l*h;
		r = (rand() % 256);
		g = (rand() % 256);
		bb = (rand() % 256);
		id = num;
	}
};

#endif