#include "pch.h"

/*
Создать и запустить программу на 2-х процессах 
с применением функций int MPI_Init(int* argc, char*** argv) 
и int MPI_Finalize( void ).
----------------------------------------------------------------
Для того, чтобы запустить на двух процессах нужно просто в 
свойствах проекта на вкладке "Debugging" указать аргумент
командной строки: "-n 2"
*/

using namespace std;

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);
	cout << "Parallel Section..." << endl;
	MPI_Finalize();
}


