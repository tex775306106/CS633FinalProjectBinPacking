#include<vector>
#include<math.h>
#include<string> 
#include<sstream>
#include<fstream> 
#include<cstdlib>
#include<algorithm>
#include"item.h"
#include"strip.h"
#include"inflection.h"

//items with higher value are lists at the front
bool comp(const item &a, const item &b) {
	return a.value > b.value;
}
//read items and sort them in non-increasing order
std::vector<item> readItems(std::string filename) {
	std::ifstream file(filename);
	std::string in;
	file >> in;
	int m = atoi(in.c_str());
	std::vector<item> itemlist;
	for (int i = 0;i <m;i++) {
		file >> in;
		int l = atoi(in.c_str());
		file >> in;
		int h = atoi(in.c_str());
		itemlist.insert(itemlist.begin(), *(new item(l, h, i)));
	}
	std::sort(itemlist.begin(), itemlist.end(), comp);   //sort the list to non-increasing order to improve performance for free pattern generation
	file.close();
	return itemlist;
}


/**********************************Guillotine Cuts Patter Generation*******************************************************************/

//generate all possible strips will given length L, the output should be 2*m strips
std::vector<strip*> getStrip(int L, std::vector<item> validItemList) {
	int m = validItemList.size();			//m items
	std::vector<strip*> striplist;		//the list of all strips with length L
	//compute all strips with length L
	for (int k = 0;k < m;k++) {
		strip *cstrip = new strip(L, k, validItemList);
		int W = cstrip->W;
		cstrip->svalue = cstrip->z(m - 1, L, W, validItemList, cstrip->q);
		cstrip->updateStripValue(validItemList);
		striplist.insert(striplist.end(), cstrip);
	}
	for (int k = 0;k < m;k++) {
		strip *cstrip = new strip(L, k + m, validItemList);
		int W = cstrip->W;
		cstrip->svalue = cstrip->z(m - 1, L, W, validItemList, cstrip->q);
		cstrip->updateStripValue(validItemList);
		striplist.insert(striplist.end(), cstrip);
	}
	return striplist;
}


//compute incremental value for F(x,y)
double Usp(int *b, std::vector<item> itemlist, strip *stripj) {
	double usp = 0;
	int m = stripj->q->size();
	for (int i = 0;i < m;i++) {
		if (stripj->q->at(i) > 0 && b[i] == 1)
			usp += itemlist[i].value;
	}
	return usp;
}

//compute the possible value of bin x*y
double* F(int x, int y, int *b, std::vector<item> validItemList, std::vector<item> itemlist, std::vector<strip*>* striplist) {
	double revalue[2];
	revalue[0] = 0;
	if (x == 0 || y == 0)
		return revalue;
	double horizontal = 0;
	double vertical = 0;
	int m = validItemList.size();
	for (int j = 0;j < 2 * m;j++) {
		if (striplist[x][j]->W <= y)
			horizontal = F(x, y - striplist[x][j]->W, b, validItemList,itemlist, striplist)[0] + Usp(b, itemlist, striplist[x][j]);
		if (striplist[y][j]->W <= x)
			vertical = F(x - striplist[y][j]->W, y, b, validItemList,itemlist, striplist)[0] + Usp(b, itemlist, striplist[y][j]);
		if (revalue[0] < horizontal) {
			revalue[0] = horizontal;
			revalue[1] = 0;
		}
		if (revalue[0] < vertical) {
			revalue[0] = vertical;
			revalue[1] = 1;
		}
	}

	return revalue;
}

