#pragma once

//���Ͷ� ����� �ͽ�ü���� == ���ٸ� 1, �ٸ��ٸ� 0
#include "CMemoryPoolTLS\CMemoryPool.h"

template<class DATA>
class LockFreeStack
{
public:
	__declspec(align(16))struct INT128
	{
		UINT64 nodePtr;
		UINT64 count;
	};
	struct stNODE
	{
		stNODE()
		{
			nextNodePtr = nullptr;
		}
		DATA data;
		stNODE* nextNodePtr;
	};

	//topNodePtr = ������, count = ī��Ʈ
	INT128 topNode;
	stNODE* dummyNode;
	//��带 �����ϴ� �޸� Ǯ
	CMemoryPool<stNODE> memoryPool;
public:
	LockFreeStack()
		:topNode({ 0, 0 })
		, memoryPool(1, false)
	{
		dummyNode = new stNODE;
		topNode.nodePtr = (UINT64)dummyNode;
		topNode.count = 0;
	}

	void Push(DATA data)
	{
		stNODE* pNewNode = memoryPool.Alloc();
		pNewNode->data = data;

		INT128 tempNode;
		do
		{
			tempNode.count = topNode.count;
			tempNode.nodePtr = topNode.nodePtr;
			pNewNode->nextNodePtr = (stNODE*)tempNode.nodePtr;
		} while ((long long)tempNode.nodePtr != InterlockedCompareExchange64((long long*)&topNode.nodePtr, (long long)pNewNode, (long long)tempNode.nodePtr));
	}
	bool Pop(DATA* pData)
	{
		
		INT128 tempArray;
		INT128 tempNewTop;
		do
		{
			tempArray.count = topNode.count;
			tempArray.nodePtr = topNode.nodePtr;
			if (((stNODE*)tempArray.nodePtr) == dummyNode)
			{
				return false;
			}
		} while (!InterlockedCompareExchange128((long long*)&topNode, tempArray.count + 1, (UINT64)(((stNODE*)tempArray.nodePtr)->nextNodePtr), (long long*)&tempArray));

		*pData = ((stNODE*)tempArray.nodePtr)->data;

		memoryPool.Free((stNODE*)tempArray.nodePtr);

		return true;
	}
};