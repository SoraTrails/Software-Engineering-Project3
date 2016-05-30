#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include<math.h>
#include<time.h>

#define M 50
#define BLOCKNUM 3
#define STEPSIZE 500	//(-STEPSIZE, STEPSIZE)
#define CORENUM 4

#define TRANS(s) (int)(s * STEPSIZE)
	
typedef struct _PRARM{
	int *presentNum;
	int quadrant;
	int r;
}*PPRARM;

typedef struct _circle {
	//int order;
	int x;
	int y;
	int r;
	_circle *next;
}CIRCLE;

typedef struct _ball {
	//int order;
	double x;
	double y;
	double z;
	double r;
	_ball *next;
}BALL;

typedef struct _block2
{
	int x;
	int y;
}BLOCK2;

typedef struct _block3
{
	double x;
	double y;
	double z;
}BLOCK3;

CIRCLE* circlesHead = new CIRCLE, *circlesTail = circlesHead;
BALL* ballsHead = new BALL, *ballsTail = ballsHead;

BLOCK2 blocks2[BLOCKNUM];
BLOCK3 blocks3[BLOCKNUM];

CRITICAL_SECTION mutex2;

void produceBlock()
{
	//srand(time(NULL));
	//test1:
	blocks2[0].x = TRANS(0.9);
	blocks2[0].y = TRANS(0.9);
	blocks2[1].x = TRANS(-0.9);
	blocks2[1].y = TRANS(0.9);
	blocks2[2].x = TRANS(0.9);
	blocks2[2].y = TRANS(-0.9);
	//test2:
	//blocks2[0].x = 0;
	//blocks2[0].y = 0;
	//blocks2[1].x = -0.9;
	//blocks2[1].y = 0.9;
	//blocks2[2].x = 0.9;
	//blocks2[2].y = -0.9;

	//for (int i = 0;i < BLOCKNUM;i++)
	//{
	//	blocks2[i].x = (double)((double)rand() / RAND_MAX) * 2 - 1;
	//	blocks2[i].y = (double)((double)rand() / RAND_MAX) * 2 - 1;
	//	blocks3[i].x = (double)((double)rand() / RAND_MAX) * 2 - 1;
	//	blocks3[i].y = (double)((double)rand() / RAND_MAX) * 2 - 1;
	//	blocks3[i].z = (double)((double)rand() / RAND_MAX) * 2 - 1;
	//}
}

bool ifOverlap2(int x, int y, int r)
{
	for (int i = 0;i < BLOCKNUM;i++)
		if (r > sqrt((blocks2[i].x - x)*(blocks2[i].x - x) + (blocks2[i].y - y)*(blocks2[i].y - y)))
			return false;

  	EnterCriticalSection(&mutex2);

	CIRCLE *temp = circlesHead->next;
	while (temp != NULL)
	{
		if (temp->r + r > sqrt((temp->x - x)*(temp->x - x) + (temp->y - y)*(temp->y - y)))
		{
			LeaveCriticalSection(&mutex2);
			return false;
		}
		temp = temp->next;
	}

	LeaveCriticalSection(&mutex2);
	return true;
}
bool ifOverlap3(double x, double y, double z, double r)
{
	//if (x + r > 1 || x - r < -1 || y + r > 1 || y - r < -1 || z + r > 1 || z - r < -1)
	//	return false;
	BALL *temp = ballsHead->next;

	for (int i = 0;i < BLOCKNUM;i++)
		if (r > sqrt((blocks3[i].x - x)*(blocks3[i].x - x) + (blocks3[i].y - y)*(blocks3[i].y - y) +
			(blocks3[i].z - z)*(blocks3[i].z - z)))
			return false;

	while (temp != NULL)
	{
		if (temp->r + r > sqrt((temp->x - x)*(temp->x - x) + (temp->y - y)*(temp->y - y) + (temp->z - z)*(temp->z - z)))
			return false;
		temp = temp->next;
	}
	return true;
}


