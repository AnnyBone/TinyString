#ifndef __TINY_STRING_H__
#define __TINY_STRING_H__

#include <map>

#include "TinyStringDefine.h"

namespace tinyString
{
	class TinyString
	{
		public:
		TinyString();
		TinyString(const TinyString& value);
		TinyString(const wchar_t* lpString);
		~TinyString();

		const TinyString& operator=(const TinyString& value);
		const TinyString& operator=(const wchar_t* lpString);
		bool operator==(const TinyString& value);
		bool operator==(const wchar_t* lpString);

		bool isNull();
		bool isEmpty();
		bool isNullOrEmpty();

		private:
		// -----------------------------------------------------------------------------------
		class StringPool;
		class StringMemory;

		// -----------------------------------------------------------------------------------
		StringMemory* m_lpStringMemory;

		void release();

		// StringAlloc-----------------------------------------------------------------------------------
		class StringAlloc
		{
		friend TinyString;

		private:
			static const uint NUM_BLOCKS_PER_STRINGPOOL;
			static StringMemory m_nullStringMemory;
			static StringMemory m_emptyStringMemory;

			typedef std::map<uint, StringPool*> StringPoolMap;
			static StringPoolMap m_stringPoolMap;

			static StringMemory* alloc(const wchar_t* lpString);
			static uint getCeilPowerOf2(uint value);

			StringAlloc(const StringAlloc& value);
			StringAlloc& operator=(const StringAlloc& value);
		};

		// IndexStack-----------------------------------------------------------------------------------
		class IndexStack
		{
		public:
			IndexStack(uint capability);
			~IndexStack();

			bool push(uint index);
			bool pop(uint* index);

		private:
			uint* m_lpIndexList;
			uint m_position;
			uint m_capability;

			IndexStack(const IndexStack& value);
			IndexStack& operator=(const IndexStack& value);
		};

		// StringPool-----------------------------------------------------------------------------------
		class StringPool
		{
		public:
			StringPool(uint numBlocks, uint blockChars);
			~StringPool();

			inline uint getBlockChars() const { return m_blockChars; }
			StringMemory* pushString(const wchar_t* lpString);
			StringMemory* setString(const wchar_t* lpString);
			void recycle(StringMemory* lpStringMemory);

		private:
			uint m_numBlocks;
			uint m_blockChars;
			StringMemory* m_lpStringMemoryList;
			StringPool* m_lpNextStringPool;
			IndexStack* m_lpFreeIndexStack;

			StringPool(const StringPool& value);
			StringPool& operator=(const StringPool& value);
		};

		// StringMemory-----------------------------------------------------------------------------------
		class StringMemory
		{
		public:
			StringMemory();

			StringPool* lpBelongToWhichStringPool;
			uint usedCount;
			uint indexInStringPool;
			wchar_t* lpStr;

		private:
			StringMemory(const StringMemory& value);
			StringMemory& operator=(const StringMemory& value);
		};
	};
}

#endif