//generate guillotine pattern for a bin
std::vector<int> getGuillotinePattern(int L, int H, std::vector<item> itemlist, int *b, Mat img) {
	int m = itemlist.size();			//m items
	int sumvalid = m;
	std::vector<int> containItem;

	int length = max(L, H);
	int Length = 300;			//the size of the output image
	int Height = 300;
	int *btemp = new int[itemlist.size()];
	for (int i = 0;i < itemlist.size();i++) {
		btemp[i] = 1;
	}
	while(sumvalid>0) {          //there are remaining items
		bool hon = true;
		bool ver = true;
		std::vector<item> validItemList;      //items which are not packed 
		validItemList.clear();
		for (int i = 0;i < m;i++) {
			if (b[i] == 1)
				validItemList.insert(validItemList.end(), itemlist[i]);
		}

		std::vector<strip*>* striplist = new std::vector<strip*>[length + 1];
		for (int i = 0;i <= length;i++) {
			striplist[i] = getStrip(i, validItemList);			//generate strips for current length
		}
		for (int x = L;hon&&ver&&x > 0;x--)
	{	
		for (int y = H;hon&&ver&&y > 0 && x > 0;y--) {


			for (int j = 0;hon&&ver&&j < 2 * m;j++) {
				if (F(x, y, b, validItemList, itemlist, striplist)[1] == 0) {
					int width = striplist[x][j]->W;
					striplist[x][j]->updateEffectiveValue(validItemList, itemlist, b, btemp);
					if (y - width < 0 || striplist[x][j]->effvalue == 0)   //this strip is too large or has no value to current pattern
						continue;

					double fx_Wyvalue = 0;
					if (x - width >= 0)
						fx_Wyvalue = F(x - width, y, b, validItemList, itemlist, striplist)[0];			//value of pack strip with the width vertically
					double nv = 0;																		//the value of packed item for current bin and strips
					for (int i = 0;i < validItemList.size();i++) {
						int tid = validItemList[i].id;
						if (btemp[tid] == 0 || (btemp[tid] == 1 && b[i] == 1 && striplist[x][j]->q->at(i) > 0))
							nv += validItemList[tid].value;
					}
					if (nv < fx_Wyvalue)																//packs vertically is better
						continue;
					else {
						for (int i = 0;i < validItemList.size();i++) {
							int id = validItemList[i].id;
							printf("%d %d %d\n", b[id], btemp[id], striplist[x][j]->q->at(i));
							if (b[id] == 1 && btemp[id] == 1 && striplist[x][j]->q->at(i) > 0) {
								b[id] = 0;
								btemp[id] = 0;
								containItem.insert(containItem.end(), id);
								if (hon)
									y = y - width;														//height is reduced after packing this strip
								hon = false;															//a strip is packed horizontally 
								sumvalid--;																//number of available item


							}

						}
						if (x == 0 || y == 0)															//this bin is full, I think this conditional statement is not right
							sumvalid = 0;
					}
				}
				else {
					int width = striplist[y][j]->W;															//considering pack strip vertically
					striplist[y][j]->updateEffectiveValue(validItemList, itemlist, b, btemp);
					if (x - width < 0 || striplist[y][j]->effvalue == 0)   //this strip is too large or has no value to current pattern
						continue;

					double fxy_Wvalue = 0;
					if (y - width >= 0)
						fxy_Wvalue = F(x, y - width, b, validItemList, itemlist, striplist)[0];
					double nv = 0;
					for (int i = 0;i < validItemList.size();i++) {
						int tid = validItemList[i].id;
						if (btemp[tid] == 0 || (btemp[tid] == 1 && b[i] == 1 && striplist[y][j]->q->at(i) > 0))
							nv += itemlist[tid].value;
					}
					if (nv < fxy_Wvalue)
						continue;
					else {
						for (int i = 0;i < validItemList.size();i++) {
							int id = validItemList[i].id;
							//printf("%d: %d %d %d\n", id, b[id], btemp[id], striplist[y][j]->q->at(i));
							if (b[id] == 1 && btemp[id] == 1 && striplist[y][j]->q->at(i) > 0) {
								b[id] = 0;
								btemp[id] = 0;
								containItem.insert(containItem.end(), id);
								if (ver)
									x = x - width;
								sumvalid--;
								ver = false;

							}

						}
						if (x == 0 || y == 0)
							sumvalid =0 ;

					}
				}
			}
			if (!hon)
				y++;
		}
		if (!ver)
			x++;
	}
		
		if (hon&&ver)				//no strip is packed, the bin should be full
			break;
	}

	return containItem;
}




