/*header.h*/

struct objectList
{
	int x;
	int y;
	struct objectList *next;
};

struct movingObject
{
	int area;
	int label;
	int x;
	int y;
	int move;
	int track;
	int keeptime;

	CvPoint points[2];      //���ο�������Խŵ�
//	struct objectList *head;
	struct movingObject *share_next;
	struct movingObject *next;
};

