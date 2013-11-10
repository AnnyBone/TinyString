#pragma comment(lib, "TinyString.lib")

#include <cstdio>

#include "TinyString.h"

using namespace tinyString;

int main()
{
	TinyString str1;
	printf("%s\n", str1.isEmpty() ? "is empty string" : "not empty string");
	printf("%s\n", str1 == L"" ? "is empty string" : "not empty string");

	TinyString str2(L"");
	printf("%s\n", str2.isEmpty() ? "is empty string" : "not empty string");
	printf("%s\n", str2 == L"" ? "is empty string" : "not empty string");

	TinyString str3 = null;
	printf("%s\n", str3.isNull() ? "is null string" : "not null string");
	printf("%s\n", str3 == null ? "is null string" : "not null string");
	printf("length:%d\n", str3.length());

	TinyString str4(null);
	printf("%s\n", str4.isNull() ? "is null string" : "not null string");
	printf("%s\n", str4 == null ? "is null string" : "not null string");

	TinyString str5(L"abcdefg");
	printf("%s\n", str5 == L"abcdefg" ? "equal abcdefg" : "not equal abcdefg");

	TinyString str6 = L"abcdefg";
	printf("%s\n", str6 == L"abcdefg" ? "equal abcdefg" : "not equal abcdefg");
	printf("%s\n", str5 == str6 ? "str5 == str6" : "str5 != str6");
	printf("length:%d\n", str6.length());

	getchar();
	return 0;
}