/**********************************Free Cuts Patter Generation*******************************************************************/

//Update the boundary of available space after Item cItem is packed at position j in a L*H bin
//A lot of cases are considered
std::vector<inflection> updateInflist(int L, int H, item cItem, int j, std::vector<inflection> inflist) {

	if (inflist.empty() == true) {
		inflist.insert(inflist.begin(), *(new inflection(cItem.l, 0, true)));
		inflist.insert(inflist.begin(), *(new inflection(cItem.l, cItem.h, false)));
		inflist.insert(inflist.begin(), *(new inflection(0, cItem.h, true)));
	}
	else {
		int length = inflist[j].p.x + cItem.l;
		int height = cItem.h + inflist[j].p.y;

		if (length <= L&&height <= H) {
			if (j == 0) {

				if (inflist[j + 1].p.x > length) {
					inflist.insert(inflist.begin() + 1, *(new inflection(length, inflist[1].p.y, true)));
					inflist.insert(inflist.begin() + 1, *(new inflection(length, height, false)));
					inflist[0].p.y = height;
				}
				else if (inflist[j + 1].p.x == length) {
					inflist[0].p.y = height;
					inflist[1].p.y = height;
				}
				else {
					int k;
					for (k = 1;k < inflist.size();k++) {
						if (inflist[k].p.x == length) {
							inflist[0].p.y = height;
							inflist[k].p.y = height;
							inflist.erase(inflist.begin() + 1, inflist.begin() + k);

							break;
						}
						else if (inflist[k].p.x > length) {
							if (1 > k - 2) {
								printf("%d %d\n", cItem.l, cItem.h);
								for (int q = 0;q < inflist.size();q++) {
									printf("%d: %d, %d\n", q, inflist[q].p.x, inflist[q].p.y);
								}
								getchar();
							}
							inflist[0].p.y = height;
							inflist[k - 1].p.x = length;
							inflist[k - 2].p.x = length;
							inflist[k - 2].p.y = height;
							//printf("a2\n");
							inflist.erase(inflist.begin() + 1, inflist.begin() + k - 2);
							//printf("a2\n");
							if (1 > k - 2) {
								for (int q = 0;q < inflist.size();q++) {
									printf("%d: %d, %d\n", q, inflist[q].p.x, inflist[q].p.y);
								}
								getchar();
							}
							break;
						}
					}
					if (k == inflist.size()) {
						inflist[0].p.y = height;
						inflist[k - 1].p.x = length;
						inflist[k - 2].p.x = length;
						inflist[k - 2].p.y = height;

						inflist.erase(inflist.begin() + 1, inflist.begin() + k - 2);

					}


				}

			}
			else if (j == inflist.size() - 1) {
				if (inflist[j - 1].p.y > height) {
					inflist[j].p.y = height;
					inflist.insert(inflist.begin() + j + 1, *(new inflection(length, 0, true)));
					inflist.insert(inflist.begin() + j + 1, *(new inflection(length, height, false)));
				}
				else if (inflist[j - 1].p.y == height) {
					inflist[j].p.x = length;
					inflist[j - 1].p.x = length;
				}
				else {
					int k;
					for (k = j - 1;k >= 0;k--) {
						if (inflist[k].p.y == height) {
							inflist[k].p.x = length;
							inflist[k + 1].p.x = length;
							inflist[k + 1].p.y = 0;

							inflist.erase(inflist.begin() + k + 2, inflist.end());

							break;
						}
						else if (inflist[k].p.y > height) {
							inflist[k + 1].p.y = height;

							inflist.erase(inflist.begin() + k + 2, inflist.end());

							inflist.insert(inflist.begin() + k + 2, *(new inflection(length, 0, true)));
							inflist.insert(inflist.begin() + k + 2, *(new inflection(length, height, false)));

							break;
						}
					}
					if (k == -1) {

						inflist.erase(inflist.begin(), inflist.end());

						inflist.insert(inflist.begin(), *(new inflection(length, 0, true)));
						inflist.insert(inflist.begin(), *(new inflection(length, height, false)));
						inflist.insert(inflist.begin(), *(new inflection(0, height, true)));

					}
				}
			}
			else {
				if (height < inflist[j - 1].p.y&&length < inflist[j + 1].p.x) {
					inflist[j].p.x += cItem.l;
					inflist.insert(inflist.begin() + j, *(new inflection(length, height, false)));
					inflist.insert(inflist.begin() + j, *(new inflection(length - cItem.l, height, true)));

				}
				else if (height == inflist[j - 1].p.y&&length < inflist[j + 1].p.x) {
					inflist[j - 1].p.x = length;
					inflist[j].p.x = length;

				}
				else if (height < inflist[j - 1].p.y&&length == inflist[j + 1].p.x) {
					inflist[j].p.y = height;
					inflist[j + 1].p.y = height;

				}
				else if (height == inflist[j - 1].p.y&&length == inflist[j + 1].p.x) {
					inflist[j - 1].p.x = length;

					inflist.erase(inflist.begin() + j, inflist.begin() + j + 2);

				}
				else if (height == inflist[j - 1].p.y&&length > inflist[j + 1].p.x) {
					if (inflist[inflist.size() - 1].p.x <= length) {

						inflist.erase(inflist.begin() + j + 1, inflist.end());

						inflist[j - 1].p.x = length;
						inflist[j].p.x = length;
						inflist[j].p.y = 0;

					}
					else {
						int k = 0;
						for (k = j + 1;k < inflist.size();k++) {
							if (inflist[k].candidate == true && inflist[k].p.x == length) {
								inflist[j - 1].p.x = length;
								inflist.erase(inflist.begin() + j, inflist.begin() + k);


								break;
							}
							if (inflist[k].p.x > length) {
								inflist[j - 1].p.x = length;
								inflist[k - 1].p.x = length;

								inflist.erase(inflist.begin() + j, inflist.begin() + k - 1);


								break;
							}
						}

					}

				}
				else if (height > inflist[j - 1].p.y&&length == inflist[j + 1].p.x) {
					if (inflist[0].p.y <= height) {

						inflist.erase(inflist.begin(), inflist.begin() + j + 2);

						inflist.insert(inflist.begin(), *(new inflection(length, height, false)));
						inflist.insert(inflist.begin(), *(new inflection(0, height, true)));

					}
					else {
						int k = 0;
						for (k = j - 1;k >= 0;k--) {
							if (inflist[k].p.y == height) {
								inflist[k].p.x = length;

								inflist.erase(inflist.begin() + k + 1, inflist.begin() + j + 2);

								break;
							}
							else if (inflist[k].p.y > height) {
								inflist[k + 1].p.y = height;
								inflist[j + 1].p.y = height;

								inflist.erase(inflist.begin() + k + 2, inflist.begin() + j + 1);

								break;
							}
						}
					}
				}
				else if (height > inflist[j - 1].p.y&&length < inflist[j + 1].p.x) {
					if (inflist[0].p.y <= height) {

						inflist.erase(inflist.begin(), inflist.begin() + j + 1);

						inflist.insert(inflist.begin(), *(new inflection(length, height - cItem.h, true)));
						inflist.insert(inflist.begin(), *(new inflection(length, height, false)));
						inflist.insert(inflist.begin(), *(new inflection(0, height, true)));

					}
					else {
						for (int k = j - 1;k >= 0;k--) {
							if (inflist[k].p.y > height) {
								inflist[k + 1].p.y = height;
								inflist[j].p.x = length;

								inflist.erase(inflist.begin() + k + 2, inflist.begin() + j);

								inflist.insert(inflist.begin() + k + 2, *(new inflection(length, height, false)));

								break;
							}
							else if (inflist[k].p.y == height) {
								inflist[k].p.x = length;
								inflist[j].p.x = length;

								inflist.erase(inflist.begin() + k + 1, inflist.begin() + j);

								break;
							}
						}

					}

				}
				else if (height < inflist[j - 1].p.y&&length > inflist[j + 1].p.x) {
					if (length > inflist[inflist.size() - 1].p.x) {
						inflist[j].p.y = height;

						inflist.erase(inflist.begin() + j + 1, inflist.end());

						inflist.insert(inflist.begin() + j + 1, *(new inflection(length, 0, true)));
						inflist.insert(inflist.begin() + j + 1, *(new inflection(length, height, false)));

					}
					else {
						for (int k = j + 1;k < inflist.size();k++) {
							if (inflist[k].p.x == length) {
								inflist[k].p.y = height;
								inflist[j].p.y = height;

								inflist.erase(inflist.begin() + j + 1, inflist.begin() + k);

								break;
							}
							else if (inflist[k].p.x > length) {

								inflist[k - 1].p.x = length;
								inflist[k - 2].p.x = length;
								inflist[k - 2].p.y = height;

								inflist[j].p.y = height;
								//printf("a3\n");

								inflist.erase(inflist.begin() + j + 1, inflist.begin() + k - 2);
								//printf("a3\n");

								break;
							}
						}
					}
				}
				else if (height > inflist[j - 1].p.y&&length > inflist[j + 1].p.x) {
					int k;
					for (k = j;k < inflist.size();k++) {
						if (inflist[k].p.x == length) {

							inflist[k].p.y = height;
							inflist.erase(inflist.begin() + j + 1, inflist.begin() + k);
							break;
						}
						else if (inflist[k].p.x > length) {
							inflist[k - 1].p.x = length;
							inflist[k - 2].p.x = length;
							inflist[k - 2].p.y = height;
							inflist.erase(inflist.begin() + j + 1, inflist.begin() + k - 2);

							break;
						}
					}
					if (k == inflist.size()) {
						inflist.erase(inflist.begin() + j + 1, inflist.end());
						inflist.insert(inflist.begin() + j + 1, *(new inflection(length, 0, true)));
						inflist.insert(inflist.begin() + j + 1, *(new inflection(length, height, false)));

					}
					for (k = j - 1;k >= 0;k--) {
						if (inflist[k].p.y == height) {

							inflist.erase(inflist.begin() + k, inflist.begin() + j + 1);

							break;
						}
						else if (inflist[k].p.y > height) {
							inflist[k + 1].p.y = height;

							inflist.erase(inflist.begin() + k + 2, inflist.begin() + j + 1);

							break;
						}
					}
					if (k == -1) {

						inflist.erase(inflist.begin(), inflist.begin() + j + 1);

						inflist.insert(inflist.begin(), *(new inflection(0, height, true)));


					}
				}

			}
		}
	}

	return inflist;
}
//compute the area of unavailable space
int computeArea(std::vector<inflection> inflist) {
	int area = 0;
	for (int i = 0;i < inflist.size();i++) {
		if (inflist[i].candidate == false)
			area += (inflist[i].p.x - inflist[i - 1].p.x)*inflist[i].p.y;
	}
	return area;
}
//compute the total value of all items
double getTotalItemValue(std::vector<item> itemlist) {
	double sum = 0;
	for (int i = 0;i < itemlist.size();i++) {
		sum += itemlist[i].value;
	}
	return sum;
}
//compute the equivalent area after an item is packed
int computeEA(std::vector<inflection> inflistBefore, std::vector<inflection> inflistAfter) {
	return computeArea(inflistAfter) - computeArea(inflistBefore);
}

