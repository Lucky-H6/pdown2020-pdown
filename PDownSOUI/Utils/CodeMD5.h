#pragma once
#include <string>
#include <fstream>
using namespace std;

/*
			//MD5 md5;
			//md5.update(out.data(), pmd5);
			//string cmd5 = md5.toString();
*/

/* Type define */
typedef unsigned char byte;
typedef unsigned int uint32;
class CodeMD5
{
public:
	CodeMD5();
	CodeMD5(const void* input, size_t length);
	CodeMD5(const string& str);
	CodeMD5(ifstream& in);
	void update(const void* input, size_t length);
	void update(const string& str);
	void update(ifstream& in);
	const byte* digest();
	string toString();
	wstring toStringW();
	void reset();

private:
	void update(const byte* input, size_t length);
	void final();
	void transform(const byte block[64]);
	void encode(const uint32* input, byte* output, size_t length);
	void decode(const byte* input, uint32* output, size_t length);
	string bytesToHexString(const byte* input, size_t length);
	wstring wbytesToHexString(const byte* input, size_t length);

	/* class uncopyable */
	CodeMD5(const CodeMD5&);
	CodeMD5& operator=(const CodeMD5&);

private:
	uint32 _state[4];    /* state (ABCD) */
	uint32 _count[2];    /* number of bits, modulo 2^64 (low-order word first) */
	byte _buffer[64];    /* input buffer */
	byte _digest[16];    /* message digest */
	bool _finished;        /* calculate finished ? */

	static const byte PADDING[64];    /* padding for calculate */
	static const char HEX[16];
	enum { BUFFER_SIZE = 1024 };

};


