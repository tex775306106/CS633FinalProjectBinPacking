#include <iostream>  
#include<vector>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include"item.h"


class strip
{
public:
	int W, l;						//width and length
	std::vector<item> containItems;			//a list of the items the bin contains
	double svalue;     //total value of the strip
	double effvalue;   //only counts the value of items not in the pattern
	int type;			//type or id
	int m;				//total number of items (not just in this bin)
	std::vector<int> *q;				//trace
	
	strip(int length,int t, std::vector<item> itemlist){
		m = itemlist.size();
		type = t;
		if (type < m)      //width=hi,type<=m
			W = itemlist[type ].h;
		else				//width=li,m<type<=2m
			W = itemlist[type - m].l;
		l = length;
		q = new std::vector<int>();
	}

	//For a particular strip type with width W and length x,
	//    let z(i,x) be the maximum value of the strip obtained from considering the first i items
	//q is used to record how the strip is organized£¬ 0 means not in the bin, 1 means in the bin withoutrotation, 2 means in the bin with rotation
	double z(int i, int x, int W, std::vector<item> itemlist,std::vector<int> *q) {
		if (i == -1)
			return 0;
		std::vector<int> *qa = new std::vector<int>();
		std::vector<int> *qb = new std::vector<int>();
		std::vector<int> *qc = new std::vector<int>();
		double a = z(i - 1, x, W, itemlist,qa);
		double b = 0;
		double c = 0;

		if (x >= itemlist[i].l&&W >= itemlist[i].h)
			b = z(i - 1, x - itemlist[i].l, W, itemlist,qb) + itemlist[i].value;
		if (x >= itemlist[i].h&&W >= itemlist[i].l)
			c = z(i - 1, x - itemlist[i].h, W, itemlist,qc) + itemlist[i].value;
		if (a >= b&&a >= c) {

			qa->insert(qa->end(), 0);
			q->swap(*qa);
			return a;
		}
		else if (b >= a&&b >= c) {

			qb->insert(qb->end(), 1);
			q->swap(*qb);
			return b;
		}
		else if (c >= a&&c >= b){ 

			qc->insert(qc->end(), 2);
			q->swap(*qc);
			return c;
		}
		else
			return -1;
	}
	//update a strip's total value
	double updateStripValue(std::vector<item> itemlist) {
		this->svalue = 0;
		int n = q->size();
		for (int i = 0;i < n;i++) {
			if (q->at(i) > 0)
				this->svalue += itemlist[i].value;
		}
		return this->svalue;
	}
	//update a strip's total effective value, this is more practical
	double updateEffectiveValue(std::vector<item> validItemList,std::vector<item> itemlist, int *b,int *btemp) {
		this->effvalue = 0;
		int n = q->size();
		for (int i = 0;i < n;i++) {
			int tid = validItemList[i].id;
			if (b[tid]==1&&btemp[tid]==1&&this->q->at(i)>0)
				this->effvalue += itemlist[tid].value;
		}
		return this->effvalue;
	}
	~strip() {};
};
