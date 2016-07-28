#include <cxcore.h>
#include <cv.h>
#include <highgui.h>
# include <iostream>
#include <math.h>
#include "header.h"
#include <stdio.h>

# define NULL 0

static int nFrmNum = 0;
static int label_num = 0;

void detect_object(IplImage* , IplImage* , IplImage* , CvMat* , CvMat*, CvMat*,int);
void frame_dif(IplImage*, IplImage*, IplImage*, IplImage*, IplImage*, IplImage*,int);
void computeObject(CvMat* , int , int, CvMat* , movingObject* ,int);
int cal_dist(int x1, int y1, int x2, int y2);
void release_link(movingObject*,movingObject*);
void detect_hiding(movingObject* prev_head, int width, movingObject* curr_head,int value);

int main()
{
	/*�������ļ�*/
	FILE* fp;
	char readin[100],file[100];
	int value1=0, value2=0, value3=0,value4=0,value5=0;
	fopen_s(&fp, "configure/configure.txt", "r");
	fscanf_s(fp, "video: %s", readin, sizeof(readin));
	sprintf(file, readin);
	memset(readin, 0, sizeof(readin));
	fgets(readin, sizeof(readin),fp);
	fscanf_s(fp, "value1: %d", &value1, sizeof(int));
	memset(readin, 0, sizeof(readin));
	fgets(readin, sizeof(readin), fp);
	fscanf_s(fp, "value2: %d", &value2, sizeof(int));
	memset(readin, 0, sizeof(readin));
	fgets(readin, sizeof(readin), fp);;
	fscanf_s(fp, "value3: %d", &value3, sizeof(int));
	memset(readin, 0, sizeof(readin));
	fgets(readin, sizeof(readin), fp);
	fscanf_s(fp, "value4: %d", &value4, sizeof(int));
	memset(readin, 0, sizeof(readin));
	fgets(readin, sizeof(readin), fp);
	fscanf_s(fp, "value5: %d", &value5, sizeof(int));
	printf("%s %d %d %d\n", file,value1,value2,value3);


	/*����IplImageָ��*/
	IplImage *image0 = NULL;		//ԭʼ֡
	IplImage *image = NULL;			//��ǰ֡
	IplImage *image_pass = NULL;	//��һ֡
	IplImage *res = NULL;			//֡��
	IplImage *res0 = NULL;			//֡��
	IplImage *pFrame = NULL;
	IplImage *pFrImg = NULL;
	IplImage *pBkImg = NULL;

	/*����CvMatָ��*/
	CvMat* pFrameMat = NULL;
	CvMat* pFrMat = NULL;
	CvMat* pBkMat = NULL;
	CvMat* IndexMat = NULL;


	/*����caputerָ��*/
	CvCapture *capture = NULL;
	capture = cvCaptureFromFile(file);
	image0 = cvQueryFrame(capture);
	nFrmNum++;

	// ��������
	//cvNamedWindow("video", 1);
	cvNamedWindow("background", 1);
	cvNamedWindow("tracking", 1);

	// ���д���
	//cvMoveWindow("video", 30, 80);
	cvMoveWindow("background",10, 100);
	cvMoveWindow("tracking", 660, 100);

	/*�ƶ�����Ķ���*/
	movingObject *curr_head = new movingObject();
	movingObject *prev_head = new  movingObject();
	movingObject *p_obj = new movingObject();
	movingObject *share_head = new movingObject();
	curr_head->next = NULL;
	curr_head->share_next = NULL;
	prev_head->next = NULL;
	prev_head->share_next = NULL;
	share_head->next = NULL;

	CvScalar color[7] = { { 0, 0, 0 }, { 0, 255, 0 }, {255, 0, 0 }, { 255, 201, 14 }, { 255, 0, 255 }, { 0, 166, 0 }, {121,255,121} };

	image = cvCreateImage(cvSize(640,360), IPL_DEPTH_8U, 3);
	cvResize(image0, image, CV_INTER_LINEAR);

	int image_width = image->width;
	int image_height = image->height;

	image_pass = cvCreateImage(cvSize(image_width, image_height), IPL_DEPTH_8U, 3);
	res = cvCreateImage(cvSize(image_width, image_height), IPL_DEPTH_8U, 1);
	res0 = cvCreateImage(cvSize(image_width, image_height), IPL_DEPTH_8U, 3);
	pBkImg = cvCreateImage(cvSize(image_width, image_height), IPL_DEPTH_8U, 1);
	pFrImg = cvCreateImage(cvSize(image_width, image_height), IPL_DEPTH_8U, 1);
	pFrame = cvCreateImage(cvSize(image_width, image_height), IPL_DEPTH_8U, 1);

	pBkMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
	pFrMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
	pFrameMat = cvCreateMat(pFrame->height, pFrame->width, CV_32FC1);
	IndexMat = cvCreateMat(image_height, image_width, CV_32FC1);

	cvCopy(image, image_pass, NULL);


	/*����*/
	cvCvtColor(image, pBkImg, CV_BGR2GRAY);
	cvConvert(pBkImg, pBkMat);

	while (1)
	{
		image0 = cvQueryFrame(capture);
		if (!image0) break;	

		cvResize(image0, image, CV_INTER_LINEAR);


		/*��˹ƽ��*/
		cvSmooth(image, image, CV_GAUSSIAN, 3, 0, 0);

		/*�˶�Ŀ����*/
		detect_object(image, pBkImg, pFrImg, pFrameMat, pBkMat, pFrMat,value1);

		/*֡��Ż�*/
		frame_dif(image, image_pass, res,res0, pFrImg,pFrame,value2);

		cvConvert(pFrame, pFrMat);

		/*������ͨ���������*/
		computeObject(pFrMat, image_width, image_height,IndexMat, curr_head,value3);

		/*��������*/
		p_obj = curr_head;
		while (p_obj->next != NULL)
		{
			p_obj = p_obj->next;
			cvRectangle(image, cvPoint(p_obj->x - 1, p_obj->y - 1), cvPoint(p_obj->x + 1, p_obj->y + 1), cvScalar(0, 0, 255), 2, 8, 0);
		}


		if (nFrmNum == 2)
		{
			movingObject* q = NULL;
			computeObject(pFrMat, image_width, image_height, IndexMat, prev_head,value3);

		}
		/*�������ٿ�*/
		if (nFrmNum > 2)
		{
			movingObject *q = NULL, *p = NULL;
			movingObject *last=curr_head;

			for (q = curr_head->next; q; )
			{
				int close = 0;
 				share_head->share_next = NULL;
  				for (p = prev_head->next; p; p = p->next)
				{
					int dist = cal_dist(p->x, p->y, q->x, q->y);
					if (dist <= value5)
					{
						close++;
						p->share_next = share_head->share_next; 
						share_head->share_next= p;
					}

				}

				if (close == 1)
				{
					if (share_head->share_next->track == 1)	//�ѱ���,ɾ����ǰ���
					{
						last->next = q->next;
						movingObject* t=q;
						q = q->next;
						delete t;
						continue;
					}
					q->label = (share_head->share_next)->label;
					cvRectangle(image, q->points[0], q->points[1], color[q->label],2);
					q->move = q->x - (share_head->share_next)->x;	//��֡λ��
					share_head->share_next->track = 1;
					q->track = 0; 
					q->keeptime = 0;
					last = q;
					q = q->next;
				}
				else if (close==0)
				{
					//�����±�ǩ
					q->label = ++label_num;
					cvRectangle(image, q->points[0], q->points[1], color[q->label],2);
					q->track = 0;
					q->keeptime = 0;
					last = q;
					q = q->next;
				}
				else if (close>1)
				{
					movingObject* t = share_head->share_next;
					while ( t != NULL)
					{
						if (t->track==1)
						t = t->share_next;
						else break;
					}
					if (t==NULL)	//ȫ���������
					{
						last->next = q->next;
						t = q;
						q = q->next;
						delete t;
						continue;
					}

					//���õ�ǰ���е���һobject
					q->label = t->label;
					q->move = t->move;
					q->area = t->area;
					q->x = t->x + t->move;
					q->y = t->y;
					q->points[0].x = t->points[0].x+t->move;
					q->points[0].y = t->points[0].y;
					q->points[1].x = t->points[1].x + t->move;
					q->points[1].y = t->points[1].y;
					q->track = 0;
					t->track = 1;
					q->keeptime = 0;
					cvRectangle(image, q->points[0], q->points[1], color[q->label],2);

					t = t->share_next;
					while (t)
					{
						if (t->track == 1)
						{
							t = t->share_next;
							continue;
						}
						movingObject* newobject = new movingObject();
						newobject->area = t->area;
						newobject->label = t->label;
						newobject->move = t->move;
						newobject->next = q->next;
						q->next = newobject;
						q = newobject;
						newobject->points[0].x = t->points[0].x + t->move;
						newobject->points[0].y = t->points[0].y;
						newobject->points[1].x = t->points[1].x + t->move;
						newobject->points[1].y = t->points[1].y;
						newobject->x = t->x + t->move;
						newobject->y = t->y;
						newobject->track = 0;
						newobject->keeptime = 0;
						cvRectangle(image, newobject->points[0], newobject->points[1], color[newobject->label],2);
						t->track = 1;			//�Ѹ�����һ��prevĿ��
						t = t->share_next;
					}
					last = q;
					q = q->next;

				}

			}//end for
			detect_hiding(prev_head, image_width, curr_head,value4);
		}//end if
		
		cvShowImage("tracking", image);
		release_link(prev_head,share_head);
		prev_head->next = curr_head->next;
		curr_head->next = NULL;
		
		int ctrl;
		if ( (ctrl=cvWaitKey(20)) == 27)//ESC�˳�
		{
			break;
		}
		else if (ctrl == 32)	//�ո���ͣ
		{
			while ((ctrl=cvWaitKey(0))!=13)//�س�����
			{
				if (ctrl == 27)
				exit(0);
				continue;
			}

			
		}
	}

	cvReleaseImage(&res);
	cvReleaseImage(&res0);
	cvReleaseImage(&image_pass);
	cvReleaseImage(&pBkImg);
	cvReleaseImage(&pFrImg);
	cvReleaseImage(&pFrame);
	cvReleaseCapture(&capture);

	return 1;
}

