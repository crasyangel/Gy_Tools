#ifndef _GY_BISORTT_H_
#define _GY_BISORTT_H_

#ifdef __cplusplus
extern "C" {
#endif

//********************** Include Files ***************************************
#include "Gy_Common.h"

//********************** struct typedef *************************************

/*����������Ķ���������ṹ���� */
typedef  struct BiTNode    /* ���ṹ */
{
    INT32_T data[2];    /* ������� */
    struct BiTNode *lchild, *rchild; /* ���Һ���ָ�� */
} BiTNode, *BiTree;


//********************** Global Functions *************************************


//���������ӡ��,��ӡ�����Ľ����ź���Ľ��
void DisplayBST(const BiTree T);

//�������ɾ����
void DeleteBST(BiTree *T);

//�ڶ����������в���ĳ��ֵ�Ƿ���ĳ��������
bool SearchBST_Key(const BiTree T, const INT32_T key);

//����QAMname������ɵĶ���������
//�����buffer��������[[23001,23010],[23027,23036],[24821,24830],[24847,24856]]
bool CreateQamNameBiTree(BiTree *T, CHAR_T *rangebuf);


#ifdef __cplusplus
}
#endif

#endif  /*_GY_BISORTT_H_*/

