#include "ealloc.h"
struct Node{
	struct Node *next;
	int addr;
	int size;
};

struct Page{
	struct Page *next;
	struct Node *headA;
	struct Node *headD;

	int cnt;
	char* src;
};

struct Node nodeArr[PAGESIZE];
struct Page pageArr[PAGESIZE];

int nodeCnt;
int pageCnt;

//char *src;
//struct Page *headA;
//struct Page *headD;

struct Page *headP;

void init_alloc(void)
{
	nodeCnt=0;
	pageCnt=0;
	headP=&pageArr[pageCnt++];
	headP->cnt=0;

	return;
}

char *alloc(int size)
{
	struct Node *cur;
	struct Node *back;
	struct Node *best;

	struct Page *curP=headP;

	int cursize;
	int sum=0;
	int curPage=0;
	char* test;
	if(size % MINALLOC!=0 || size > PAGESIZE)
		return NULL;


	int bestfit=PAGESIZE+1;
	int debug=0;
	while(curP->next!=NULL){ //page 단위  while
		curP=curP->next;
		debug++;
		cur=curP->headD;
		while(cur->next!=NULL){ //chunk 단위 while
			cur=cur->next;
			if(size <= (cur->size)){
				if(bestfit>(cur->size)){ //size에 가장 적합한 size 찾기
					bestfit=cur->size;
					best=cur;
					curPage=curP->cnt; //가장 적합한 size가 있는 page num 저장
				}
			}
		}
	}


	if(bestfit<PAGESIZE+1){ //재활용할 node 찾음

		struct Node *newNode=&nodeArr[nodeCnt++];
		newNode->addr=best->addr;
		newNode->size=size;

		//할당 node 리스트에 추가

		curP=headP;
		while(curP->next!=NULL){
			curP=curP->next;
			if(curP->cnt==curPage) //bestfit 자리가 있던 page까지 찾아가기
				break;
		}

		int smaller=1;
		struct Node* maxAddr=curP->headA;
		back=curP->headA;
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


		if(smaller==1){ //newNode가 가장 작은 addr
			newNode->next=curP->headA->next;
			curP->headA->next=newNode;
		}


		//chunk 재설정
		if(best->size == size){ //재할당한 size가 같다면 바로 node 삭제
			back=curP->headD;
			while(back->next!=best)
				back=back->next;

			back->next=best->next;
			best->addr=0;
			best->size=0;
			best->next=NULL;

			test=curP->src+newNode->addr;
			return test;
		}
		else{ //chunk 재할당
			best->addr=(newNode->addr)+size;
			(best->size)-=size;
			test=curP->src+newNode->addr;
			return test;
		}
	}

	//새로운 공간 할당 page 내에서 한 번도 할당되지 않은 부분을 쓰거나 or 새로운 page 할당을 해줘야 함


	int curPageCnt=0;
	if(bestfit==PAGESIZE+1){
		curP=headP;

		while(curP->next!=NULL){
			curP=curP->next;
			cur=curP->headA;
			curPageCnt++;
			

			while(cur->next!=NULL){
				cur=cur->next;
				sum+=cur->size;
			}


			if(PAGESIZE-sum>=size){ //여유 공간이 있는 경우
				struct Node *newNode=&nodeArr[nodeCnt++];
				newNode->addr=cur->addr+cur->size;
				newNode->size=size;
				newNode->next=NULL;
				cur->next=newNode;
				test=curP->src+sum;
				return test;
			}
		}

		//새로운 page 할당해줘야 함

		if(curP->cnt==4) //이미 4page까지 할당했으면 NULL
			return NULL;

		struct Page *newPage=&pageArr[pageCnt++];


		newPage->src=mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
		newPage->cnt=curPageCnt+1;
		

		curP->next=newPage; //page list에 추가
		newPage->next=NULL;


		struct Node *headA=&nodeArr[nodeCnt++];
		headA->addr=0;
		headA->size=0;


		struct Node *headD=&nodeArr[nodeCnt++];
		headD->addr=0;
		headD->size=0;


		struct Node *newNode=&nodeArr[nodeCnt++];
		newNode->addr=0; //방금 생성한 page의 첫 node이므로 0
		newNode->size=size;
		newNode->next=NULL;

		headA->next=newNode; //head와 연결

		newPage->headA=headA; //page와 연결
		newPage->headD=headD;

		test=newPage->src;
		return test;

	}
	return NULL;

}

void dealloc(char *ptr)
{
	struct Node *cur;
	struct Node *back;
	struct Node *back2;
	struct Node *pre;
	struct Node *post;

	int isCombine=0;
	char* src;

	struct Page *curP=headP->next;
	while(curP!=NULL){
		cur=curP->headA->next;
		src=curP->src;
		pre=curP->headD->next;
		back=curP->headA;
		back2=curP->headD;
		post=curP->headD->next;

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
					struct Node *newNode=&nodeArr[nodeCnt++];
					newNode->next=curP->headD->next;
					curP->headD->next=newNode;
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
		curP=curP->next;

	}

}

void cleanup(void)
{
	struct Page *curP=headP->next;
	struct Page *backP;
	struct Node *cur;
	struct Node *back;

	while(curP!=NULL){
		cur=curP->headA->next;
		back=curP->headA;

		while(cur!=NULL){
			back=cur;
			cur=cur->next;
			back->addr=0;
			back->size=0;
			back->next=NULL;
		}
		curP->headA->addr=0;
		curP->headA->size=0;
		curP->headA->next=NULL;

		cur=curP->headD->next;

		while(cur!=NULL){
			back=cur;
			cur=cur->next;
			back->addr=0;
			back->size=0;
			back->next=NULL;
			
		}

		curP->headD->addr=0;
		curP->headD->size=0;
		curP->headD->next=NULL;

		backP=curP;
		curP=curP->next;
		backP->next=NULL;
		backP->headA=NULL;
		backP->headD=NULL;

	}

	return ;

}