//calculate the lower bound for packing item[cItem] at candidate position[candidatePos]
double* getLB(int L, int H, int candidatePos, int cItem, std::vector<item> itemlist, int *b, std::vector<inflection> inflistOrigin) {
	std::vector<inflection> inflist = inflistOrigin;
	//std::vector<item> itemlist = itemlistOrigin;

	if (inflist.empty() == false && (H < inflist[candidatePos].p.x + itemlist[cItem].h || L < inflist[candidatePos].p.y + itemlist[cItem].l) && (H < inflist[candidatePos].p.x + itemlist[cItem].l || L < inflist[candidatePos].p.y + itemlist[cItem].h))   //item cItem can't be placed here
	{
		double vlbDir[2] = { 0, 0 };
		return vlbDir;
	}
	bool cItemDir = itemlist[cItem].rotated;
	int bestEA;              //Minimum equivalent area
	int bestPOS;					//ID of the position with the minimum equivalent area
	int bestGN = -1;					//GN value related with bestPos
	bool bestDir = false;				//Direction of the item related with bestPos
	int m = itemlist.size();			//m items
	double vlb = 0;
	int EA = 0;
	int GN = -1;
	//update the copy of inflectionlist asuume item i is placed at position j (here I use can to refer j)
	if (!inflist.empty()) {
		if (itemlist[cItem].h + inflist[candidatePos].p.y <= H&&itemlist[cItem].l + inflist[candidatePos].p.x <= L) {
			inflist = updateInflist(L, H, itemlist[cItem], candidatePos, inflist);
		}
		else
			return NULL;
	}
	else {
		inflist.insert(inflist.begin(), *(new inflection(itemlist[cItem].l, 0, true)));
		inflist.insert(inflist.begin(), *(new inflection(itemlist[cItem].l, itemlist[cItem].h, false)));
		inflist.insert(inflist.begin(), *(new inflection(0, itemlist[cItem].h, true)));
	}
	b[cItem] = 0;
	for (int i = 0;i < m;i++) {
		if (b[i] == 0)
			continue;
		bestEA = 2147483647;
		bestPOS = -1;
		GN = -1;
		for (int j = 0;j < inflist.size();j++) {
			if (inflist[j].candidate == true) {		//j is a candidate position
				if (itemlist[i].h + inflist[j].p.y > H || itemlist[i].l + inflist[j].p.x > L)
					continue;
				else {
					itemlist[i].rotated = false;
					std::vector<inflection> infTemp = updateInflist(L, H, itemlist[i], j, inflist);
					EA = computeEA(infTemp, inflist);
					if ((j != inflist.size() - 1) && itemlist[i].l == inflist[j + 1].p.x - inflist[j].p.x)
						GN++;
					if (j > 0 && (itemlist[i].h == inflist[j - 1].p.y - inflist[j].p.y))
						GN++;
					if (EA < bestEA || (EA == bestEA&&GN > bestGN)) {
						bestEA = EA;
						bestGN = GN;
						bestPOS = j;
						bestDir = false;
					}

				}
				//rotate item
				if (itemlist[i].l + inflist[j].p.y > H || itemlist[i].h + inflist[j].p.x > L)
					continue;
				else {
					itemlist[i].rotated = true;
					int t = itemlist[i].h;
					itemlist[i].h = itemlist[i].l;
					itemlist[i].l = t;

					std::vector<inflection> infTemp = updateInflist(L, H, itemlist[i], j, inflist);
					EA = computeEA(infTemp, inflist);

					if ((j != inflist.size() - 1) && itemlist[i].l == inflist[j + 1].p.x - inflist[j].p.x)
						GN++;
					if (j > 0 && (itemlist[i].h == inflist[j - 1].p.y - inflist[j].p.y))
						GN++;
					if (EA < bestEA || (EA == bestEA&&GN > bestGN)) {
						bestEA = EA;
						bestGN = GN;
						bestPOS = j;
						bestDir = true;
					}
					t = itemlist[i].h;
					itemlist[i].h = itemlist[i].l;
					itemlist[i].l = t;
					itemlist[i].rotated = false;
				}
			}
		}
		if (bestPOS >= 0) {
			itemlist[i].rotated = bestDir;
			inflist = updateInflist(L, H, itemlist[i], bestPOS, inflist);
			vlb += itemlist[i].value;
		}
	}
	b[cItem] = 1;
	if (cItemDir == true) {
		double* vlbDir = new double[2];
		vlbDir[0] = vlb + itemlist[cItem].value;
		vlbDir[1] = 1;
		return vlbDir;
	}
	else {
		double* vlbDir = new double[2];
		vlbDir[0] = vlb + itemlist[cItem].value;
		vlbDir[1] = 0;
		return vlbDir;
	}

}

