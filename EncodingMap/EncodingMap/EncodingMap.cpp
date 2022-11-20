#include <iostream>

size_t CharacterCount(size_t szFirstByteRange, size_t szSecondByteRange)
{
	return szFirstByteRange * szSecondByteRange;
}

unsigned char* CreateMapTable(size_t szTable)
{
	unsigned char* alloc = (unsigned char*)malloc(szTable);
	if (alloc)
	{
		memset(alloc, 0, szTable);
	}

	return (unsigned char*)alloc;
}

void FreeMapTable(unsigned char* pAlloc)
{
	if (pAlloc)
	{
		free(pAlloc);
	}
}

unsigned short* InitMapTable(size_t szCharacterCount, size_t szCharacterBytes)
{
	unsigned char* pTable = CreateMapTable(szCharacterCount * szCharacterBytes);
	return (unsigned short*)pTable;
}

void MakeMapTable(unsigned short* pTable, size_t szTable, const char* lpString)
{
	unsigned short newChar = 0;
	unsigned short oldChar = 0;

	for (size_t iteString = 0;; iteString++)
	{
		if ((unsigned char)lpString[iteString] == 0x00)
		{
			break;
		}

		if ((unsigned char)lpString[iteString] < 0x81)
		{
			continue;
		}

		oldChar = *(unsigned short*)&lpString[iteString];
		for (size_t iteTable = 0; iteTable < szTable; iteTable++)
		{
			if (pTable[iteTable] == oldChar)
			{
				newChar = iteTable + 0x8100;
				*(unsigned char*)(&lpString[iteString]) = ((unsigned char*)(&newChar))[1];
				*(unsigned char*)(&lpString[iteString + 1]) = ((unsigned char*)(&newChar))[0];

				iteString++;
				break;
			}

			if (pTable[iteTable] == NULL)
			{
				pTable[iteTable] = oldChar;

				newChar = iteTable + 0x8100;
				*(unsigned char*)(&lpString[iteString]) = ((unsigned char*)(&newChar))[1];
				*(unsigned char*)(&lpString[iteString + 1]) = ((unsigned char*)(&newChar))[0];

				iteString++;
				break;
			}
		}
	}
}

void MapCharacter(unsigned short* pTable, unsigned short* lpCharacter)
{
	unsigned short high = lpCharacter[0] << 8;
	unsigned short low = lpCharacter[0] >> 8;

	if (high < 0x81)
	{
		return;
	}

	size_t offset = (high | low) - 0x8100;

	if (offset >= 0x0 && offset <= 0x39C6)
	{
		lpCharacter[0] = pTable[offset];
	}
}

void SaveMapTable(unsigned short* pTable, size_t szTable)
{
	FILE* fp = NULL;
	errno_t err = fopen_s(&fp, "MapTable.bin", "wb");
	if (!err && fp)
	{
		fwrite(pTable, 2, szTable, fp);
		fflush(fp);
		fclose(fp);
	}
}

void MapString(unsigned short* pTable, const char* lpString)
{
	for (size_t i = 0; ; i++)
	{
		if ((unsigned char)lpString[i] == 0x00)
		{
			break;
		}

		if ((unsigned char)lpString[i] < 0x81)
		{
			continue;
		}

		MapCharacter(pTable, (unsigned short*)&lpString[i]);
		i++;
	}
}

void SJISRangeCheck(char* lpString)
{
	for (size_t iteString = 0;; iteString++)
	{
		if ((unsigned char)lpString[iteString] == 0x00)
		{
			break;
		}

		if ((unsigned char)lpString[iteString] < 0x81)
		{
			continue;
		}

		if ((unsigned char)lpString[iteString] >= 0x81 && (unsigned char)lpString[iteString] <= 0x9F)
		{
			iteString++;
			continue;
		}

		if ((unsigned char)lpString[iteString] >= 0xE0 && (unsigned char)lpString[iteString] <= 0xFC)
		{
			iteString++;
			continue;
		}

		//If Out Of Range Set Character To NULL
		lpString[iteString] = 0x00;
		lpString[iteString + 1] = 0x00;
		iteString++;
	}
}

int main()
{
	size_t characterCount = CharacterCount((0x9F - 0x81) + (0xFC - 0xE0), 0xFF);
	size_t characterBytes = 0x2;
	unsigned short* pTable = InitMapTable(characterCount, characterBytes);

	//����ҳ[932->SJIS, 936->GBK]
	system("chcp 936");

	//��ͬ�ַ�����ͬ����
	//char testStr[] = "�V�̍��ł͐_�l�̉��A�����̓V�g��������炵�Ă��܂����B";	//SJIS������ַ���
	char testStr[] = "��ι��Ǥ��񘔤��¡��य����ʹ������ĺ�餷�Ƥ��ޤ�����"; //GBK������ַ���

	//����ӳ���
	MakeMapTable(pTable, characterCount, testStr);

	//����Ƿ���SJIS���뷶Χ��
	SJISRangeCheck(testStr);

	//ӳ���ַ���
	MapString(pTable, testStr);

	//����ַ���
	std::cout << testStr << std::endl;

	//����ӳ���
	SaveMapTable(pTable, characterCount);

	//�ͷ�ӳ���
	FreeMapTable((unsigned char*)pTable);
}