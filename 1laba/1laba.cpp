#include <iostream>
#include <conio.h>
#include <sstream>
#include <Windows.h>
#include <iomanip>
#include <fstream>

using namespace std;

struct matrix {
	int row;
	int col;
};

struct InvalidElement {
	int row;
	int col;
	int pos;
};

enum Error {

	NotFail,
	ErrorOpenFile,
	ErrorFileEOF,
	ErrorRectangle, // - прямоугольность
	ErrorMemoryAllocation, // - выделение памяти
	ErrorProcessingMatr, // - обработка матрицы
	ErrorInvalidElement

};

union ErrorInfo {

	int OpenFile;
	int Rectangle;
	int ProcessingMatr;
	int MemoryAllocation;
	InvalidElement Element;

};

void DeleteMatr(int row, int** matr) {

	for (int i = 0; i < row; i++)
		delete[] matr[i];
	delete[] matr;
}

int MemoryAllocation(const matrix& Matr, int**& matr) {

	int i;
	matr = NULL;
	try
	{
		matr = new int* [Matr.row];
		for (i = 0; i < Matr.row; i++)
			matr[i] = new int[Matr.col];
	}
	catch (...)
	{
		if (!matr) // matr == NULL
			return -1;
		DeleteMatr(i, matr);
		return i;
	}

	return -2;
}

Error LoadMatr(const string& FileName, matrix& DimensionMatr, int**& matr, ErrorInfo* EInfo = NULL) {

	ifstream fin;
	int symbol, details = 0, AbsSymbolPos = 0;
	char FlowState = 0, state = 0;
	bool CompleteCheck = true;
	DimensionMatr.col = 0;
	DimensionMatr.row = 0;
	matr = NULL;

	fin.open(FileName);

	if (!fin.is_open()) {
		//fin.close();
		if (EInfo) {
			EInfo->OpenFile = errno;
		}
		return ErrorOpenFile;
	}

	fin >> ws;
	if (fin.eof()) {
		fin.close();
		return ErrorFileEOF;
	}


	while (CompleteCheck) {
		AbsSymbolPos = fin.tellg();
		fin >> symbol;

		if (fin.fail() || ((state = fin.peek()) != ' ' && state != '\t' && state != '\n' && state != EOF)) {
			fin.close();
			if (EInfo) {
				EInfo->Element.row = DimensionMatr.row;
				EInfo->Element.col = DimensionMatr.col;
				EInfo->Element.pos = AbsSymbolPos;
			}
			DimensionMatr.col = 0;
			DimensionMatr.row = 0;
			return ErrorInvalidElement;
		}

		DimensionMatr.col++;

		while (FlowState != EOF) {
			FlowState = fin.get();
			switch (FlowState) {
			case ' ': case '\t':
				break;
			case EOF:
				CompleteCheck = false;
			case '\n':
				if (DimensionMatr.col) {

					DimensionMatr.row++;
					details += DimensionMatr.col;

					if (details != DimensionMatr.col * DimensionMatr.row) {
						fin.close();

						if (EInfo)
							EInfo->Rectangle = DimensionMatr.row;

						DimensionMatr.col = 0;
						DimensionMatr.row = 0;
						return ErrorRectangle;
					}

					DimensionMatr.col = 0;
				}
				break;
			default:
				fin.unget();
				FlowState = EOF;
			}
		}
		FlowState = 0;
	}

	DimensionMatr.col = details / DimensionMatr.row;

	//cout << "строк " << DimensionMatr.row << "\n" << "столбцов "  << DimensionMatr.col << "\n";

	fin.clear();
	fin.seekg(0);

	int ErrorMA; // ErrorMA - ErrorMemoryAllocation
	ErrorMA = MemoryAllocation(DimensionMatr, matr);
	if (ErrorMA != -2) {
		fin.close();
		if (EInfo != NULL)
			EInfo->MemoryAllocation = ErrorMA;
		return ErrorMemoryAllocation;
	}

	for (int i = 0; i < DimensionMatr.row; i++) {
		for (int j = 0; j < DimensionMatr.col; j++)
			fin >> matr[i][j];
	}

	fin.close();
	return NotFail;
}


