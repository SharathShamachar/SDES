// SDES.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>



void xorFunc(unsigned char* operandL, unsigned char* operandR, unsigned char* out) 
{
	for (std::size_t i = 0; i < strlen((char*)operandL); i++)
	{
		out[i] = ((uint8_t)operandL[i] ^ (uint8_t)operandR[i]) + 48;
	}
}

void sBox(unsigned char* in, unsigned char* out)
{
	std::string result; // Result in string form
	int Lrow;
	int Lcol;
	int Rrow;
	int Rcol;

	std::string bin[4] = {"00","01","10","11"}; // Used to convert sbox element to binary string

	int s0[4][4] = { // Sbox1
		{ 1,0,3,2 },
		{ 3,2,1,0 },
		{ 0,2,1,3 },
		{ 3,1,3,2 }
	};

	int s1[4][4] = { // Sbox 2
		{ 0,1,2,3 },
		{ 2,0,1,3 },
		{ 3,0,1,0 },
		{ 2,1,0,3 }
	};a

	if (in[0] == '1')
	{
		Lrow = 2 + (uint8_t)in[3] - 48;
	}
	else { Lrow = (uint8_t)in[3] - 48;}


	if (in[1] == '1')
	{
		Lcol = 2 + (uint8_t)in[2] - 48;
	}
	else { Lcol = (uint8_t)in[2] - 48; }

	if (in[4] == '1')
	{
		Rrow = 2 + (uint8_t)in[7] - 48;
	}
	else { Rrow= (uint8_t)in[7] - 48; }
	if (in[5] == '1')
	{
		Rcol = 2 + (uint8_t)in[6] - 48;
	}else { Rcol = (uint8_t)in[6] - 48; }

	// Uses string to store result since it is easier to perform concatenation with string than char arrays
	result = bin[s0[Lrow][Lcol]] + bin[s1[Rrow][Rcol]]; 
	
	strncpy((char*)out, result.c_str(), 5); // Copies result string into out char array

}

void permutate(unsigned char* in, unsigned char* out, const char* perm)
{

	for (std::size_t i = 0; i < strlen(perm); i++)
	{
		out[i] = in[perm[i] - '0'];
	}

}

void leftShift(unsigned char* k1)
{
	char left[5];
	char right[5];
	unsigned int t;

	for (int i = 0; i < 5; i++)
	{
		left[i] = k1[i];
	}

	for (int i = 0; i < 5; i++)
	{
		right[i] = k1[i + 5];
	}

	t = left[0];
	memmove(left, left + 1, 5 - 1);
	left[4] = t;

	t = right[0];
	memmove(right, right + 1, 5 - 1);
	right[4] = t;

	for (int i = 0; i < 5; i++)
	{
		k1[i] = left[i];
	}
	for (int i = 0; i < 5; i++)
	{
		k1[i+5] = right[i];
	}
}

void keyGen(unsigned char* k,unsigned char* k1, unsigned char* k2)
{
	std::cout << "\n--------------KEY GEN--------------";
	unsigned char kTemp[11] = { 0 };
	permutate(k, kTemp, "2416390875"); // P10 

	std::cout << "\nP10:    " << kTemp;

	leftShift(kTemp); // LS1

	std::cout << "\nLS1:    " << kTemp;
	
	for (int i = 0; i < 8; i++) // Shrink key from 10 bits to 8
	{
		k1[i] = kTemp[i + 2];
	}

	permutate(kTemp, k1, "52637498"); // P8 (k1)

	std::cout << "\nK1:     " << k1;

	leftShift(kTemp); // LS2
	leftShift(kTemp);

	permutate(kTemp, k2, "52637498"); // P8 (k2)

	std::cout << "\nK2:     " << k2;
}

