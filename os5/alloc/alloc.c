#include "alloc.h"

struct Node{
	struct Node *next;
	int addr;
	int size;
};

char *src;
struct Node arr[PAGESIZE];

struct Node *headA;
struct Node *headD;



int allocSize;


int init_alloc()
{
	struct Node* test;
	if((src=mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0))==MAP_FAILED)
		return -1;

	allocSize=0;
	test=&arr[allocSize++];
	headA=test;

	headA->addr=0;
	headA->size=0;

	test=&arr[allocSize++];
	headD=test;
	headD->addr=0;
	headD->size=0;

	return 0;
}

int cleanup()
{
	struct Node *cur=headA->next;
	struct Node *back=headA;

	while(cur!=NULL){
		back=cur;
		cur=cur->next;
		back->addr=0;
		back->size=0;
		back->next=NULL;
	}
		headA->addr=0;
		headA->size=0;
		headA->next=NULL;

	cur=headD->next;

	while(cur!=NULL){
		back=cur;
		cur=cur->next;
		back->addr=0;
		back->size=0;
		back->next=NULL;
	}
		headD->addr=0;
		headD->size=0;
		headD->next=NULL;


	if((munmap(src, PAGESIZE))<0)
		return -1;

	return 0;
}

char* alloc(int size)
{
	struct Node *cur=headD;
	struct Node *back=headD;
	struct Node *best=headD;
	int cursize;
	int sum=0;
	char* test;
	if(size%8!=0)
		return NULL;

	//공간 재활용
	int bestfit=PAGESIZE+1;
	while(cur->next!=NULL){
		cur=cur->next;
		if(size <= (cur->size)){
			if(bestfit>(cur->size)){ //size에 가장 적합한 size 찾기
				bestfit=cur->size;
				best=cur;
			}
		}
	}
	if(bestfit<PAGESIZE+1){

		struct Node *newNode=&arr[allocSize++];
		newNode->addr=best->addr;
		newNode->size=size;

		//할당 node 리스트에 추가
		int smaller=1;
		struct Node* maxAddr=headA;
		back=headA;
		while(maxAddr->next!=NULL){
			back=maxAddr;
			maxAddr=maxAddr->next;
			if(maxAddr->addr>newNode->addr){ //나보다 addr 큰 노드 발견
				newNode->next=maxAddr;
				back->next=newNode;
				smaller=0;
				break;
			}
			
		}
		if(smaller==1){
			newNode->next=headA->next;
			headA->next=newNode;
		}
		

		//chunk 재설정
		if(best->size == size){ //재할당한 size가 같다면 바로 node 삭제
			back=headD;
			while(back->next!=best)
				back=back->next;

			back->next=best->next;
			best->addr=0;
			best->size=0;
			best->next=NULL;

			test=src+newNode->addr;
			return test;
		}
		else{ //chunk 재할당 
			best->addr=(newNode->addr)+size;
			(best->size)-=size;
			test=src+newNode->addr;
			return test;
		}
	}
	
	//새로운 공간 할당

	if(bestfit==PAGESIZE+1){
		cur=headA;
		int i=0;
		while(cur->next!=NULL){
			cur=cur->next;
			sum+=cur->size;
		}
		if(PAGESIZE-sum<size) //여유 공간이 부족한 경우
			return NULL;
		struct Node *newNode=&arr[allocSize++];
		newNode->addr=cur->addr+cur->size;
		newNode->size=size;
		newNode->next=NULL;
		cur->next=newNode;
		test=src+sum;
		return test;
	}
	return NULL;

}

void dealloc(char *ptr)
{
	struct Node *cur=headA->next;
	struct Node *back=headA;
	struct Node *back2=headD;
	struct Node *pre=headD->next;
	struct Node *post=headD->next;

	int isCombine=0;
	while(cur!=NULL){
		if(src+cur->addr==ptr){
			int preAddr;
			while(pre!=NULL){
				if(src+pre->addr+pre->size==src+cur->addr){ //free memory chunk끼리 인접함 (앞)
					(pre->size)+=cur->size;
					isCombine=1;
					break;
				}
				pre=pre->next;
			}

			while(post!=NULL){
				if(src+cur->addr+cur->size==src+post->addr){//free memory chunk끼리 인접함 (뒤)
					if(isCombine==1){ //앞뒤로 인접
						(pre->size)+=(cur->size);
						pre->next=post->next;
						post->addr=0;
						post->size=0;
						post->next=NULL;

						break;
					}
					else{
						(post->size)+=(cur->size);
						(post->addr)-=cur->size;
						break;
					}
				}
				post=post->next;
			}
			if(isCombine>0){ //인접 chunk 합친 적 있음
				//할당된 Node 리스트에서 제외하고 리턴

				back->next=cur->next;
				cur->addr=0;
				cur->size=0;
				cur->next=NULL;
				return;
			}

			//재활용 chunk node에 추가 (인접 free memory chunk 없음)
			else{
				struct Node *newNode=&arr[allocSize++];
				newNode->next=headD->next;
				headD->next=newNode;
				newNode->addr=cur->addr;
				newNode->size=cur->size;


				//할당된 Node 리스트에서 제외하고 리턴
				back->next=cur->next;
				cur->addr=0;
				cur->size=0;
				cur->next=NULL;
				return;
			}
		}
		back=cur;
		cur=cur->next;
	}
}






