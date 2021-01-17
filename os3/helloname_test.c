#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int main(int argc, char *argv[])
{
	*ptr=argv[1];
	hello_name(ptr);
	exit();
}

