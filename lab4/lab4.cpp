#include <iostream>
#include <random>
#include <iomanip>
#include <mpi.h>
#include <chrono>
#include <thread>

using namespace std;

const int M = 17; //number of rows (and vector elements)
const int N = 4; //number of collumns
const int Ni = N + 1; // additional column for row index
const int MIN_VALUE = -4;
const int MAX_VALUE = 9;

std::random_device rd; // obtain a random number from hardware
std::mt19937 eng(rd()); // seed the generator
std::uniform_int_distribution<> distr(MIN_VALUE, MAX_VALUE);


void fillVectorRange(int vector[], int startElem, int endElem) {
	for (int i = startElem; i < endElem; i++) {
		vector[i] = distr(eng);
	}
}

void printVectorRange(int vector[], int startElem, int endElem, int rank) {
	for (int i = startElem; i < endElem; i++) {
		cout << "rank #" << rank << ", index[" << setw(2) << right << i << "] ";
		cout << setw(2) << right << vector[i] << endl;
	}
}

void fillMatrixRange(int matrix[][Ni], int startRow, int endRow) {
	for (int i = startRow; i < endRow; i++) {
		for (int j = 0; j < N; j++) {
			matrix[i][j] = distr(eng);
		}
	}
}

void printMatrixRange(int matrix[][Ni], int startRow, int endRow, int rank) {
	for (int i = startRow; i < endRow; i++) {
		cout << "rank #" << rank << ", index[" << setw(2) << right << matrix[i][N] << "] ";
		for (int j = 0; j < N; j++) {
			cout << setw(2) << right << matrix[i][j] << " ";
		}
		cout << endl;
	}
}

void printChunkVector(int* matrix, int count, int rank) {
	int index = 0;
	while (index < count) {
		cout << "rank #" << rank << ", index[" << setw(2) << right << matrix[index + N] << "] ";

		for (int j = 0; j < N; j++) {
			cout << setw(2) << right << matrix[index] << " ";
			index++;
		}
	
		index++;
		cout << endl;
	}
}

int nextNegativeInRow(int row[]) {
	for (int i = 0; i < N; i++) {
		if (row[i] < 0) {
			return row[i];
		}
	}
	return 0;
}

void replacePositive(int* matrix, int count) {

	int* temp = matrix;

	int index = 0;
	while (index < count) {
	
		int negVal = 0;

		if (nextNegativeInRow(matrix) == 0) {
			matrix += Ni;
			index += Ni;
			continue;
		} else {
			negVal = nextNegativeInRow(matrix);
		}
		
		for (int j = 0; j < N; j++) {
			if (matrix[j] >= 0) {
				matrix[j] = negVal;
			}
		}

		matrix += Ni;
		index += Ni;
	}
}

int main(int argc, char** argv) {

	MPI_Init(&argc, &argv);
	/* Произведение суммы положительных элементов первой части вектора на сумму
	отрицательных элементов второй части вектора. */

	int rank, grSize;

	MPI_Comm_size(MPI_COMM_WORLD, &grSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int mid = M / 2;
	int chunk = M / grSize;

	int startI = chunk * rank;
	int endI = chunk * (rank + 1);

	if (rank == grSize - 1) {
		endI = M;
	}

	int vector[M];

	fillVectorRange(vector, startI, endI);
	printVectorRange(vector, startI, endI, rank);

	MPI_Barrier(MPI_COMM_WORLD);
	cout << "----------------" << endl;
	MPI_Barrier(MPI_COMM_WORLD);

	int sums[2]; // 0 - positive, 1 - negative
	sums[0] = 0;
	sums[1] = 0;

	for (int i = startI; i < endI; i++) {
		if (i >= mid) {
			if (vector[i] < 0) {
				sums[1] += vector[i];
			}
		}
		else {
			if (vector[i] > 0) {
				sums[0] += vector[i];
			}
		}
	}

	int finalSum[2];
	MPI_Reduce(sums, finalSum, 2, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	if (rank == 0) {
		cout << "Сумма положительных в левой половине : " << finalSum[0] << endl;
		cout << "Cyммa отрицательных в правой половине : " << finalSum[1] << endl;
		cout << "Искомое произведение : " << finalSum[0] * finalSum[1] << endl;
	}

	MPI_Barrier(MPI_COMM_WORLD);
	cout << "****************" << endl;
	MPI_Barrier(MPI_COMM_WORLD);

	/* Заменить в прямоугольной матрице все положительные элементы на ближайший по
	строке отрицательный(при наличии).
	(Можно было сделать проще, но необходимость освоить коллективные функции потребовала
	сделать более запутанную реализацию)
	*/

	int matrix[M][Ni];
	if (rank == 0) {
		fillMatrixRange(matrix, 0, M);
		

		for (int i = 0; i < M; i++) {
			matrix[i][N] = i;
		}

		printMatrixRange(matrix, 0, M, rank);
	}

	int* rowsChunk = new int[Ni * chunk];

	MPI_Scatter(matrix, Ni * chunk, MPI_INT, rowsChunk, Ni * chunk, MPI_INT, 0, MPI_COMM_WORLD);
	
	replacePositive(rowsChunk, Ni * chunk);

	MPI_Barrier(MPI_COMM_WORLD);
	cout << "----------------" << endl;
	MPI_Barrier(MPI_COMM_WORLD);

	printChunkVector(rowsChunk, Ni * chunk, rank);

	MPI_Gather(rowsChunk, Ni * chunk, MPI_INT, matrix, Ni * chunk, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
	cout << "----------------" << endl;
	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0) {
		int remainder = M % chunk;



		while (remainder != 0) {
			int tailRow = chunk * grSize;
			int* tail = matrix[tailRow];

			replacePositive(tail, N);
			tail += Ni;

			remainder--;
		}

		printMatrixRange(matrix, 0, M, rank);
	}
	

	//int endRow = (rank == grSize - 1) ? maxRows : chunk;

	//replacePositive(rowChunk, endRow);

	//MPI_Gather(&matrix, N * maxRows, )

	MPI_Finalize();

}