#include<stdlib.h>
#include<stdio.h>
#include<string.h>


int logOfTwo(int x){
	int i = 0;
  	for(i = 0; i < 31; i++) {
   		if((x >> i) == 1){
     			break;
		}
  	}
  	return i;
}

int powerOfTwo(int x){
	int n = 1;
	for(int i = 0; i < x; i++){
		n = n * 2;	
	}
	return n;
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

void moveUp2D(unsigned long** cache, int setIndex, int i, int lines){
	for (int j=i; j<lines-1; j++){
		cache[setIndex][j] = cache[setIndex][j+1];

	}

}
void moveUp1D(unsigned long* cache, int i, int lines){
	for (int j=i; j<lines-1; j++){
		cache[j] = cache[j+1];

	}

}

int main(int argc, char* argv[argc+1]){
	if(argc < 6 || argc > 6){   //checking number or arguements
		printf("error\n");
		return EXIT_SUCCESS;
	}
	
	int cSize = atoi(argv[1]);
	int bSize = atoi(argv[4]);	
	
	if (IsPowerOfTwo(cSize) == 0){ //checking if cache size is a power of two
		printf("error\n");
		return EXIT_SUCCESS;
	}
	if (IsPowerOfTwo(bSize) == 0){ //checking if block size is a power of two
		printf("error\n");
		return EXIT_SUCCESS;
	}


	FILE* fp = fopen(argv[5], "r");

	if(fp == 0){
		printf("error\n");
		return EXIT_SUCCESS;
	}

	int reads=0;
	int writes=0;
	int hits=0;
	int misses=0;

	int bBits = logOfTwo(bSize);

	//int count; //count the size of the cache
		
	
	if(strcmp(argv[2],"direct") == 0){ //direct associtivity
		
		int sBits = logOfTwo((cSize/bSize));
		int tBits = 48 - bBits - sBits;

		int sets = cSize/bSize;
		unsigned long* direct = malloc(sets*sizeof(unsigned long));		
		
		char rOrW;
		unsigned long address;
	
		int setIndex;
		unsigned long tag;
		
		
		
		while(fscanf(fp, "%c %lx\n", &rOrW, &address) != EOF){

			setIndex = (address >> bBits) & (powerOfTwo(sBits) - 1);
			tag = (address >> (bBits + sBits)) & (powerOfTwo(tBits) - 1);
		
			if(rOrW == 'W'){
				writes++;		
			}
			if(direct[setIndex] == tag){
				hits++;

			}else{
				direct[setIndex] = tag;
				misses++;
				reads++;
			}
		
			
			
		}	
		free(direct);
	
	}else if(strcmp(argv[2],"assoc")==0){ //fully associative
		int lines = cSize/bSize;
		unsigned long* assoc = malloc(lines*sizeof(unsigned long));		
		int sBits = logOfTwo((1));
		int tBits = 48 - bBits - sBits;

		char rOrW;
		unsigned long address;

		unsigned long tag;

		while(fscanf(fp, "%c %lx\n", &rOrW, &address) != EOF){
			tag = (address >> (bBits + sBits)) & (powerOfTwo(tBits) - 1);
			if(rOrW == 'R'){
				int i = 0;
				for(i = 0; i < lines; i++){
					if(assoc[i] == tag){
						hits++;
						if(strcmp(argv[3],"fifo") != 0){
							moveUp1D(assoc, i, lines);
							assoc[lines-1] = tag;
						}
						break;
					}
				}
				if(lines == i){
					reads++;
					misses++;
		
					for(int k = 0; k < lines; k++){
						if(assoc[k] == 0){
							assoc[k] = tag;
							break;
						}
					}
					moveUp1D(assoc, 0, lines);
					assoc[lines-1] = tag;
				
				}
			}else{
				writes++;
				int i = 0;
				for(i = 0; i < lines; i++){
					if(assoc[i] == tag){
						hits++;
						if(strcmp(argv[3],"fifo") != 0){

							moveUp1D(assoc, i, lines);
							assoc[lines-1] = tag;
						}
						break;
					}
				}
				if(lines == i){
					reads++;
					misses++;
		
					for(int k = 0; k < lines; k++){
						if(assoc[k] == 0){
							assoc[k] = tag;
							break;
						}
					}

					moveUp1D(assoc, 0, lines);
					assoc[lines-1] = tag;
					
				}

			}
		}
		free(assoc);
	}else{   	//n associative
		
		char* s = argv[2];
		int n = atoi(&s[6]);
	
		int sets = (cSize)/(bSize * n);
		int lines = n;
			
		int sBits = logOfTwo((cSize)/(bSize * n));
		int tBits = 48 - bBits - sBits;

		char rOrW;
		unsigned long address;

		unsigned long tag;
		int setIndex;

		unsigned long** nAssoc = malloc(sets*sizeof(unsigned long*));
		for(int i = 0; i<sets; i++){
			nAssoc[i] = malloc(lines*sizeof(unsigned long));
		}

		while(fscanf(fp, "%c %lx\n", &rOrW, &address) != EOF){
			setIndex = (address >> bBits) & (powerOfTwo(sBits) - 1);
			tag = (address >> (bBits + sBits)) & (powerOfTwo(tBits) - 1);


			if(rOrW == 'R'){
				int i = 0;
				for(i = 0; i < lines; i++){
					if(nAssoc[setIndex][i] == tag){
						hits++;
						if(strcmp(argv[3],"lru") == 0){
							moveUp2D(nAssoc,setIndex, i, lines);
							nAssoc[setIndex][lines-1] = 0;
							for(int k =0; k<lines;k++){
								if(nAssoc[setIndex][k] == 0){
									nAssoc[setIndex][k] = tag;
									break;
								}
							}
						}
						break;
					}
				}
				if(lines == i){
					reads++;
					misses++;
		
					for(int k = 0; k < lines; k++){
						if(nAssoc[setIndex][k] == 0){
							nAssoc[setIndex][k] = tag;
							break;
						}
					}
					moveUp2D(nAssoc,setIndex, 0, lines);
					nAssoc[setIndex][lines-1] = tag;
					
				}
			}else{
				writes++;
				int i = 0;
				for(i = 0; i < lines; i++){
					if(nAssoc[setIndex][i] == tag){
						hits++;
						if(strcmp(argv[3],"lru") == 0){
							moveUp2D(nAssoc,setIndex, i, lines);
							nAssoc[setIndex][lines-1] = tag;
						}
						break;
					}
				}
				if(lines == i){
					reads++;
					misses++;
		
					for(int k = 0; k < lines; k++){
						if(nAssoc[setIndex][k] == 0){
							nAssoc[setIndex][k] = tag;
							break;
						}
					}

					moveUp2D(nAssoc,setIndex, 0, lines);
					nAssoc[setIndex][lines-1] = tag;
					
				}
			}
		}
		for(int i = 0; i < sets;i++){
			free(nAssoc[i]);
		}
		free(nAssoc);

			
		
		}

	printf("memread:%d\n", reads);
	printf("memwrite:%d\n", writes);
	printf("cachehit:%d\n", hits);
	printf("cachemiss:%d\n", misses);
	fclose(fp);
	return EXIT_SUCCESS;


	}


	

	
	


