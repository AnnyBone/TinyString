#ifndef __TINY_STRING_H__
#define __TINY_STRING_H__

#include <map>

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

		static wchar_t* wcsclone(const wchar_t* lpSrc, wchar_t* lpDest = 0);

		void release();

		// StringAlloc-----------------------------------------------------------------------------------
		class StringAlloc
		{
		friend TinyString;

		private:
			static const unsigned int NUM_BLOCKS_PER_STRINGPOOL;
			static StringMemory m_nullStringMemory;
			static StringMemory m_emptyStringMemory;

			typedef std::map<unsigned int, StringPool*> StringPoolMap;
			static StringPoolMap m_stringPoolMap;

			static StringMemory* alloc(const wchar_t* lpString);
			static unsigned int getCeilPowerOf2(unsigned int value);

			StringAlloc(const StringAlloc& value);
			StringAlloc& operator=(const StringAlloc& value);
		};

		// IndexStack-----------------------------------------------------------------------------------
		class IndexStack
		{
		public:
			IndexStack(unsigned int capability);
			~IndexStack();

			bool push(unsigned int index);
			bool pop(unsigned int* index);

		private:
			unsigned int* m_lpIndexList;
			unsigned int m_position;
			unsigned int m_capability;

			IndexStack(const IndexStack& value);
			IndexStack& operator=(const IndexStack& value);
		};

		// StringPool-----------------------------------------------------------------------------------
		class StringPool
		{
		public:
			StringPool(unsigned int numBlocks, unsigned int blockChars);
			~StringPool();

			inline unsigned int getBlockChars() const { return m_blockChars; }
			StringMemory* pushString(const wchar_t* lpString);
			void recycle(StringMemory* lpStringMemory);

		private:
			unsigned int m_numBlocks;
			unsigned int m_blockChars;
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
			unsigned int usedCount;
			unsigned int indexInStringPool;
			wchar_t* lpStr;

		private:
			StringMemory(const StringMemory& value);
			StringMemory& operator=(const StringMemory& value);
		};
	};
}

#endif