DWORD WINAPI max2dThread(LPVOID lpParam)
{
	PPRARM p = (PPRARM)lpParam;
	int *presentNum = p->presentNum;
	int quadrant = p->quadrant;///???
	int r = p->r;

	int lBoder, rBoder, uBoder, dBoder;

	if (quadrant <= 2)	{
		uBoder = STEPSIZE - r;
		dBoder = 0;
	}
	else {
		uBoder = 0;
		dBoder = (-1)*STEPSIZE + r;
	}

	if (quadrant >= 2 && quadrant <= 3)	{
		rBoder = 0;
		lBoder = (-1)*STEPSIZE + r;
	}
	else {
		rBoder = STEPSIZE - r;
		lBoder = 0;
	}

	CIRCLE *temp;

	for (int x = lBoder;x < rBoder;x++)
		for (int y = dBoder;y < uBoder;y++)
		{
			if (ifOverlap2(x, y, r))
			{
				EnterCriticalSection(&mutex2);

				temp = new CIRCLE;
				temp->r = r;
				temp->x = x;
				temp->y = y;
				temp->next = NULL;
				circlesTail->next = temp;
				circlesTail = circlesTail->next;

				LeaveCriticalSection(&mutex2);
				(*presentNum)++;
				return 1;
			}
		}
	return 0;
}

void max2d()
{
	InitializeCriticalSection(&mutex2);

	int presentNum = 0;
	PPRARM p[CORENUM];
	HANDLE hThread[CORENUM];

	for (int i = 0;i < CORENUM;i++)
	{
		p[i] = new _PRARM;
		p[i]->presentNum = &presentNum;
		p[i]->quadrant = i + 1;
	}

	for (int r = STEPSIZE;r > 0;r--)
	{
		for (int i = 0;i < CORENUM;i++)   //分四个象限
		{
			p[i]->r = r;
			hThread[i] = CreateThread(NULL, 0, max2dThread, p[i], 0, NULL);
		}
		WaitForMultipleObjects(CORENUM, hThread, true, INFINITE);

		if (presentNum == M)
			return;
		else if (presentNum > M)
		{
			CIRCLE *temp = circlesHead, *temp0;
			int i = 0;

			while (++i < M)
				temp = temp->next;

			temp0 = temp->next;
			temp->next = NULL;
			while (temp0 != NULL)
			{
				temp = temp0->next;
				delete temp0;
				temp0 = temp;
			}
			return;
		}
	}
}

void max3d()
{
	BALL *add = ballsHead, *temp;

	for (int i = 0;i < M;i++)
	{
		for (double r = 1 - 1e-6;r > 0;r -= STEPSIZE)
		{
			for (double x = -1.0 + r;x < 1.0 - r;x += STEPSIZE)
				for (double y = -1.0 + r;y < 1.0 - r;y += STEPSIZE)
					for (double z = -1.0 + r;z <1.0 - r;z += STEPSIZE)
					{
						if (ifOverlap3(x, y, z, r))
						{
							temp = new BALL;
							temp->r = r;
							temp->x = x;
							temp->y = y;
							temp->z = z;
							temp->next = NULL;
							add->next = temp;
							add = add->next;
							goto out;
						}
					}
		}
		printf("error3\n");
		break;
	out:
		continue;
	}

}

int main()
{
	circlesHead->next = NULL;
	ballsHead->next = NULL;

	produceBlock();

	clock_t start2, end2, start3, end3;
	start2 = clock();
	max2d();
	end2 = clock();

	CIRCLE *temp1 = circlesHead->next;
	double sum1 = 0;
	while (temp1 != NULL)
	{
		sum1 += temp1->r*temp1->r;
		temp1 = temp1->next;
	}
	printf("sum2(r^2):%lf\n", sum1);
	printf("time2:%.3f\n", (double)(end2 - start2) / CLOCKS_PER_SEC);
	system("pause");


	start3 = clock();
	max3d();
	end3 = clock();

	BALL *temp2 = ballsHead->next;
	double sum2 = 0;
	while (temp2 != NULL)
	{
		sum2 += temp2->r*temp2->r;
		temp2 = temp2->next;
	}
	printf("sum3(r^2):%lf\n", sum2);
	printf("time3:%.3f\n", (double)(end3 - start3) / CLOCKS_PER_SEC);
	system("pause");
}