Error MatrProcessing(const matrix& DimensionMatr, int** matr) {

	int swap, colCheck, Max;
	int* MatrCol;

	try
	{
		MatrCol = new int[DimensionMatr.row];
	}
	catch (...)
	{
		return ErrorMemoryAllocation;
	}

	for (int i = 0; i < DimensionMatr.row; i++) {
		Max = matr[i][0];
		colCheck = 0;
		for (int j = 1; j < DimensionMatr.col; j++) {
			if (matr[i][j] > Max) {
				Max = matr[i][j];
				colCheck = j;
			}
		}
		MatrCol[i] = colCheck;
		//cout << MatrCol[n] << endl;
	}

	for (int i = 0; i < DimensionMatr.row; i++) {
		int j = MatrCol[i];

		swap = matr[0][j];
		matr[0][j] = matr[DimensionMatr.row - 1][j];
		matr[DimensionMatr.row - 1][j] = swap;
	}

	delete[] MatrCol;
	return NotFail;
}


Error CopyMatr(const matrix& Matr, int** matr, int**& DuplicateMatr, ErrorInfo* EInfo = NULL) {

	int ErrorMA; // ErrorMA - ErrorMemoryAllocation

	ErrorMA = MemoryAllocation(Matr, DuplicateMatr);
	if (ErrorMA != -2) {
		if (EInfo != NULL)
			EInfo->MemoryAllocation = ErrorMA;
		return ErrorMemoryAllocation;
	}

	for (int i = 0; i < Matr.row; i++) {
		for (int j = 0; j < Matr.col; j++) {
			DuplicateMatr[i][j] = matr[i][j];
		}
	}

	return NotFail;
}


string СonclusionMatr(matrix& Matr, int**& matr) {

	stringstream sMatr;

	for (int i = 0; i < Matr.row; i++) {
		for (int j = 0; j < Matr.col; j++) {
			sMatr << left << setw(7) << matr[i][j];
		}
		sMatr << "\n";
	}

	return sMatr.str();
}

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	Error mistake;
	ErrorInfo EInfo;
	string FileName, MatrStr;
	int** Matr = NULL;
	matrix DimensionMatr;


	while (true) {
		system("cls");
		cout << "Введите название файла." << endl;
		getline(cin, FileName);
		mistake = LoadMatr(FileName, DimensionMatr, Matr, &EInfo);

		if (mistake != NotFail) {
			switch (mistake) {
			case ErrorOpenFile:
				cout << "Не удалось открыть файл. Код системной ошибки: " << EInfo.OpenFile << endl;
				cout << "Нажмите любую клавишу для продолжения." << endl;
				_getch();
				continue;

			case ErrorFileEOF:
				cout << "Файл пуст." << endl;
				cout << "Нажмите любую клавишу для продолжения." << endl;
				_getch();
				continue;

			case ErrorInvalidElement:
				cout << "Не корректный элемент в " << EInfo.Element.row << " строке, "
					<< EInfo.Element.col << " столбце, под №" << EInfo.Element.pos << endl;

				cout << "Нажмите любую клавишу для продолжения." << endl;
				_getch();
				continue;

			case ErrorRectangle:
				cout << "Матрица не прямоугольная, начиная с " << EInfo.Rectangle << " строки." << endl;

				cout << "Нажмите любую клавишу для продолжения." << endl;
				_getch();
				continue;

			case ErrorMemoryAllocation:
				if (ErrorMemoryAllocation == -1)
					cout << "Не удалось выделить память под массив указателей." << endl;

				if (ErrorMemoryAllocation > 0)
					cout << "Не удалось выделить память под " << EInfo.MemoryAllocation << " строку." << endl;

				cout << "Нажмите любую клавишу для продолжения." << endl;
				_getch();
				continue;
			}
		}
		cout << "Память выделенна." << endl;
		break;
	}

	int** DuplicateMatr = NULL;

	mistake = CopyMatr(DimensionMatr, Matr, DuplicateMatr, &EInfo);

	if (mistake != NotFail) {
		if (EInfo.MemoryAllocation == -1)
			cout << "Не удалось выделить память под массив указателей." << endl;

		if (EInfo.MemoryAllocation > 0)
			cout << "Не удалось выделить память " << EInfo.MemoryAllocation << " строку." << endl;

		cout << "Нажмите любую клавишу для выхода." << endl;
		_getch();
		return 0;
	}

	mistake = MatrProcessing(DimensionMatr, DuplicateMatr);

	if (mistake == ErrorMemoryAllocation) {
		cout << "Не удалось выделить память под массив." << endl;

		cout << "Нажмите любую клавишу для выхода." << endl;
		_getch();
		return 0;
	}

	cout << "Изначальная матрица:" << endl;

	cout << СonclusionMatr(DimensionMatr, Matr);

	cout << endl << "Обработанная матрица:" << endl;
	cout << СonclusionMatr(DimensionMatr, DuplicateMatr);

	DeleteMatr(DimensionMatr.row, Matr);
	DeleteMatr(DimensionMatr.row, DuplicateMatr);
	return 0;
}