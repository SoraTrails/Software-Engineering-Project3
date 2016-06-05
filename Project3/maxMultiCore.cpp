#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include<time.h>
#include"resource.h"

#define M 50
#define BLOCKNUM 4
#define SIZE 500	//(-SIZE, SIZE)
#define CORENUM 4

#define TRANS(s) (int)(s * SIZE)
	
typedef struct _PRARM{
	int *presentNum;
	int quadrant;
	int r;
}*PPRARM;

typedef struct _circle {
	int x;
	int y;
	int r;
	_circle *next;
}CIRCLE;

//typedef struct _ball {
//	double x;
//	double y;
//	double z;
//	double r;
//	_ball *next;
//}BALL;

typedef struct _block2
{
	int x;
	int y;
}BLOCK2;

//typedef struct _block3
//{
//	double x;
//	double y;
//	double z;
//}BLOCK3;

CIRCLE* circlesHead = new CIRCLE, *circlesTail = circlesHead;
//BALL* ballsHead = new BALL, *ballsTail = ballsHead;

BLOCK2 blocks2[BLOCKNUM];
//BLOCK3 blocks3[BLOCKNUM];

CRITICAL_SECTION mutex2;

void produceBlock()
{
	srand(time(NULL));
	//test1:
	//blocks2[0].x = TRANS(0.5);
	//blocks2[0].y = TRANS(0);
	//blocks2[1].x = TRANS(-0.5);
	//blocks2[1].y = TRANS(0);
	//blocks2[2].x = TRANS(0);
	//blocks2[2].y = TRANS(-0.5);
	//blocks2[3].x = TRANS(0);
	//blocks2[3].y = TRANS(0.5);

	//test2:
	blocks2[0].x = 0;
	blocks2[0].y = 0;
	blocks2[1].x = TRANS(-0.75);
	blocks2[1].y = TRANS(0.75);
	blocks2[2].x = TRANS(0.75);
	blocks2[2].y = TRANS(-0.75);
	blocks2[3].x = TRANS(0.75);
	blocks2[3].y = TRANS(0.75);

	for (int i = 0;i < BLOCKNUM;i++)
	{
		blocks2[i].x = rand() % (2 * SIZE - 1) - SIZE + 1;
		blocks2[i].y = rand() % (2 * SIZE - 1) - SIZE + 1;
	//	//blocks3[i].x = (double)((double)rand() / RAND_MAX) * 2 - 1;
	//	//blocks3[i].y = (double)((double)rand() / RAND_MAX) * 2 - 1;
	//	//blocks3[i].z = (double)((double)rand() / RAND_MAX) * 2 - 1;
	}
}

bool ifOverlap2(int x, int y, int r)
{
	for (int i = 0;i < BLOCKNUM;i++)
		if (r * r > (blocks2[i].x - x)*(blocks2[i].x - x) + (blocks2[i].y - y)*(blocks2[i].y - y))
			return false;

//  	EnterCriticalSection(&mutex2);

	CIRCLE *temp = circlesHead->next;
	while (temp != NULL)
	{
		if ((temp->r + r) * (temp->r + r) > (temp->x - x)*(temp->x - x) + (temp->y - y)*(temp->y - y))
		{
	//		LeaveCriticalSection(&mutex2);
			return false;
		}
		temp = temp->next;
	}

	//LeaveCriticalSection(&mutex2);
	return true;
}
//bool ifOverlap3(double x, double y, double z, double r)
//{
//	//if (x + r > 1 || x - r < -1 || y + r > 1 || y - r < -1 || z + r > 1 || z - r < -1)
//	//	return false;
//	BALL *temp = ballsHead->next;
//
//	for (int i = 0;i < BLOCKNUM;i++)
//		if (r * r > (blocks3[i].x - x)*(blocks3[i].x - x) + (blocks3[i].y - y)*(blocks3[i].y - y) +
//			(blocks3[i].z - z)*(blocks3[i].z - z))
//			return false;
//
//	while (temp != NULL)
//	{
//		if ((temp->r + r) * (temp->r + r) > (temp->x - x)*(temp->x - x) + (temp->y - y)*(temp->y - y) + (temp->z - z)*(temp->z - z))
//			return false;
//		temp = temp->next;
//	}
//	return true;
//}


