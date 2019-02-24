
/*
1)	Запустить 4 процесса.
2)	На каждом процессе создать переменные: ai,bi,ci, где i – номер процесса. 
Инициализировать переменные. Вывести данные на печать.
3)	Передать данные на другой процесс. Найти: c0=a1+b2; с1=a3+b0;  c2=a0+b3; c3=a2+b1. 
Напечатать номера процессов, исходные данные, поступившие данные и результаты операций.
*/

#include "pch.h"

using namespace std;

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank, grSize, a, b, c;

	MPI_Comm_size(MPI_COMM_WORLD, &grSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Status status;

	if (rank == 0) {
		a = 1;
		b = 2;
		MPI_Send(&a, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
		MPI_Send(&b, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	}
	else if (rank == 1) {
		a = 3;
		b = 4;
		MPI_Send(&a, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&b, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
	}
	else if (rank == 2) {
		a = 5;
		b = 6;
		MPI_Send(&a, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
		MPI_Send(&b, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	else if (rank == 3) {
		a = 7;
		b = 8;
		MPI_Send(&a, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		MPI_Send(&b, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
	}

	cout << "a" << rank << "=" << a << " b" << rank << "=" << b << endl;

	MPI_Recv(&a, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	MPI_Recv(&b, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	c = a + b;
	cout << "c" << rank << "=" << a << "+" << b << "=" << c << endl;

	MPI_Finalize();
}
