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
	if(lpString == null)
	{
		release();
		m_lpStringMemory = &TinyString::StringAlloc::m_nullStringMemory;
		return *this;
	}

	if(wcslen(lpString) == 0)
	{
		release();
		m_lpStringMemory = &TinyString::StringAlloc::m_emptyStringMemory;
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
		(m_lpStringMemory == &TinyString::StringAlloc::m_nullStringMemory && lpString == null) || 
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

uint TinyString::length()
{
	return m_lpStringMemory->length;
}

void TinyString::release()
{
	assert(m_lpStringMemory != null);
	if(m_lpStringMemory != &TinyString::StringAlloc::m_nullStringMemory && m_lpStringMemory != &TinyString::StringAlloc::m_emptyStringMemory)
	{
		--m_lpStringMemory->usedCount;
		assert(m_lpStringMemory->usedCount >= 0);
		if(m_lpStringMemory->usedCount == 0)
		{
			m_lpStringMemory->lpBelongToWhichStringPool->recycle(m_lpStringMemory);
		}
		m_lpStringMemory = null;
	}
}

// StringAlloc-----------------------------------------------------------------------------------
const uint TinyString::StringAlloc::NUM_BLOCKS_PER_STRINGPOOL = 100;
TinyString::StringMemory TinyString::StringAlloc::m_nullStringMemory;
TinyString::StringMemory TinyString::StringAlloc::m_emptyStringMemory;
TinyString::StringAlloc::StringPoolMap TinyString::StringAlloc::m_stringPoolMap;

TinyString::StringMemory* TinyString::StringAlloc::alloc(const wchar_t* lpString)
{
	if(lpString == null)
	{
		return &TinyString::StringAlloc::m_nullStringMemory;
	}

	uint length = wcslen(lpString);
	if(length == 0)
	{
		return &TinyString::StringAlloc::m_emptyStringMemory;
	}
	else
	{
		uint fixedLength = getCeilPowerOf2(length + 1/*'\0'*/);
		TinyString::StringAlloc::StringPoolMap::iterator findResult = m_stringPoolMap.find(fixedLength);
		if(findResult != m_stringPoolMap.end())
		{
			return findResult->second->pushString(lpString);
		}
		else
		{
			TinyString::StringPool* lpStringPool = null;
			lpStringPool = new TinyString::StringPool(NUM_BLOCKS_PER_STRINGPOOL, fixedLength);
			assert(lpStringPool != null);
			m_stringPoolMap.insert(TinyString::StringAlloc::StringPoolMap::value_type(fixedLength, lpStringPool));
			return lpStringPool->pushString(lpString);
		}
	}
}

uint TinyString::StringAlloc::getCeilPowerOf2(uint value)
{
	uint v = 1;
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
TinyString::IndexStack::IndexStack(uint capability):
	m_lpIndexList(0), m_position(0), m_capability(capability)
{
	assert(m_lpIndexList == null);
	m_lpIndexList = new uint[capability];
	assert(m_lpIndexList != 0);
}

TinyString::IndexStack::~IndexStack()
{
	if(m_lpIndexList != null)
	{
		delete[] m_lpIndexList;
		m_lpIndexList = null;
	}
}

bool TinyString::IndexStack::push(uint index)
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

bool TinyString::IndexStack::pop(uint* index)
{
	if(m_position == 0)
	{
		return false;
	}
	else
	{
		if(index != null)
		{ 
			*index = m_lpIndexList[--m_position];
		}
		return true;
	}
}

// StringPool-----------------------------------------------------------------------------------
TinyString::StringPool::StringPool(uint numBlocks, uint blockChars):
	m_numBlocks(numBlocks), m_blockChars(blockChars), m_lpNextStringPool(0)
{
	assert(m_numBlocks > 0);
	assert(m_blockChars > 0);
	m_lpStringMemoryList = (TinyString::StringMemory*)malloc(numBlocks * (sizeof(TinyString::StringMemory) + sizeof(wchar_t)* blockChars));
	assert(m_lpFreeIndexStack == null);
	m_lpFreeIndexStack = new TinyString::IndexStack(numBlocks);
	assert(m_lpFreeIndexStack != 0);

	for(uint i = 0; i < numBlocks; ++i)
	{
		TinyString::StringMemory* lpStringMemory = &m_lpStringMemoryList[i];
		lpStringMemory->lpBelongToWhichStringPool = this;
		lpStringMemory->usedCount = 0;
		lpStringMemory->length = 0;
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
		m_lpStringMemoryList = null;
	}
	if(m_lpFreeIndexStack != 0)
	{
		delete m_lpFreeIndexStack;
		m_lpFreeIndexStack = null;
	}

	TinyString::StringPool* lpStringPoolNext = m_lpNextStringPool;
	while(lpStringPoolNext != 0)
	{
		TinyString::StringPool* lpStringPoolNextNext = lpStringPoolNext->m_lpNextStringPool;
		if(lpStringPoolNext != 0)
		{
			delete lpStringPoolNext;
			lpStringPoolNext = null;
		}
		lpStringPoolNext = lpStringPoolNextNext;
	}
}

TinyString::StringMemory* TinyString::StringPool::pushString(const wchar_t* lpString)
{
	assert(lpString != null);
	assert(wcslen(lpString) > 0);

	TinyString::StringPool* lpStringPoolPrev = null;
	TinyString::StringPool* lpStringPoolThis = this;
	TinyString::StringMemory* lpStringMemory = null;
	while(true)
	{
		if(lpStringPoolThis == null)
		{
			lpStringPoolThis = new TinyString::StringPool(m_numBlocks, m_blockChars);
			assert(lpStringPoolThis != null);
			assert(lpStringPoolPrev != null);
			lpStringPoolPrev->m_lpNextStringPool = lpStringPoolThis;
			lpStringPoolThis->pushString(lpString);
			break;
		}
		else
		{
			lpStringMemory = lpStringPoolThis->setString(lpString);
			if(lpStringMemory != null)
			{
				++lpStringMemory->usedCount;
				return lpStringMemory;
			}

			uint index;
			if(m_lpFreeIndexStack->pop(&index))
			{
				size_t strLen = wcslen(lpString);
				memcpy(m_lpStringMemoryList[index].lpStr, lpString, (strLen + 1/*'\0'*/) * sizeof(wchar_t));
				lpStringMemory = &m_lpStringMemoryList[index];
				lpStringMemory->length = strLen;
				++lpStringMemory->usedCount;
				return lpStringMemory;
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

TinyString::StringMemory* TinyString::StringPool::setString(const wchar_t* lpString)
{
	assert(lpString != null);
	assert(wcslen(lpString) > 0);

	size_t stringLength = wcslen(lpString);
	for(uint i = 0; i < m_numBlocks; ++i)
	{
		TinyString::StringMemory* lpStringMemory = &m_lpStringMemoryList[i];
		assert(lpStringMemory != null);
		if(lpStringMemory->usedCount > 0 && stringLength == lpStringMemory->length && wcscmp(lpString, lpStringMemory->lpStr) == 0)
		{
			return lpStringMemory;
		}
	}
	return null;
}

void TinyString::StringPool::recycle(TinyString::StringMemory* lpStringMemory)
{
	assert(lpStringMemory != null);
	assert(lpStringMemory->usedCount == 0);
	m_lpFreeIndexStack->push(lpStringMemory->indexInStringPool);
}

// StringMemory-----------------------------------------------------------------------------------
TinyString::StringMemory::StringMemory():
	lpBelongToWhichStringPool(null), usedCount(0), indexInStringPool(0), length(0), lpStr(null)
{
	// Do nothing
}