DWORD WINAPI max2dThread(LPVOID lpParam)
{
	PPRARM p = (PPRARM)lpParam;
	int *presentNum = p->presentNum;
	int quadrant = p->quadrant;
	int r = p->r;

	int lBoder, rBoder, uBoder, dBoder;

	if (quadrant <= 2)	{				//判断各象限的边界
		uBoder = SIZE - r;
		dBoder = 0;
	}
	else {
		uBoder = 0;
		dBoder = (-1)*SIZE + r + 1;
	}

	if (quadrant >= 2 && quadrant <= 3)	{
		rBoder = 0;
		lBoder = (-1)*SIZE + r + 1;
	}
	else {
		rBoder = SIZE - r;
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
				if (r < SIZE / 2)
					y = y + 2 * r - 1;
				else
					return 1;
			}
		}
	return 0;
}

int FindMaxR() {
	int r = SIZE;
	int x, y;
	bool flag = true;
	r++;
	while (flag) {
		r--;
		x = -SIZE + r;
		y = -SIZE + r;
		flag = false;
		for (int i = 0; i <= BLOCKNUM; i++) {
			if (r*r > (blocks2[i].x - x)*(blocks2[i].x - x) + (blocks2[i].y - y)*(blocks2[i].y - y))
			{
				flag = true;
				break;
			}
		}
	}
	return r;
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

	for (int r = SIZE;/*FindMaxR()*/r > 0;r--)
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

//void max3d()
//{
//	BALL *add = ballsHead, *temp;
//
//	for (int i = 0;i < M;i++)
//	{
//		for (double r = 1 - 1e-6;r > 0;r -= SIZE)
//		{
//			for (double x = -1.0 + r;x < 1.0 - r;x += SIZE)
//				for (double y = -1.0 + r;y < 1.0 - r;y += SIZE)
//					for (double z = -1.0 + r;z <1.0 - r;z += SIZE)
//					{
//						if (ifOverlap3(x, y, z, r))
//						{
//							temp = new BALL;
//							temp->r = r;
//							temp->x = x;
//							temp->y = y;
//							temp->z = z;
//							temp->next = NULL;
//							add->next = temp;
//							add = add->next;
//							goto out;
//						}
//					}
//		}
//		printf("error3\n");
//		break;
//	out:
//		continue;
//	}
//
//}

BOOL CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
	HDC hdc = GetDC(hDlg);
	CIRCLE *temp = circlesHead->next;
	HBRUSH brush;
	int x, y, r;
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			Rectangle(hdc, 0, 0, 500, 500);
			brush = CreateSolidBrush(RGB(0, 255, 0));
			SelectObject(hdc, brush);
			while (temp != NULL)
			{
				x = (temp->x + SIZE) / (SIZE / 500 * 2);
				y = (temp->y + SIZE) / (SIZE / 500 * 2);
				r = temp->r / (SIZE / 500 * 2);
				Ellipse(hdc, x - r, y + r, x + r, y - r);
				temp = temp->next;
			}

			brush = CreateSolidBrush(RGB(255, 0, 0));
			SelectObject(hdc, brush);
			for (int i = 0;i < BLOCKNUM;i++)
			{
				x = (blocks2[i].x + SIZE) / (SIZE / 500 * 2);
				y = (blocks2[i].y + SIZE) / (SIZE / 500 * 2);
				Ellipse(hdc, x - 2, y + 2, x + 2, y - 2);
			}
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;

}

int main()
{
	circlesHead->next = NULL;
//	ballsHead->next = NULL;

	produceBlock();

	clock_t start2, end2;
	start2 = clock();
	max2d();
	end2 = clock();

	CIRCLE *temp1 = circlesHead->next;
	double sum1 = 0;
	while (temp1 != NULL)
	{
		sum1 += ((double)(temp1->r)/SIZE)*((double)(temp1->r) / SIZE);
		temp1 = temp1->next;
	}
	printf("sum2(r^2):%lf\n", sum1);
	printf("time2:%.3f\n", (double)(end2 - start2) / CLOCKS_PER_SEC);

	HINSTANCE hModule = GetModuleHandle(NULL);
	DialogBox(hModule, MAKEINTRESOURCEA(IDD_DIALOG1), NULL, DialogProc);
	system("pause");

	//clock_t start3, end3;
	//start3 = clock();
	//max3d();
	//end3 = clock();

	//BALL *temp2 = ballsHead->next;
	//double sum2 = 0;
	//while (temp2 != NULL)
	//{
	//	sum2 += temp2->r*temp2->r;
	//	temp2 = temp2->next;
	//}
	//printf("sum3(r^2):%lf\n", sum2);
	//printf("time3:%.3f\n", (double)(end3 - start3) / CLOCKS_PER_SEC);
	//system("pause");
}