void mangler(unsigned char* out, unsigned char* key)
{
	unsigned char p4[5] = { 0 };
	unsigned char temp[5] = { 0 };
	unsigned char ipLHalf[5] = { 0 };
	unsigned char ipRHalf[5] = { 0 };

	memcpy(ipLHalf, out, 4); // Copies cText to IPLHalf for later use
	strncpy((char*)ipRHalf, (char*)out + 4, 5);

	for (int i = 0; i < 4; i++) // Use strcpy instead?
	{
		temp[i] = out[i + 4];
	}

	permutate(temp, out, "30121230"); // EP

	std::cout << "\nEP:     " << out;

	xorFunc(out, key, out); // XOR K1

	std::cout << "\nXOR:    " << out;

	for (int i = 0; i < 4; i++)
	{
		temp[i] = out[i + 4];
	}

	sBox(out, temp); // S0 AND S1
	std::cout << "\nSBOX:   " << temp;

	permutate(temp, p4, "1320"); // P4
	std::cout << "\nP4:     " << p4;

	xorFunc(p4, ipLHalf, p4);
	std::cout << "\nP4XOR:  " << p4;

	strncpy((char*)out, (char*)p4, 4);
	strncpy((char*)out + 4, (char*)ipRHalf, 4);

	std::cout << "\nRound:  " << out;

}

void decryption(unsigned char* pText, unsigned char* cText, unsigned char* k1, unsigned char* k2)
{
	unsigned char rnd[9] = { 0 };
	unsigned char temp[5] = { 0 }; // Used for swapping halves

	permutate(cText, rnd, "15203746"); // IP
	std::cout << "\nIP:     " << cText;

	std::cout << "\n--------------ROUND 1--------------";
	mangler(rnd, k2);

	strncpy((char*)temp, (char*)rnd, 4); // SW
	strncpy((char*)rnd, (char*)rnd + 4, 4);
	strncpy((char*)rnd + 4, (char*)temp, 4);

	std::cout << "\nSW:     " << rnd;
	std::cout << "\n--------------ROUND 2--------------";
	mangler(rnd, k1);

	permutate(rnd, pText, "30246175"); //IP-1
	std::cout << "\n\nPlaintext:" << pText << "\n";
}

void encryption(unsigned char* pText, unsigned char* cText, unsigned char* k1, unsigned char* k2)
{
	unsigned char rnd[9] = { 0 };
	unsigned char temp[5] = { 0 }; // Used for swapping halves

	permutate(pText, rnd, "15203746"); //IP
	std::cout << "\nIP:     " << pText;

	std::cout << "\n--------------ROUND 1--------------";
	mangler(rnd, k1);

	strncpy((char*)temp, (char*)rnd, 4); // SW
	strncpy((char*)rnd, (char*)rnd + 4, 4);
	strncpy((char*)rnd + 4, (char*)temp, 4);

	std::cout << "\nSW:     " << rnd;
	std::cout << "\n--------------ROUND 2--------------";
	mangler(rnd, k2);

	permutate(rnd, cText, "30246175"); // IP-1
	std::cout << "\n\nCiphertext:" << cText << "\n";
}


int main()
{
	// p = 01110010
	// k = 1010000010
	unsigned char pText[9] = { 0 }; 
	unsigned char cText[9] = { 0 }; 
	unsigned char k[11];
	unsigned char k1[9] = { 0 };
	unsigned char k2[9] = { 0 };

	int type;

	std::cout << "1. Encryption \n2. Decryption\n";
	std::cin >> type;

	if (type == 1) 
	{

		std::cout << "\nEnter plaintext: ";
		std::cin >> pText;

		std::cout << "Enter key: ";
		std::cin >> k;

		keyGen(k, k1, k2); // Generates key1 and key2
		encryption(pText, cText, k1, k2);
	}
	else if (type == 2)
	{
		std::cout << "\nEnter ciphertext: ";
		std::cin >> cText;

		std::cout << "Enter key: ";
		std::cin >> k;

		keyGen(k, k1, k2); // Generates key1 and key2
		decryption(pText, cText, k1, k2);
	}
	else
	{
		std::cout << "\n Error: Incorrect input";
	}
	
	return 0;
}

