#include <stdio.h>
#include "myl.h"

int main()
{
	int i=0,num,n,temp;
	float f;
	char array[1000],ch;
	printf("Enter characters :: ");
	
	while((ch=getchar())!='\n')
	{
		array[i]=ch;
		i++;
	}
	array[i++]='\0';
	printf("Entered characters : \n");
  	num = printStr(array);
	printf("\nThe no of characters: %d\n",num);


	printf("\nEnter a number :: \n");
	n=readInt(&temp);
	if(temp==1)
	    printf("Error ! \n");
	else
	{
	    printf("The number entered : \n");
	    num = printInt(n);
	    printf("\nThe no. of characters : %d\n",num);
	}


	printf("\n");
	printf("Enter a floating point number ::::: \n");
	
	temp=readFlt(&f);
	if(temp==1)
	 printf("Error! \n");
	else
	{
	printf("The floating number entered : \n");
	//printf(" ds %f",f);
	num = printFlt(f);
	printf("\nThe number of characters : \n");
	printf("%d\n",num);
	}
	//no_of_chars = printi(n);
	//printf("%d\n",no_of_chars);
	return 0;	

}