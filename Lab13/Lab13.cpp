
#include "pch.h"

// Создать и запустить программу на n-х процессах печати таблицы умножения.

using namespace std;

const int MAX = 100;

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank, grSize, n, iBeg, iEnd;

	MPI_Comm_size(MPI_COMM_WORLD, &grSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	n = (MAX - 1) / grSize + 1;
	iBeg = rank * n + 1;
	iEnd = (rank + 1) * n;

	for (int i = iBeg; i <= iEnd; i++) {
		cout << "Process [" << rank << "] " << i << "^2=" << i * i << endl;
	}

	MPI_Finalize();
}