//this value correction function works for both algorithms
std::vector<item> valueCorrection(int L, int H, int *b, std::vector<item> itemlist) {
	double valueSum = 0;
	for (int i = 0;i < itemlist.size();i++)
		if(b[i]==0)
			valueSum += itemlist[i].value;
	double u = valueSum / (L*H);

	for (int i = 0;i < itemlist.size();i++) {
		itemlist[i].value = itemlist[i].value*0.7 + 0.3*pow(itemlist[i].l*itemlist[i].h, 1.0) / u;
	}
	return itemlist;
}

//generate free pattern for a bin
std::vector<int> getFreePattern(int L, int H, std::vector<item> itemlist, int *b, std::vector<inflection> inflist, Mat img) {
	double maxLB = 0;
	int optPOS;
	int optItem;
	int optDir;
	int m = itemlist.size();			//m items
	std::vector<int> containItem;

	while (true) {
		for (int i = 0;i < m;i++) {
			double vlb = 0;
			if (b[i] == 0) {
				continue;
			}
			else {
				if (inflist.empty()) {
					itemlist[i].rotated = false;
					double *vlbDir = getLB(L, H, -1, i, itemlist, b, inflist);

					itemlist[i].rotated = true;
					int t = itemlist[i].h;
					itemlist[i].h = itemlist[i].l;
					itemlist[i].l = t;
					double *vlbDir2 = getLB(L, H, -1, i, itemlist, b, inflist);

					t = itemlist[i].h;
					itemlist[i].h = itemlist[i].l;
					itemlist[i].l = t;
					itemlist[i].rotated = false;

					if (vlbDir != NULL&&vlbDir2 != NULL&&vlbDir2[0] > vlbDir[0])
						vlbDir = vlbDir2;
					else if (vlbDir == NULL&&vlbDir2 == NULL) {
						double tt[2] = { 0,0 };
						vlbDir = tt;
					}
					else if (vlbDir == NULL)
						vlbDir = vlbDir2;

					vlb = vlbDir[0];
					if (vlb > maxLB) {
						maxLB = vlb;
						optPOS = -1;
						optItem = i;
						optDir = vlbDir[1];
					}
				}
				else {
					for (int j = 0;j < inflist.size();j++) {
						if (inflist[j].candidate == true) {
							itemlist[i].rotated = false;
							double *vlbDir = getLB(L, H, j, i, itemlist, b, inflist);

							itemlist[i].rotated = true;
							int t = itemlist[i].h;
							itemlist[i].h = itemlist[i].l;
							itemlist[i].l = t;
							double *vlbDir2 = getLB(L, H, j, i, itemlist, b, inflist);

							t = itemlist[i].h;
							itemlist[i].h = itemlist[i].l;
							itemlist[i].l = t;
							itemlist[i].rotated = false;
							if (vlbDir != NULL&&vlbDir2 != NULL&&vlbDir2[0] > vlbDir[0])
								vlbDir = vlbDir2;
							else if (vlbDir == NULL&&vlbDir2 == NULL) {
								double tt[2] = { 0,0 };
								vlbDir = tt;
							}
							else if (vlbDir == NULL)
								vlbDir = vlbDir2;

							vlb = vlbDir[0];
							if (vlb > maxLB) {
								maxLB = vlb;
								optPOS = j;
								optItem = i;
								optDir = vlbDir[1];
							}
						}
					}
				}
			}

		}
		if (maxLB == 0) {
			return containItem;
		}
		if (optDir == 1) {
			itemlist[optItem].rotated = true;
			int t = itemlist[optItem].h;
			itemlist[optItem].h = itemlist[optItem].l;
			itemlist[optItem].l = t;
		}

		Rect itemRec;
		if (inflist.size() == 0) {
			itemRec.x = 0;
			itemRec.y = 0;
		}
		else {
			itemRec.x = inflist[optPOS].p.x * 30;
			itemRec.y = inflist[optPOS].p.y * 30;
		}
		itemRec.width = itemlist[optItem].l * 30;
		itemRec.height = itemlist[optItem].h * 30;

		char *itemIDtempt;
		rectangle(img, itemRec, Scalar(itemlist[optItem].r, itemlist[optItem].g, itemlist[optItem].bb), 3, 8, 0);
		std::stringstream s;
		s << optItem;
		std::string itemID = s.str();
		Point textpoint;
		textpoint.x = itemRec.x + 0.25 * 50 * itemlist[optItem].l;
		textpoint.y = itemRec.y + 0.5 * 50 * itemlist[optItem].h;
		putText(img, itemID, textpoint, 4, 0.5, Scalar(0, 200, 100), 1);
		inflist = updateInflist(L, H, itemlist[optItem], optPOS, inflist);
		b[optItem] = 0;
		//printf("%d\n", optItem);
		containItem.insert(containItem.end(), optItem);
		maxLB = 0;

	}
}

