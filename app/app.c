#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include "baza3.h"
#include "stdint.h"
#define DELILAC 100 //for normalization
#define QUOT 10//for normalization
#define NUMOFVAR 3
#define NUMOFSLACK NUMOFVAR
#define ROWSIZE (NUMOFSLACK+1)
#define COLSIZE (NUMOFSLACK+NUMOFVAR+1)
#define SIZE (ROWSIZE*COLSIZE+2)
char loc0[]="/dev/pivot";
char loc1[]="/dev/bram";
int i;
int j;
int p;
double uint2float(unsigned int x);
void printmat(double wv[ROWSIZE][COLSIZE])
{
	for(int j=0;j<ROWSIZE; j++)
			{
				for(int i =0;i<COLSIZE;i++)
				{
					printf("%lf,",wv[j][i]);
				}
				printf("\n");
			}
}
void printniz (unsigned int wvrow[SIZE])
{
	int p;
	printf("NIIIZ0\n");
	for(int j=0;j<ROWSIZE; j++)
			{
				for(int i =0;i<COLSIZE;i++)
				{
					//printf("((%#010x, ",(uint32_t)wvrow[j*COLSIZE+i]);
					printf("%lf, ",uint2float(wvrow[j*COLSIZE+i]));//wvrow[j*COLSIZE+i]
				}
				printf("\n");
			}
		
	
	printf("NIIIZ1\n");
}
unsigned int float2uint32(double x)
{
    if(x>=0)
    {
       // printf("%lf,%u\n",x,(unsigned int)(x*2097152));
        return (unsigned int)(x*2097152);
    }
    else
    {
       //printf("%lf,%lu\n",x,4294967295+(unsigned int)(x*2097152)+1);
        return 4294967295+(unsigned int)(x*2097152)+1;
    }
}
double uint2float(unsigned int x)
{
    if(x>=2147483648)
    {
       //printf("%u,%f\n",x,((float)(x-4294967295))/2097152);
        return ((double)(x-4294967295))/2097152;
    }
    else
    {
        //printf("%u,%f\n",x,((float)x)/2097152);
        return ((double)x)/2097152;
    }
}
int read_bram(unsigned int wvrow[SIZE])
{
	
	char endst[100];
	FILE *bram;
	if(bram == NULL)
	{
		printf("Cannot open /dev/bram for read\n");
		return -1;
	}
	uint32_t redval=33;
	/////citanje vrstu po vrstu
	for(int j=0;j<ROWSIZE; j++)
	{
		bram=fopen("/dev/bram","r");
		for(int i =0;i<COLSIZE;i++)
		{
			fscanf(bram,"%u ",&redval);
	
			printf("%ld,%d,%lf\n",ftell(bram),j*COLSIZE+i,uint2float(redval));
			wvrow[j*COLSIZE+i]=redval;
			if(feof(bram))
				printf("END %d,%d\n",j,i);
				
					
		}
		//rewind(bram);
		fflush(bram);
		fclose(bram);		
	}
	printf("krAJ\n");
	
	/////citanje sve odjednom
	/*bram=fopen("/dev/bram","r");
	printf("--------------\n");
	unsigned int redval=33;
	for(p=0;p<SIZE;p++)
	{
	
		
		fscanf(bram,"%u ",&redval);
	
		//printf("%d,%u\n",p,redval);
		wvrow[p]=redval;
		
	}
	fclose(bram);
	//bram=fopen("/dev/bram","r");	
	//fgets(endst,100,bram);
	//fgets(endst,100,bram);
	//fclose(bram);
	printf("--------------\n");
	//printniz(wvrow);*/
	return 0;
	
}
int write_bram(unsigned int wvrow[SIZE])
{
	
	FILE *bram;
	
	for(p=0;p<SIZE;p++)
	{
		bram=fopen("/dev/bram","w");
		if(bram == NULL)
		{
			printf("Cannot open /dev/bram for write\n");
			return -1;
		}
		fprintf(bram,"%d,%u\n",p,wvrow[p]);
		fclose(bram);
	}
	return 0;

}
int read_pivot(int *start,int *ready)
{
	char endst[100];
	FILE *pivot;
	pivot=fopen("/dev/pivot","r");
	int ready_reg=1;
	if(pivot == NULL)
	{
		printf("Cannot open /dev/pivot for read\n");
		return -1;
	}
	
	fscanf(pivot,"%d %d",start,ready);
	printf("Start=%d\n",*start);
	printf("Ready=%d\n",*ready);
	fgets(endst,100,pivot);
	fgets(endst,100,pivot);
	ready_reg=*ready;
	fclose(pivot);
	return ready_reg;
	return 0;
	
}
int write_pivot(int value)
{
	FILE *pivot;
	pivot=fopen("/dev/pivot","w");
	if(pivot == NULL)
	{
		printf("Cannot open /dev/pivot for write\n");
		return -1;
	}
	fprintf(pivot,"%d",value);
	fclose(pivot);
	return 0;
}

