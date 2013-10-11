#include "TinyString.h"
#include <cassert>
#include <cstdlib>
#include <cstring>

using namespace tinyString;

// String-----------------------------------------------------------------------------------
TinyString::TinyString()
{
	m_lpStringMemory = &TinyString::StringAlloc::m_emptyStringMemory;
}

TinyString::TinyString(const TinyString& value)
{
	m_lpStringMemory = value.m_lpStringMemory;
	++m_lpStringMemory->usedCount;
}

TinyString::TinyString(const wchar_t* lpString)
{
	m_lpStringMemory = TinyString::StringAlloc::alloc(lpString);
}

TinyString::~TinyString()
{
	release();
}

const TinyString& TinyString::operator=(const TinyString& value)
{
	if(m_lpStringMemory == value.m_lpStringMemory)
	{
		return *this;
	}

	release();
	m_lpStringMemory = value.m_lpStringMemory;
	++m_lpStringMemory->usedCount;
	return *this;
}

const TinyString& TinyString::operator=(const wchar_t* lpString)
{
	if(lpString == 0 || wcslen(lpString) == 0)
	{
		release();
		m_lpStringMemory = TinyString::StringAlloc::alloc(lpString);
		return *this;
	}

	if(m_lpStringMemory != &TinyString::StringAlloc::m_emptyStringMemory && 
		m_lpStringMemory != &TinyString::StringAlloc::m_nullStringMemory && 
		wcscmp(m_lpStringMemory->lpStr, lpString) == 0)
	{
		return *this;
	}

	release();
	m_lpStringMemory = TinyString::StringAlloc::alloc(lpString);
	return *this;
}

bool TinyString::operator==(const TinyString& value)
{
	return m_lpStringMemory == value.m_lpStringMemory;
}

bool TinyString::operator==(const wchar_t* lpString)
{
	return 
		(m_lpStringMemory == &TinyString::StringAlloc::m_nullStringMemory && lpString == 0) || 
		(m_lpStringMemory == &TinyString::StringAlloc::m_emptyStringMemory && wcslen(lpString) == 0) || 
		wcscmp(m_lpStringMemory->lpStr, lpString) == 0;
}

bool TinyString::isNull()
{
	return m_lpStringMemory == &TinyString::StringAlloc::m_nullStringMemory;
}

bool TinyString::isEmpty()
{
	return m_lpStringMemory == &TinyString::StringAlloc::m_emptyStringMemory;
}

bool TinyString::isNullOrEmpty()
{
	return isNull() || isEmpty();
}

void TinyString::release()
{
	assert(m_lpStringMemory != 0);
	if(m_lpStringMemory != &TinyString::StringAlloc::m_nullStringMemory && m_lpStringMemory != &TinyString::StringAlloc::m_emptyStringMemory)
	{
		--m_lpStringMemory->usedCount;
		assert(m_lpStringMemory->usedCount >= 0);
		if(m_lpStringMemory->usedCount == 0)
		{
			m_lpStringMemory->lpBelongToWhichStringPool->recycle(m_lpStringMemory);
		}
		m_lpStringMemory = 0;
	}
}

wchar_t* TinyString::wcsclone(const wchar_t* lpSrc, wchar_t* lpDest)
{
	if(lpSrc == 0)
	{
		return 0;
	}

	size_t strLen = wcslen(lpSrc) + 1/*'\0'*/;
	wchar_t* lpStrClone = 0;
	if(lpDest == 0)
	{
		assert(lpStrClone == 0);
		lpStrClone = (wchar_t*)malloc(strLen * sizeof(wchar_t));
		assert(lpStrClone != 0);
	}
	else
	{
		lpStrClone = lpDest;
	}
	if(lpSrc != lpStrClone)
	{
		assert(lpStrClone != 0);
		memcpy(lpStrClone, lpSrc, strLen * sizeof(wchar_t));
	}
	return lpStrClone;
}

// StringAlloc-----------------------------------------------------------------------------------
const unsigned int TinyString::StringAlloc::NUM_BLOCKS_PER_STRINGPOOL = 100;
TinyString::StringMemory TinyString::StringAlloc::m_nullStringMemory;
TinyString::StringMemory TinyString::StringAlloc::m_emptyStringMemory;
TinyString::StringAlloc::StringPoolMap TinyString::StringAlloc::m_stringPoolMap;

TinyString::StringMemory* TinyString::StringAlloc::alloc(const wchar_t* lpString)
{
	if(lpString == 0)
	{
		return &TinyString::StringAlloc::m_nullStringMemory;
	}
	else if(wcslen(lpString) == 0)
	{
		return &TinyString::StringAlloc::m_emptyStringMemory;
	}
	else
	{
		unsigned int length = wcslen(lpString);
		unsigned int fixedLength = getCeilPowerOf2(length);
		TinyString::StringAlloc::StringPoolMap::iterator findResult = m_stringPoolMap.find(fixedLength);
		if(findResult != m_stringPoolMap.end())
		{
			return findResult->second->pushString(lpString);
		}
		else
		{
			TinyString::StringPool* lpStringPool = 0;
			assert(lpStringPool == 0);
			lpStringPool = new TinyString::StringPool(NUM_BLOCKS_PER_STRINGPOOL, fixedLength);
			assert(lpStringPool != 0);
			m_stringPoolMap.insert(TinyString::StringAlloc::StringPoolMap::value_type(fixedLength, lpStringPool));
			return lpStringPool->pushString(lpString);
		}
	}
}

