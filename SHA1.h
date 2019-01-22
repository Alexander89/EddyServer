#ifndef SHA1_H
#define SHA1_H

#if !defined(SHA1_UTILITY_FUNCTIONS) && !defined(SHA1_NO_UTILITY_FUNCTIONS)
#define SHA1_UTILITY_FUNCTIONS
#endif

#if !defined(SHA1_STL_FUNCTIONS) && !defined(SHA1_NO_STL_FUNCTIONS)
#define SHA1_STL_FUNCTIONS
#if !defined(SHA1_UTILITY_FUNCTIONS)
#error STL functions require SHA1_UTILITY_FUNCTIONS.
#endif
#endif

#include <memory.h>
#include <limits.h>

#include <stdio.h>
#include <string.h>

#include <stdlib.h>

// You can define the endian mode in your files without modifying the SHA-1
// source files. Just #define SHA1_LITTLE_ENDIAN or #define SHA1_BIG_ENDIAN
// in your files, before including the SHA1.h header file. If you don't
// define anything, the class defaults to little endian.
#if !defined(SHA1_LITTLE_ENDIAN) && !defined(SHA1_BIG_ENDIAN)
#define SHA1_LITTLE_ENDIAN
#endif

///////////////////////////////////////////////////////////////////////////
// Declare SHA-1 workspace

typedef union
{
	unsigned char c[64];
	unsigned int l[16];
} SHA1_WORKSPACE_BLOCK;

class SHA1
{
public:
	// Constructor and destructor
	SHA1();
	~SHA1();

	void Reset();

	// Hash in binary data and strings
	void update(const unsigned char* pbData, unsigned int uLen);

	// Finalize hash; call it before using ReportHash(Stl)
	void final();

	// Get the raw message digest (20 bytes)
	bool getHash(unsigned char* pbDest20) const;

private:
	// Private SHA-1 transformation
	void Transform(unsigned int* pState, const unsigned char* pBuffer);

private:    // Member variables
	unsigned int m_state[5];
	unsigned int m_count[2];
	unsigned int m_reserved0[1]; // Memory alignment padding
	unsigned char m_buffer[64];
	unsigned char m_digest[20];
	unsigned int m_reserved1[3]; // Memory alignment padding

	unsigned char m_workspace[64];
	SHA1_WORKSPACE_BLOCK* m_block; // SHA1 pointer to the byte array above
};

#endif // SHA1_H
