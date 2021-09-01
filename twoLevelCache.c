
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int powerOfTwo(int x);
int logOfTwo(int x);
int IsPowerOfTwo(int x);
unsigned long** createCache(int x, int y);
unsigned long** initArray(unsigned long** arr,int x,int y);
void freeArray(unsigned long** arr, int x);
void moveUp(unsigned long** cache, unsigned long** hex, int setIndex, int i, int lines);

int main(int argc, char* argv[argc+1]){
	if (argc < 9 || argc > 9){
		printf("error\n");
		return EXIT_SUCCESS;
	}
	int cSize1 = atoi(argv[1]);
	int cSize2 = atoi(argv[5]);
	int bSize = atoi(argv[4]);
	if (IsPowerOfTwo(cSize1) == 0){
		printf("error\n");
		return EXIT_SUCCESS;
	}
	if (IsPowerOfTwo(cSize2) == 0){
		printf("error\n");
		return EXIT_SUCCESS;
	}
	if (IsPowerOfTwo(bSize) == 0){
		printf("error\n");
		return EXIT_SUCCESS;
	}
	FILE *fp = fopen(argv[8], "r");
	if (fp==0){
		printf("error\n");
		return EXIT_SUCCESS;
	}
	int l1Sets = 0;
	int l2Sets = 0;
	int l1lines = 0;
	int l2lines = 0;
	if (strcmp(argv[2],"direct") == 0){
		l1Sets = cSize1/bSize;
		l1lines = 1;
	}
	else if (strcmp(argv[2],"assoc") == 0){
		l1Sets = 1;
		l1lines = cSize1/bSize;
	}
	else{
		char* strL1 = argv[2];
		l1lines = atoi(&strL1[6]);
		l1Sets = (cSize1)/(bSize*l1lines);
	}
	if (strcmp(argv[6],"direct") == 0){
		l2Sets = cSize2/bSize;
		l2lines = 1;
	}
	else if (strcmp(argv[6],"assoc") == 0){
		l2Sets = 1;
		l2lines = cSize2/bSize;
	}
	else{
		char* strL2 = argv[6];
		l2lines = atoi(&strL2[6]);
		l2Sets = (cSize2)/(bSize*l2lines);
	}
	unsigned long** l1Cache = createCache(l1Sets,l1lines);
	unsigned long** hex1 = createCache(l1Sets,l1lines);
	unsigned long** l2Cache = createCache(l2Sets,l2lines);
	unsigned long** hex2 = createCache(l2Sets,l2lines);
	l1Cache = initArray(l1Cache,l1Sets,l1lines);
	hex1 = initArray(hex1,l1Sets,l1lines);
	l2Cache = initArray(l2Cache,l2Sets,l2lines);
	hex2 = initArray(hex2,l2Sets,l2lines);
	int reads=0;
	int writes=0;
	int l1hits=0;
	int l1misses=0;
	int l2hits=0;
	int l2misses=0;
	char rOrW;
	unsigned long address;
	int bBits = logOfTwo(bSize);
	int s1Bits = logOfTwo(l1Sets);
	int s2Bits = logOfTwo(l2Sets);
	int t1Bits = 48-bBits-s1Bits;
	int t2Bits = 48-bBits-s2Bits;

	while(fscanf(fp,"%c %lx\n",&rOrW,&address)!=EOF){
		int setIndex1 = (address>>bBits)&(l1Sets-1);
		int setIndex2 = (address>>bBits)&(l2Sets-1);
		unsigned long tag1 = (address>>(bBits+s1Bits))&(powerOfTwo(t1Bits)-1);
		unsigned long tag2 = (address>>(bBits+s2Bits))&(powerOfTwo(t2Bits)-1);
		if(rOrW == 'W'){
			writes++;
			int i=0;
			for(i=0; i<l1lines; i++){
				if(l1Cache[setIndex1][i] == tag1){
					l1hits++;
					if(strcmp(argv[3],"lru") == 0){
						moveUp(l1Cache,hex1,setIndex1,i,l1lines);
						hex1[setIndex1][l1lines-1] = 0;
						l1Cache[setIndex1][l1lines-1] = 0;
						for(int k =0; k<l1lines;k++){
							if(hex1[setIndex1][k] == 0){
								hex1[setIndex1][k] = address;
								l1Cache[setIndex1][k] = tag1;
								break;
							}
						}
						
						
					}
				break;
				}
			}
			if(i==l1lines){
				l1misses++;
				int i=0;
				for(i=0;i<l2lines;i++){
					if(l2Cache[setIndex2][i]==tag2){
						l2hits++;
						moveUp(l2Cache,hex2,setIndex2,i,l2lines);
						hex2[setIndex2][l2lines-1] = 0;
						l2Cache[setIndex2][l2lines-1] = 0;
						int z=0;
						for(z=0; z<l1lines; z++){
							if (l1Cache[setIndex1][z] == 0){
								hex1[setIndex1][z] = address;
								l1Cache[setIndex1][z] = tag1;

								break;
							}
						}
						if(z==l1lines){
							unsigned long temp = hex1[setIndex1][0];
							moveUp(l1Cache,hex1,setIndex1,0,l1lines);
							hex1[setIndex1][l1lines-1] = address;
							l1Cache[setIndex1][l1lines-1] = tag1;
							int newIndex = (temp>>bBits)&(l2Sets-1);
							unsigned long newTag = (temp>>(bBits+s2Bits))&(powerOfTwo(t2Bits)-1);
							int qu=0;
							for(qu=0; qu<l2lines; qu++){
								if(l2Cache[newIndex][qu] == 0){
									hex2[newIndex][qu] = temp;
									l2Cache[newIndex][qu] = newTag;
									break;
								}
							}
							if(qu==l2lines){
								moveUp(l2Cache,hex2,newIndex,0,l2lines);
								hex2[newIndex][l2lines-1] = temp;
								l2Cache[newIndex][l2lines-1] = newTag;
							}
						}
						break;
					}
				}
				if(i==l2lines){
					l2misses++;
					reads++;
					int q=0;
					for(q=0; q<l1lines; q++){
						if(l1Cache[setIndex1][q] == 0){
							hex1[setIndex1][q] = address;
							l1Cache[setIndex1][q] = tag1;
							break;
						}
					}
					if(q==l1lines){
						unsigned long temp = hex1[setIndex1][0];
						moveUp(l1Cache,hex1,setIndex1,0,l1lines);
						hex1[setIndex1][l1lines-1] = address;
						l1Cache[setIndex1][l1lines-1] = tag1;

						int y=0;
						int newIndex = (temp>>bBits)&(l2Sets-1);
						unsigned long newTag = (temp>>(bBits+s2Bits))&(powerOfTwo(t2Bits)-1);
						for(y=0; y<l2lines; y++){
							if(l2Cache[newIndex][y] == 0){
								hex2[newIndex][y] = temp;
								l2Cache[newIndex][y] = newTag;
								break;
							}
						}
						if(y==l2lines){
							hex2[newIndex][l2lines-1] = temp;
							moveUp(l2Cache,hex2,newIndex,0,l2lines);
							l2Cache[newIndex][l2lines-1] = newTag;
						}
					}
				}
			}
		}else{
			int i=0;
			for(i=0; i<l1lines; i++){
				if(l1Cache[setIndex1][i] == tag1){
					l1hits++;
					if(strcmp(argv[3],"lru") == 0){
						moveUp(l1Cache,hex1,setIndex1,i,l1lines);
						hex1[setIndex1][l1lines-1] = 0;
						l1Cache[setIndex1][l1lines-1] = 0;
						for(int k =0; k<l1lines;k++){
							if(hex1[setIndex1][k] == 0){
								hex1[setIndex1][k] = address;
								l1Cache[setIndex1][k] = tag1;
								break;
							}
						}

					}
				break;
				}
			}
			if(i==l1lines){
				l1misses++;
				int i=0;
				for(i=0; i<l2lines; i++){
					if(l2Cache[setIndex2][i] == tag2){
						l2hits++;
						moveUp(l2Cache,hex2,setIndex2,i,l2lines);
						hex2[setIndex2][l2lines-1] = 0;
						l2Cache[setIndex2][l2lines-1] = 0;
						int z=0;
						for(z=0; z<l1lines; z++){
							if(l1Cache[setIndex1][z] == 0){
								hex1[setIndex1][z] = address;
								l1Cache[setIndex1][z] = tag1;
								break;
							}
						}
						if(z==l1lines){
							unsigned long temp = hex1[setIndex1][0];
							moveUp(l1Cache,hex1,setIndex1,0,l1lines);
							hex1[setIndex1][l1lines-1] = address;
							l1Cache[setIndex1][l1lines-1] = tag1;
							int newIndex = (temp>>bBits)&(l2Sets-1);
							unsigned long newTag = (temp>>(bBits+s2Bits))&(powerOfTwo(t2Bits)-1);
							int qu=0;
							for(qu=0; qu<l2lines; qu++){
								if(l2Cache[newIndex][qu] == 0){
									hex2[newIndex][qu] = temp;
									l2Cache[newIndex][qu] = newTag;
									break;
								}
							}
							if(qu==l2lines){
								moveUp(l2Cache,hex2,newIndex,0,l2lines);
								hex2[newIndex][l2lines-1] = temp;
								l2Cache[newIndex][l2lines-1] = newTag;
							}
						}
						break;
					}
				}
				if(i==l2lines){
					l2misses++;
					reads++;
					int q=0;
					for(q=0; q<l1lines; q++){
						if(l1Cache[setIndex1][q] == 0){
							hex1[setIndex1][q] = address;
							l1Cache[setIndex1][q] = tag1;
							break;
						}
					}
					if(q==l1lines){
						unsigned long temp = hex1[setIndex1][0];
						moveUp(l1Cache,hex1,setIndex1,0,l1lines);
						hex1[setIndex1][l1lines-1] = address;
						l1Cache[setIndex1][l1lines-1] = tag1;
						int newIndex = (temp>>bBits)&(l2Sets-1);
						unsigned long newTag = (temp>>(bBits+s2Bits))&(powerOfTwo(t2Bits)-1);
						int y = 0;
						for(y=0; y<l2lines; y++){
							if (l2Cache[newIndex][y] == 0){
								hex2[newIndex][y] = temp;
								l2Cache[newIndex][y] = newTag;
								break;
							}
						}
						if(y==l2lines){
							moveUp(l2Cache,hex2,newIndex,0,l2lines);
							hex2[newIndex][l2lines-1] = temp;
							l2Cache[newIndex][l2lines-1] = newTag;
						}
					}
				}
			}
		}	
	}
	
	
	printf("memread:%d\n",reads);
	printf("memwrite:%d\n",writes);
	printf("l1cachehit:%d\n",l1hits);
	printf("l1cachemiss:%d\n",l1misses);
	printf("l2cachehit:%d\n",l2hits);
	printf("l2cachemiss:%d\n",l2misses);
	

	freeArray(l1Cache, l1Sets);
	freeArray(l2Cache, l2Sets);
	freeArray(hex1, l1Sets);
	freeArray(hex2, l2Sets);
	
	fclose(fp);
	return EXIT_SUCCESS;
}
int powerOfTwo(int x){
	int n = 1;
	for (int i=0; i<x; i++){
		n = n*2;
	}
	return n;
	}
int logOfTwo(int x){
	int i = 0;
  	for(i = 0; i < 31; i++) {
   		if((x >> i) == 1){
     			break;
		}
  	}
  	return i;
}
int IsPowerOfTwo(int x){
	if (x!=0){
		while (x!=1){
			if(x%2 == 1){
				return 0;
			}
			x = x/2;
		}
		return 1;
	}	
	return 0;
}
unsigned long** createCache(int x, int y){
	unsigned long** arr = malloc(x*sizeof(unsigned long*));
	for (int i=0; i<x; i++){
		arr[i] = malloc(y*sizeof(unsigned long));
	}
	return arr;
}
unsigned long** initArray(unsigned long** arr,int x,int y){
	for (int i=0; i<x; i++){
		for (int j=0; j<y; j++){
		arr[i][j] = 0;
		}
	}
	return arr;
}
void freeArray(unsigned long** arr, int x){
	for (int i=0; i<x; i++){
		free(arr[i]);
	}
	free(arr);
}
void moveUp(unsigned long** cache, unsigned long** hex, int setIndex, int i, int lines){
	for (int j=i; j<lines-1; j++){
		cache[setIndex][j] = cache[setIndex][j+1];
		hex[setIndex][j] = hex[setIndex][j+1];
	}

}