void release_link(movingObject* head, movingObject* share_head)
{
	movingObject* p=head->next;
	movingObject* q = NULL;
	while (p!=NULL)
	{
		q = p;
		p = p->next;
		q->share_next = NULL;
		delete q;
	}
	head->next = NULL;
	head->share_next = NULL;
	share_head->share_next = NULL;
}

void frame_dif(IplImage* image, IplImage* image_pass, IplImage* res,IplImage* res0, IplImage* pFrImg,IplImage* pFrame,int thre_limit)
{
	cvZero(pFrame);

	cvAbsDiff(image, image_pass, res0);
	cvCvtColor(res0, res, CV_RGB2GRAY);
	cvThreshold(res, res, thre_limit, 255, CV_THRESH_BINARY);
	unsigned char data1, data2, data;
	
	int i, j;
	int width = pFrame->width;
	int height = pFrame->height;
	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
		{
			data1 = (unsigned char)res->imageData[i * width + j];
			data2 = (unsigned char)pFrImg->imageData[i * width + j];
			if (data1 == 255 || data2 == 255)
			{
				data = 255;
				pFrame->imageData[i * width + j] = (char)data;
			}
		}

	cvCopy(image, image_pass, NULL);

}

void detect_object(IplImage *image, IplImage *pBkImg, IplImage *pFrImg, CvMat *pFrameMat, CvMat *pBkMat, CvMat *pFrMat,int thre_limit)
{
	nFrmNum++;
	cvCvtColor(image, pFrImg, CV_BGR2GRAY);
	cvConvert(pFrImg, pFrameMat);
	//��˹�˲�
	cvSmooth(pFrameMat, pFrameMat, CV_GAUSSIAN, 3, 0, 0);
	//��ǰ֡��ȥ����ͼ��ȡ����ֵ
	cvAbsDiff(pFrameMat, pBkMat, pFrMat);
	//��ֵ��ǰ��ͼ��
	cvThreshold(pFrMat, pFrImg,thre_limit, 255.0, CV_THRESH_BINARY);

	/*��̬ѧ�˲�*/
	//IplConvKernel* element = cvCreateStructuringElementEx(2, 2, 0, 0, CV_SHAPE_RECT);
	//cvErode(pFrImg, pFrImg,element, 1);	// ��ʴ
	//delete element;

	//element = cvCreateStructuringElementEx(2, 2, 1, 1, CV_SHAPE_RECT);
	//cvDilate(pFrImg, pFrImg, element, 1);	//����
	//delete element;
	cvErode(pFrImg, pFrImg,0, 1);	// ��ʴ
	cvDilate(pFrImg, pFrImg,0, 1);	//����

	//����ƽ�����±�������ƽ����
	cvRunningAvg(pFrameMat, pBkMat, 0.004, 0);
	//����������ת��Ϊͼ���ʽ��������ʾ
	cvConvert(pBkMat, pBkImg);

	cvShowImage("background", pFrImg);
//	cvShowImage("background", pBkImg);
}

