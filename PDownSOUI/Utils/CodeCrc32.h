#pragma once
#include <stdint.h>
#include <string>
using namespace std;

class CodeCrc32
{
public:
	CodeCrc32() {
		makeCrcTable();
		crc = 0;
	}
	~CodeCrc32() {

	}

private:
	/** The crc data checksum so far. */
	uint32_t crc = 0;
	/** The fast CRC table. Computed once when the CRC32 class is loaded. */
	uint32_t crcTable[256];

	/** Make the table for a fast CRC. */
	void makeCrcTable() {
		for (uint32_t n = 0; n < 256; n++) {
			uint32_t c = n;
			for (int k = 8; --k >= 0; ) {
				if ((c & 1) != 0) c = 0xedb88320 ^ (c >> 1);
				else c = c >> 1;
			}
			crcTable[n] = c;
		}
	}

public:
	/**
	 * Returns the CRC32 data checksum computed so far.
	 */
	uint32_t getValue() {
		return crc & 0xffffffff;
	}

	/**
	 * Resets the CRC32 data checksum as if no update was ever called.
	 */
	void reset() {
		crc = 0;
	}

	/**
	 * Adds the complete byte array to the data checksum.
	 *
	 * @param buf the buffer which contains the data
	 */
	void update(const unsigned char* buf, int64_t len) {
		uint32_t off = 0;
		uint32_t c = ~crc;
		while (--len >= 0) c = crcTable[(c ^ buf[off++]) & 0xff] ^ (c >> 8);
		crc = ~c;
	}

	/**
	 * Adds the complete byte array to the data checksum.
	 *
	 * @param buf the buffer which contains the data
	 */
	void update(const unsigned char* buf, int64_t off, int64_t len)
	{
		uint32_t c = ~crc;
		while (--len >= 0) c = crcTable[(c ^ buf[off++]) & 0xff] ^ (c >> 8);
		crc = ~c;
	}

#define BUFFSIZE_CRC32 1024*1024
	uint32_t getFileCrc32(FILE* fp) {
		_fseeki64(fp, 0, SEEK_SET);
		unsigned char* buff = (unsigned char*)malloc(BUFFSIZE_CRC32);
		if (nullptr == buff)
		{
			return 0;
		}
		//unsigned char buff[BUFFSIZE];
		size_t count = 0;
		while (true) {
			count = fread(buff, sizeof(unsigned char), BUFFSIZE_CRC32, fp);
			if (count <= 0) break;
			update(buff, count);
		}
		free(buff);
		buff = nullptr;
		return getValue();
	}

	uint32_t getFileCrc32(FILE* fp, uint64_t pos, uint64_t len) {
		_fseeki64(fp, pos, SEEK_SET);
		unsigned char* buff = (unsigned char*)malloc(BUFFSIZE_CRC32);
		if (nullptr == buff)
		{
			return 0;
		}
		//unsigned char buff[BUFFSIZE];
		uint64_t count = 0;
		while (true) {
			count = fread(buff, sizeof(unsigned char), BUFFSIZE_CRC32, fp);
			if (count <= 0) break;
			if (len >= count) {
				len = len - count;
			}
			else {
				count = len;
				len = 0;
			}
			update(buff, count);
			if (len <= 0) {
				break;
			}
		}
		free(buff);
		buff = nullptr;
		return getValue();
	}
};