int main()
{
    printf("\nHeloo\n");
    double wv[ROWSIZE][COLSIZE];
	int wvrow[SIZE];
	int indeks=0;
	int ready=1;
	int start=0;
	//FILE *baza;
	//baza=fopen("baza.txt","r");
	for(int j=0;j<ROWSIZE; j++)
	{
		for(int i =0;i<COLSIZE;i++)
		{
			wv[j][i]=0;
		}
	}

        for(int j = 0; j < ROWSIZE; j++)
        {
            for(int i = 0; i< NUMOFVAR; i++)
            {
              wv[j][i]=baza[indeks];
			  indeks++;

            }
        }
		for(int j = 0;j< NUMOFSLACK;j++)
		{
			wv[j][COLSIZE-1]=baza[indeks];
			indeks++;
		}
	for(int j=0;j<ROWSIZE; j++)
	{
		for(int i =0;i<COLSIZE;i++)
		{
			wv[j][i]=wv[j][i]/QUOT;
		}
		//cout<<"wv: pre baze:"<<wv[j][COLSIZE-1]<<endl;
		wv[j][COLSIZE-1]=wv[j][COLSIZE-1]/DELILAC;

	}

	for(int j=0;j<ROWSIZE-1;j++)
	{
		{
			wv[j][NUMOFVAR+j]=1;
		}
	}
	

	printmat(wv);
	//write_pivot(0);
	read_pivot(&start,&ready);
	printf("Start=%d,ready=%d",start,ready);
	printf("Read from baza\n");
	
	//-----------------------------------------------------------------
    //CalculateSimplex
	printf("Starting simplex algorithm\n");
    int pivotRow=0;
    int pivotCol=0;
    bool unbounded=false;
    bool optimality=false;
    double pivot;
    int count=0;
    int tempor=0;
    //u32 tempneg=0;
    double tempflo;
    //float temp;
    //int count=0;

    //while(!optimality)
    for(count=0;count<1;count++)
    {
    	//int tempwv;
    	//count++;
        //checkOptimality(wv)
        optimality=true;
        for(int i=0;i<COLSIZE-1;i++)
        {
            if(wv[ROWSIZE-1][i]<0)
                optimality=false;
        }
        //findPivotCol(wv);
        double minnegval=wv[ROWSIZE-1][0];
        int loc=0;
        for(int i=1;i<COLSIZE-1;i++)
        {
            if(wv[ROWSIZE-1][i]<minnegval)
            {
                minnegval=wv[ROWSIZE-1][i];
                loc=i;
            }
        }
        pivotCol=loc;
        printf("pivotcol=%d\n",pivotCol);
        //isUnbounded(wv,pivotCol)
        unbounded=false;
        for(int j=0;j<ROWSIZE-1;j++)
        {
            if(wv[j][pivotCol]>0)
                unbounded=false;
        }
        if(unbounded)
        {
            break;
        }
        //findPivotRow(wv,pivotCol);
        double rat[ROWSIZE-1];
        for(int j=0;j<ROWSIZE-1;j++)
        {
            if(wv[j][pivotCol]>0)
            {
                rat[j]=wv[j][COLSIZE-1]/wv[j][pivotCol];
            }
            else
            {
                rat[j]=0;
            }
        }
        double minpozval=99999999;
        loc=0;
        for(int j=0;j<ROWSIZE-1;j++)
        {
            if(rat[j]>0)
            {
                if(rat[j]<minpozval)
                {
                    minpozval=rat[j];
                    loc=j;
                }
            }
        }
        pivotRow=loc;

        pivot=1/wv[pivotRow][pivotCol];
		//wv[pivotRow][pivotCol]=1/wv[pivotRow][pivotCol]; //za softverski implementiran hardver u drajveru

       // printmat(wv);
        printf("Sending to bram\n");
  p = 0;
  for(int j=0;j<COLSIZE;j++)
  {
		wvrow[p]=float2uint32(wv[pivotRow][j]);
		p++;
  }
  
  printf("Sent pivot row\n");
 for (int i = 1; i < ROWSIZE; ++i)
  {
  	if(i!=pivotRow){
        for (int j = 0; j < COLSIZE; ++j)
        {
			wvrow[p]=float2uint32(wv[i][j]);
			p++;
		}
	}
  }
  printf("Sent matrix \n");
  wvrow[SIZE-2]=float2uint32(pivotCol);
  wvrow[SIZE-1]=float2uint32(pivot);
  
  //printniz(wvrow);
  
  printf("p=%d\n",p);
 
  printf("row=%d, col=%d\n",pivotRow,pivotCol);
  printf("pivot=%f\n",pivot);
  //printniz(wvrow);
  write_bram(wvrow);
  for(int j=0;j<ROWSIZE; j++)
	{
		for(int i =0;i<COLSIZE;i++)
		{
			wv[j][i]=100;
		}
	}
  printmat(wv);
  printf("Sent to bram\n");


		write_pivot(1);///

		//write_pivot(0);///
		
		while(!read_pivot(&ready,&start))
			printf("Start=%d,ready=%d\n",start,ready);

		printf("Pivot complete\n");

read_bram(wvrow);
  
  p=0;  //p=1 za softverski implementiran hardver u drajveru
for (int i = 0; i < ROWSIZE; ++i)
  {
  for (int j = 0; j < COLSIZE; ++j)
  {


	  
	  wv[i][j]=uint2float(wvrow[p]);
	  
  p++;
  }
  }
  printmat(wv);
  printniz(wvrow);
  wv[0][0]=uint2float(wvrow[0]);
	//printmat(wv);
	//printf("Counter1:%d\n",(int)Xil_In32(COUNTER_REG));
  }
   //Writing results
    if(unbounded)
    {
        printf("Unbounded\n");
    }
    else
    {
        //solutions(wv);
        for(int i=0;i<NUMOFVAR; i++)
        {
            int count0 = 0;
            int index = 0;
            for(int j=0; j<ROWSIZE-1; j++)
            {
            	
                if(fabs(wv[j][i]-0.0)<0.1)
                {
                    count0 = count0+1;
                }
                else if(fabs(wv[j][i]-1.0)<0.1)
                {
                    index = j;
                }
            }
            if(count0 == ROWSIZE - 2 )
	    {	
	    	printf("variable%d: %f\n",(i+1),(DELILAC*wv[index][COLSIZE-1]));
	    }
	    else
            {
				printf("variable%d: %d\n",(i+1),0);
            }


	}
		printf("Optimal solution is %f\n",(DELILAC*QUOT*wv[ROWSIZE-1][COLSIZE-1]));

		printf("Number of iterations is %d\n",count);
	

   }

    return 0;
}
