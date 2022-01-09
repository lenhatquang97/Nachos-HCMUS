#include "syscall.h"

#define MAX_LENGTH 32

int main()
{
	// KHAI BAO
	int f_Success;										 // Bien co dung de kiem tra thanh cong
	SpaceId si_input, si_output, si_sinhvien, si_result; // Bien id cho file
	int SLTD;											 // Luu so luong thoi diem xet
	char c_readFile;									 // Bien ki tu luu ki tu doc tu file
	//int flag;

	//-----------------------------------------------------------

	// Khoi tao 4 Semaphore de quan ly 3 tien trinh
	f_Success = CreateSemaphore("main", 1);
	if (f_Success == -1){
		return 1;
	}
	f_Success = CreateSemaphore("sinhvien", 1);
	if (f_Success == -1)
		return 1;
	f_Success = CreateSemaphore("voinuoc", 1);
	if (f_Success == -1)
		return 1;
	f_Success = CreateSemaphore("m_vn", 1);
	if (f_Success == -1)
		return 1;

	// Tao file output.txt de ghi ket qua cuoi cung
	f_Success = CreateFile("output.txt");
	if (f_Success == -1)
		return 1;

	// Mo file input.txt chi de doc
	si_input = Open("input.txt", 1);
	if (si_input == -1)
		return 1;

	// Mo file output.txt de doc va ghi
	si_output = Open("output.txt", 0);
	if (si_output == -1)
	{
		Close(si_input);
		return 1;
	}

	// Doc so luong thoi diem xet o file input.txt
	//**** Thuc hien xong doan lenh duoi thi con tro file o input.txt o dong 1
	SLTD = 0;
	while (1)
	{
		Read(&c_readFile, 1, si_input);
		if (c_readFile != '\n')
		{
			if (c_readFile >= '0' && c_readFile <= '9')
				SLTD = SLTD * 10 + (c_readFile - 48);
		}
		else
			break;
	}

	// // Goi thuc thi tien trinh sinhvien.c
	f_Success = Exec("./test/sinhvien");
	if (f_Success == -1)
	{
		Close(si_input);
		Close(si_output);
		return 1;
	}

	// Goi thuc thi tien trinh voinuoc.c
	f_Success = Exec("./test/voinuoc");
	
	if (f_Success == -1)
	{
		Close(si_input);
		Close(si_output);
		return 1;
	}
	while (SLTD--)
	{
		// if(SLTD == 1){
		// 	Exit(0);
		// }
		PrintNum(SLTD);
		PrintString("\n");
		// Tao file sinhvien.txt
		f_Success = CreateFile("sinhvien.txt");
		if (f_Success == -1)
		{
			Close(si_input);
			Close(si_output);
			return 1;
		}

		si_sinhvien = Open("sinhvien.txt", 0);
		if (si_sinhvien == -1)
		{
			Close(si_input);
			Close(si_output);
			return 1;
		}
		while (1)
		{
			if (Read(&c_readFile, 1, si_input) < 1)
			{
				break;
			}
			if (c_readFile != '\n')
			{
				Write(&c_readFile, 1, si_sinhvien);
			}
			else
				break;
		}
		Close(si_sinhvien);

		Signal("sinhvien");

		Wait("main");

		si_result = Open("result.txt", 1);
		if (si_result == -1)
		{
			Close(si_input);
			Close(si_output);
			return 1;
		}

		PrintString("\n Lan thu: ");
		PrintNum(SLTD);
		PrintString("\n");

		while (1)
		{
			if (Read(&c_readFile, 1, si_result) < 1)
			{
				Write("\r\n", 2, si_output);
				Close(si_result);
				Signal("m_vn");
				break;
			}
			Write(&c_readFile, 1, si_output);
			Write(" ", 1, si_output);
		}
	}
	PrintString("Ayyo come\n");
	Close(si_input);
	Close(si_output);
	return 0;
}