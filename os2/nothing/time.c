#include <stdio.h> 
#include <time.h> 
#include <conio.h>
void main() 
{ 
	 // 현재 시작 시간 설정. 
	 clock_t start = clock(); 
	 while(1) 
	 { 
			if(kbhit()) 
			{ 
				   // 아무키나 눌리면 출력. 
				   printf("%c", getch()); 
				   // 아무키나 입력되면 입력대기 시간 초기화. 
				   start = clock(); 
			} 
			// 입력대기 상태가 2초가 지나면 중단 
			if( (clock() - start) / CLOCKS_PER_SEC > 2.0f ) 
				break; 
	 } 
} 

