#ifndef _VND_H_
#define _VND_H_

typedef enum { VND_TEI, VND_TIE } VndOrder;

long long int vnd(long int* s, long long int cost, VndOrder order);

#endif