//computes the number of bins needed for both algorithm
//outputs images of cutting plans for free cuts algorithm
int computeBins(int L, int H, int Gmax, std::vector<item> itemlist, std::vector<inflection> inflist, int type) {
	int G = 0;		
	int N = 0;
	int bestN = 999999;
	int *b = new int[itemlist.size()];

	double totalvalue = 0;
	for (int i = 0;i < itemlist.size();i++) {
		b[i] = 1;								//if item[i] is not packed, b[i] is 1
		totalvalue += itemlist[i].value;
	}
	bool skip = false;							//if the solution is almost optimal, the value correction will be skiped and output the result 
	for (G = 1;G <= Gmax;G++, N++) {
		int sumb = 0;
		N = 0;
		std::vector<std::vector<int>> binContainList;
		for (int i = 0;i < itemlist.size();i++) {
			b[i] = 1;
		}
		int m = itemlist.size();

		printf("G=%d operating...\n", G);
		if (G == Gmax)
			printf("\nOutputing images of bins...\n");
		for (sumb = itemlist.size();sumb > 0;N++) {
			//printf("N %d\n", N);

			Mat img = Mat::zeros(302, 302, CV_8UC3);

			Rect itemRec;
			itemRec.x =0;
			itemRec.y =0;
			itemRec.width =300;
			itemRec.height =300;

			rectangle(img, itemRec, Scalar(255,100, 0), 3, 8, 0);

			if (type == 0)
				binContainList.insert(binContainList.end(), getFreePattern(L, H, itemlist, b, inflist, img));
			else {

				binContainList.insert(binContainList.end(), getGuillotinePattern(L, H, itemlist, b,img));
			}
			std::stringstream s;
			s << N + 1;			

			//It there are existing images with the same name, they will be painted with the patterns. 
			//So remove them before running the program
			std::string outputfile = "outputs/output" + s.str() + ".bmp";
			if (G == Gmax&&type==0) {
				cv::imwrite(outputfile, img);
			}
			sumb = 0;
			for (int i = 0;i < itemlist.size();i++) {
				sumb += b[i];
			}
		}
		if (N < bestN) {
			//save current plan as the best one for now
			bestN = N;
			if (N == ceil(totalvalue / (L*H))) {
				//The soulution of number of bins needed is optimal,because it's impossible to use less bins
				skip = true;
				G = Gmax;
			}
		}
		if(!skip)
			itemlist = valueCorrection(L, H, b, itemlist);
		printf("%d bins needed.\n", N);

	}
	return bestN;
}