void computeObject(CvMat* foreImageMat, int width, int height, CvMat* IndexMat, movingObject* head,int area_limit)
{
	int neighbor[8][2] = { { -1, -1 }, { -1, 0 }, { -1, 1 }, { 0, -1 }, { 0, 1 }, { 1, -1 }, { 1, 0 }, { 1, 1 } };
	int index = 0, row, col, tRow, tCol;
	CvPoint *points = new CvPoint[width * height];
	int i, j, top = 0;
	cvSetZero(IndexMat);

	/************�����ͨ����**************/
	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
		{
			if (cvmGet(foreImageMat, i, j) == 255 && cvmGet(IndexMat, i, j) == 0)
			{
				index++;
				points[top].x = j;
				points[top].y = i;
				top++;
				while (top > 0)
				{
					top--;
					row = points[top].y;
					col = points[top].x;
					cvmSet(IndexMat, row, col, index);
					for (int m = 0; m < 8; m++)
					{
						tRow = row + neighbor[m][0];
						tCol = col + neighbor[m][1];
						if (tRow >= 0 && tRow < height && tCol >= 0 && tCol < width)
							if (cvmGet(foreImageMat, tRow, tCol) == 255 && cvmGet(IndexMat, tRow, tCol) == 0)
							{
								points[top].x = tCol;
								points[top].y = tRow;
								top++;
							}
					}
				}
			}

		}

	/*************ͳ���˶�Ŀ��*************/
	int *pmArr = new int[index];			//�������������
	int *mx0Arr = new int[index];			//x�ܺ�
	int *my0Arr = new int[index];			//y�ܺ�
	int value;					
	CvPoint * rect_points = new CvPoint[index * 2];	//����

	for (i = 0; i < index; i++)
	{
		pmArr[i] = 0;
		mx0Arr[i] = 0;
		my0Arr[i] = 0;

		rect_points[i].x = width;      //��¼�������½ǵ�
		rect_points[i].y = height;
		rect_points[i + index].x = 0;  //��¼�������Ͻǵ�
		rect_points[i + index].y = 0;
	}

	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
		{
			value = (int)cvmGet(IndexMat, i, j);
			if (value != 0)
			{
				pmArr[value - 1] ++;   
				mx0Arr[value - 1] += j;
				my0Arr[value - 1] += i; 

				if (i < rect_points[value - 1].y)
					rect_points[value - 1].y = i;
				if (j < rect_points[value - 1].x)
					rect_points[value - 1].x = j;

				if (i > rect_points[value - 1 + index].y)
					rect_points[value - 1 + index].y = i;
				if (j > rect_points[value - 1 + index].x)
					rect_points[value - 1 + index].x = j;
			}
		}

	/**********�������ģ����˶�Ŀ�괮������***********/
	movingObject *p = head;
	movingObject *t = NULL;
	p->next = NULL;


	for (i = 0; i < index; i++)  
	{
		if (pmArr[i] > area_limit)   //Ŀ������ص��������300 �Ž���ͳ�ƣ��������Ժ��Ե�һЩ���ŵĵ�
		{
			t = new movingObject();
			t->area = pmArr[i]; 
			t->x = mx0Arr[i] / pmArr[i];  
			t->y = my0Arr[i] / pmArr[i];
			t->next = NULL;

			//�����Ӿ���,�ʵ�����һ����ο�
			t->points[0].x = rect_points[i].x-5;
			t->points[0].y = rect_points[i].y - 5;
			t->points[1].x = rect_points[i + index].x+5;	
			t->points[1].y = rect_points[i + index].y + 5;

			p->next = t;		 //��Ŀ�괮������
			p = p->next;		//null
		}

	}

	delete[] pmArr;
	delete[] mx0Arr;
	delete[] my0Arr;
	delete[] points;
	delete[] rect_points;

}

int cal_dist(int x1, int y1, int x2, int y2)
{
	int distance;
	double res;
	res = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
	distance = (int)(sqrt(res));
	return distance;
}

void detect_hiding(movingObject* prev_head,int width,movingObject* curr_head,int time_limit)
{
	movingObject* temp = prev_head->next;
	movingObject* temp_last = prev_head;
	while (temp)
	{
		if (temp->track == 0)
		{
			int dis = temp->x + temp->move;;
			if (dis >10 &&dis < width-10)//��Ϊ����δ�ӻ����뿪
			{
				if (temp->keeptime <= time_limit)		//����time_limit֡û�г���,��Ϊ��ʧ,�����뵱ǰ����
				{
					temp->x = temp->x + temp->move;
					temp_last->next = temp->next;
					temp_last = temp;
					temp->next = curr_head->next;
					curr_head->next = temp;
					temp->keeptime += 1;
					temp = temp_last->next;
					continue;
				}
			}
		}
		temp_last = temp;
		temp = temp->next;

	}



}


