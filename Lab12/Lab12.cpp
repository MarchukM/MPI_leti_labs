
#include "pch.h"

using namespace std;

/*
Создать и запустить программу на 3-х процессах с применением функций:
1)	int MPI_Init( int* argc, char*** argv);
2)	int MPI_Finalize( void );
3)	int MPI_Comm_size( MPI_Comm comm, int* size)
4)	int MPI_Comm_rank( MPI_Comm comm, int* rank)
*/

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);
	
	int rank, grSize;

	MPI_Comm_size(MPI_COMM_WORLD, &grSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		cout << "Total : " << grSize << endl;
	}
	cout << "This is process [rank " << rank << " ]" << endl;

	MPI_Finalize();
}