unsigned int TinyString::StringAlloc::getCeilPowerOf2(unsigned int value)
{
	unsigned int v = 1;
	while(true)
	{
		if(v >= value)
		{
			return v;	
		}
		v *= 2;
	}
	assert(false);
	return 0;
}

// IndexStack-----------------------------------------------------------------------------------
TinyString::IndexStack::IndexStack(unsigned int capability):
	m_lpIndexList(0), m_position(0), m_capability(capability)
{
	assert(m_lpIndexList == 0);
	m_lpIndexList = new unsigned int[capability];
	assert(m_lpIndexList != 0);
}

TinyString::IndexStack::~IndexStack()
{
	if(m_lpIndexList != 0)
	{
		delete[] m_lpIndexList;
		m_lpIndexList = 0;
	}
}

bool TinyString::IndexStack::push(unsigned int index)
{
	if(m_position >= m_capability)
	{
		return false;
	}
	else
	{
		m_lpIndexList[m_position++] = index;
		return true;
	}
}

bool TinyString::IndexStack::pop(unsigned int* index)
{
	if(m_position == 0)
	{
		return false;
	}
	else
	{
		if(index != 0)
		{
			*index = m_lpIndexList[m_position--];
		}
		return true;
	}
}

// StringPool-----------------------------------------------------------------------------------
TinyString::StringPool::StringPool(unsigned int numBlocks, unsigned int blockChars):
	m_numBlocks(numBlocks), m_blockChars(blockChars), m_lpNextStringPool(0)
{
	assert(m_numBlocks > 0);
	assert(m_blockChars > 0);
	m_lpStringMemoryList = (TinyString::StringMemory*)malloc(numBlocks * (sizeof(TinyString::StringMemory) + sizeof(wchar_t)* blockChars));
	assert(m_lpFreeIndexStack == 0);
	m_lpFreeIndexStack = new TinyString::IndexStack(numBlocks);
	assert(m_lpFreeIndexStack != 0);

	for(unsigned int i = 0; i < numBlocks; ++i)
	{
		TinyString::StringMemory* lpStringMemory = &m_lpStringMemoryList[i];
		lpStringMemory->lpBelongToWhichStringPool = this;
		lpStringMemory->usedCount = 0;
		lpStringMemory->indexInStringPool = i;
		lpStringMemory->lpStr = (wchar_t*)(lpStringMemory + sizeof(TinyString::StringMemory));
		m_lpFreeIndexStack->push(i);
	}
}

TinyString::StringPool::~StringPool()
{
	if(m_lpStringMemoryList != 0)
	{
		free(m_lpStringMemoryList);
		m_lpStringMemoryList = 0;
	}
	if(m_lpFreeIndexStack != 0)
	{
		delete m_lpFreeIndexStack;
		m_lpFreeIndexStack = 0;
	}

	TinyString::StringPool* lpStringPoolNext = m_lpNextStringPool;
	while(lpStringPoolNext != 0)
	{
		TinyString::StringPool* lpStringPoolNextNext = lpStringPoolNext->m_lpNextStringPool;
		if(lpStringPoolNext != 0)
		{
			delete lpStringPoolNext;
			lpStringPoolNext = 0;
		}
		lpStringPoolNext = lpStringPoolNextNext;
	}
}

TinyString::StringMemory* TinyString::StringPool::pushString(const wchar_t* lpString)
{
	TinyString::StringPool* lpStringPoolPrev = 0;
	TinyString::StringPool* lpStringPoolThis = this;
	TinyString::StringMemory* lpStringMemory = 0;
	while(true)
	{
		if(lpStringPoolThis == 0)
		{
			assert(lpStringPoolThis == 0);
			lpStringPoolThis = new TinyString::StringPool(m_numBlocks, m_blockChars);
			assert(lpStringPoolThis != 0);
			assert(lpStringPoolPrev != 0);
			lpStringPoolPrev->m_lpNextStringPool = lpStringPoolThis;
			lpStringPoolThis->pushString(lpString);
			break;
		}
		else
		{
			unsigned int index;
			if(m_lpFreeIndexStack->pop(&index))
			{
				TinyString::wcsclone(lpString, m_lpStringMemoryList[index].lpStr);
				lpStringMemory = &m_lpStringMemoryList[index];
				break;
			}
			else
			{
				lpStringPoolPrev = lpStringPoolThis;
				lpStringPoolThis = m_lpNextStringPool;
			}
		}
	}
	return lpStringMemory;
}

void TinyString::StringPool::recycle(TinyString::StringMemory* lpStringMemory)
{
	assert(lpStringMemory != 0);
	assert(lpStringMemory->usedCount == 0);
	m_lpFreeIndexStack->push(lpStringMemory->indexInStringPool);
}

// StringMemory-----------------------------------------------------------------------------------
TinyString::StringMemory::StringMemory():
	lpBelongToWhichStringPool(0), usedCount(0), indexInStringPool(0), lpStr(0)
{
	// Do nothing
}