#include<stdio.h>



int main()

{

	int a,b;



	while(1)

	{



		

		scanf("%d",&a);

		if(getchar()=='q')

		{

			printf("종료");

			return 0;

		}

//		fflush(stdin);

		

		scanf("%d",&b);

		printf("%d %d\n",a,b);



	}



	return 0;

}
