#include "myl.h"

//Printing the character string 
int printStr(char *str)
{
	int num=0;
	while(*(str+num)!='\0')
		num++;
	__asm__ __volatile__(
		"movl $1, %%eax \n\t"
		"movq $1, %%rdi \n\t"
		"syscall \n\t"
		:
		: "S"(str), "d"(num)
		);
	return num;
}





// Reading a signed integer as input
int readInt(int *n)
{

char buff[1000];
int i=0,flag=0,neg,temp=0;

do
{
    __asm__ __volatile(
        "movl $0, %%eax \n\t"
        "movq $1, %%rdi \n\t"
        "syscall \n\t"
        :
        :"S"(buff+i), "d"(1)
    );
}while(buff[i++]!='\n');
i--;

if(buff[0]=='-')
{
	flag=1;
	neg=-1;
}
else
{
	flag=0;
	neg=1;
}

while(flag<i)
{
	if(buff[flag]<'0'||buff[flag]>'9')
	{    
		(*n) = ERR;
		return ERR; 	
	}        
	temp = temp*10 + (int)(buff[flag]-'0');
	flag++;
}
temp=temp*neg;
*n=OK;
return temp;

}



// printing a signed integer
int printInt(int n)
{
	char buff[1000];
	int i=0,flag,k,num=n;
	if(num==0)
		buff[i++]='0';
	else
	{
		if(num<0)
		{
			buff[i]='-';
			i++;
			num=(-1)*num;
		}

		while(num>0)
		{
			int dig=num%10;
			buff[i]=(char)(dig+'0');
			i++;
			num=num/10;
		}


		if(buff[0]=='-')
			flag=1;
		else
			flag=0;
		k=i-1;
		while(flag<=k)
		{
			char temp=buff[flag];
			buff[flag++]=buff[k];
			buff[k--]=temp;
		}
	}
	buff[i]='\n';
	__asm__ __volatile__(
          "movl $1, %%eax \n\t"
          "movq $1, %%rdi \n\t"
          "syscall \n\t"
          :
          : "S"(buff), "d"(i)
          );
	return i;
}




// printing a signed float 
int printFlt(float f)
{

char buff[1000];
int i=0,flag,k,front,l;
int count=0;
float dec;
if(f==0)
	buff[i++]='0';
else 
{
	if(f<0)
	{
		buff[i++]='-';
		f=(-1)*f;
	}
}

	front=(int)f;
	dec=f-front;

	while(front>0)
	{
		int dig=front%10;
		buff[i++]=(char)(dig+'0');
		front=front/10;
	}

	if(buff[0]=='-')
		flag=1;
	else
		flag=0;
	l=k=i-1;
	while(flag<k)
	{
		char temp=buff[flag];
		buff[flag++]=buff[k];
		buff[k--]=temp;
	}
	buff[i++]='.';
	if(dec==0.0)
		buff[i++]='0';
	else
	{
		int d=0;
		while(dec!=0.00000)
		{
			d=(int)(dec*10);
			buff[i++]=(char)(d+'0');
			dec=dec*10;
			dec-=d;
			count++;
		}
		//i=i-count;
	}
__asm__ __volatile__(
        "movl $1, %%eax \n\t"
        "movq $1, %%rdi \n\t"
        "syscall \n\t"
        :
        : "S"(buff), "d"(i-count+l+1)
        );
return (i-count+l+1);
}


// reading a signed float
int readFlt(float *f)
{
char buff[1000];
int i=0,flag=0,neg;
*f=0;
do
{
	__asm__ __volatile__(
		"movl $0, %%eax \n\t"
		"movq $1, %%rdi \n\t"
		"syscall \n\t"
		:
		: "S"(buff+i), "d"(1)
		);

}while(buff[i++]!='\n');
i--;


if(buff[0]=='-')
{
	flag=1;
	neg=-1;
}
else
{
	flag=0;
	neg=1;
}

while(flag<i && buff[flag]!='.')
{
	if(buff[flag]<'0' || buff[flag]>'9')
	{
		*f=ERR;
		return ERR;
	}
	(*f)=(*f)*10+(float)(buff[flag]-'0');
	flag++;
}

flag++;
int power=1;
while(flag<i)
{
	if(buff[flag]<'0' || buff[flag]>'9')
	{
		(*f)=ERR;
		return ERR;
	}
	(*f)=(*f)*10+(float)(buff[flag]-'0');
	power*=10;
	flag++;
}
(*f)=((*f)*neg)/power;
return OK;
}





