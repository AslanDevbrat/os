#include "ssufs-ops.h"

extern struct filehandle_t file_handle_array[MAX_OPEN_FILES];

int ssufs_allocFileHandle() {
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if (file_handle_array[i].inode_number == -1) {
			return i;
		}
	}
	return -1;
}

int ssufs_create(char *filename){
	int index=0;

	if(open_namei(filename)!=-1) //같은 이름의 파일 존재
		return -1;

	index=ssufs_allocInode();
	if(index==-1) //free inode 없음
		return -1;
	
	//inode 초기화
	struct inode_t *inode=(struct inode_t *)malloc(sizeof(struct inode_t));
	inode->status=INODE_IN_USE;
	inode->file_size=0;
	for(int i=0; i<MAX_FILE_SIZE; i++)
		inode->direct_blocks[i]=-1;
	memcpy(inode->name, filename, sizeof(filename));

	ssufs_writeInode(index, inode);



	free(inode);
	return index;
}

void ssufs_delete(char *filename){
	int index=0;
	index=open_namei(filename);
	if(index==-1) //해당하는 이름의 파일 없으므로 바로 return
		return;
	for(int i=0; i<MAX_OPEN_FILES; i++){
		if(file_handle_array[i].inode_number==index){
			file_handle_array[i].offset=0;
			file_handle_array[i].inode_number=-1;
			break;
		}
	}
	ssufs_freeInode(index);
}

int ssufs_open(char *filename){
	int index=0;
	index=open_namei(filename);
	if(index==-1) //해당하는 이름의 파일 없음
		return -1;
	for(int i=0; i<MAX_OPEN_FILES; i++){
		if(file_handle_array[i].inode_number==-1){
			file_handle_array[i].offset=0;
			file_handle_array[i].inode_number=index;
			return i;
		}
	}
	return -1; //사용하지 않는 파일 핸들 없음

}

void ssufs_close(int file_handle){
	file_handle_array[file_handle].inode_number = -1;
	file_handle_array[file_handle].offset = 0;
}

int ssufs_read(int file_handle, char *buf, int nbytes){
	int offset;
	int block=offset/BLOCKSIZE; 
	int num=offset%BLOCKSIZE; 
	int nodeNum=0;
	int totalBlock=0;
	char tempBuf[BLOCKSIZE];


	struct inode_t *inode=(struct inode_t *)malloc(sizeof(struct inode_t));
	nodeNum=file_handle_array[file_handle].inode_number;
	offset=file_handle_array[file_handle].offset;

	ssufs_readInode(nodeNum, inode);

	totalBlock=inode->file_size/BLOCKSIZE;
	if(inode->file_size%BLOCKSIZE!=0)
		totalBlock++;

	if(offset+nbytes>BLOCKSIZE*totalBlock) //파일 끝 초과
		return -1;


	ssufs_readDataBlock(inode->direct_blocks[block++], tempBuf);
	memcpy(buf, tempBuf+num, BLOCKSIZE-num);

	int endBlock=0;
	endBlock=(offset+nbytes)/BLOCKSIZE;
	if((offset+nbytes)%BLOCKSIZE!=0)
		endBlock++;

	for(int k=0; k<=endBlock; k++){
		memset(tempBuf, 0x00, sizeof(tempBuf));
		ssufs_readDataBlock(inode->direct_blocks[block+k], tempBuf);
		memcpy(buf+num+(k*BLOCKSIZE), tempBuf, sizeof(tempBuf));
	}
	ssufs_lseek(file_handle, nbytes);

	return 0;


}

int ssufs_write(int file_handle, char *buf, int nbytes){
	int index[MAX_FILE_SIZE];
	int blockCnt=0; //새로 할당해야하는 block 수
	int nodeNum=0;
	int curblock=0;
	int i=0;
	int offset=0;
	struct inode_t *inode=(struct inode_t *)malloc(sizeof(struct inode_t));
	char tempBuf[BLOCKSIZE];

	nodeNum=file_handle_array[file_handle].inode_number;
	offset=file_handle_array[file_handle].offset;
	ssufs_readInode(nodeNum, inode);

	if(offset+nbytes>MAX_FILE_SIZE*BLOCKSIZE) //사이즈 초과
		return -1;

	curblock=offset/BLOCKSIZE;
	if(offset%BLOCKSIZE!=0)
		curblock++;

	blockCnt=((offset%BLOCKSIZE)+nbytes)/BLOCKSIZE;

	if((offset%BLOCKSIZE)+nbytes%BLOCKSIZE!=0){
		blockCnt++;
	}


	for(int i=0; i<blockCnt; i++){
		index[curblock+i]=ssufs_allocDataBlock();
		if(index[curblock+i]==-1){ //충분한 free 디스크 블록 없음
			for(int j=0; j<i; j++){
				ssufs_freeDataBlock(index[curblock+j]); //할당되었던 데이터 블록 해제
			}
			free(inode);
			return -1;
		}
		inode->direct_blocks[curblock+i]=index[curblock+i];
	}
	

	int size=0;

	if(inode->file_size!=0){
		memset(tempBuf, 0x00, sizeof(tempBuf));
		ssufs_readDataBlock(inode->direct_blocks[curblock], tempBuf);
		memcpy(tempBuf, buf, BLOCKSIZE-(offset%BLOCKSIZE));
		ssufs_writeDataBlock(inode->direct_blocks[curblock++], tempBuf);
	}

	
	for(int k=0; k<blockCnt; k++){
		memset(tempBuf, 0x00, sizeof(tempBuf));
		memcpy(tempBuf, buf+(offset%BLOCKSIZE)+(k*BLOCKSIZE), sizeof(tempBuf));
		ssufs_writeDataBlock(inode->direct_blocks[curblock++], tempBuf);
	}
	ssufs_lseek(file_handle, nbytes);
	if(inode->file_size-offset<=nbytes) //덮어쓰기 고려
		inode->file_size+=nbytes-(inode->file_size-offset);

	ssufs_writeInode(nodeNum, inode);
	free(inode);

	return 0;
}

int ssufs_lseek(int file_handle, int nseek){
	int offset = file_handle_array[file_handle].offset;

	struct inode_t *tmp = (struct inode_t *) malloc(sizeof(struct inode_t));
	ssufs_readInode(file_handle_array[file_handle].inode_number, tmp);
	
	int fsize = tmp->file_size;
	
	offset += nseek;

	if ((fsize == -1) || (offset < 0) || (offset > fsize)) {
		free(tmp);
		return -1;
	}

	file_handle_array[file_handle].offset = offset;
	free(tmp);

	return 0;
}
