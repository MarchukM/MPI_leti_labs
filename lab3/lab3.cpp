#include <iostream>
#include <random>
#include <iomanip>
#include <mpi.h>
#include <chrono>
#include <thread>

using namespace std;

const int M = 16; //number of rows
const int N = 4; //number of collumns
const int MIN_VALUE = -3;
const int MAX_VALUE = 9;
MPI_Datatype MPI_NEGSUM;

typedef struct NegSum
{
	int topHalf = 0;
	int botHalf = 0;
} NegSum;

void fillMatrixRange(int matrix[][N], int startRow, int endRow) {
	std::random_device rd; // obtain a random number from hardware
	std::mt19937 eng(rd()); // seed the generator
	std::uniform_int_distribution<> distr(MIN_VALUE, MAX_VALUE);

	for (int i = startRow; i < endRow; i++) {
		for (int j = 0; j < N; j++) {
			matrix[i][j] = distr(eng);
		}
	}
}

void printMatrixRange(int matrix[][N], int startRow, int endRow, int rank) {
	for (int i = startRow; i < endRow; i++) {
		cout << "rank #" << rank << ", index[" << setw(2) << right << i << "] ";
		for (int j = 0; j < N; j++) {
			cout << setw(2) << right << matrix[i][j] << " ";
		}
		cout << endl;
	}
}

int nextNegativeInRow(int row[], int indexToStart) {
	for (int i = indexToStart; i < N; i++) {
		if (row[i] < 0) {
			return row[i];
		} 
	}
	return 0;
}

void replacePositive(int matrix[][N], int startRow, int endRow) {
	for (int i = startRow; i < endRow; i++) {
		if (nextNegativeInRow(matrix[i], 0) == 0) {
			continue;
		}
		int negVal = 0;

		for (int j = 0; j < N; j++) {
			if (nextNegativeInRow(matrix[i], j) != 0) {
				negVal = nextNegativeInRow(matrix[i], j);
			}
			if (matrix[i][j] >= 0) {
				matrix[i][j] = negVal;
			}
		}
	}
}

NegSum calculateNumOfNegativeElements(int matrix[][N], int startRow, int endRow, int middle) {
	NegSum negSum;

	for (int i = startRow; i < endRow; i++) {
		for (int j = 0; j < N; j++) {
			if (matrix[i][j] < 0) {
				if (i < middle) {
					++negSum.topHalf;
				}
				else {
					++negSum.botHalf;
				}
			}
		}
	}
	return negSum;
}

void sendNumOfNegativeElements(int matrix[][N], int startRow, int endRow, int middle) {
	NegSum negSum = calculateNumOfNegativeElements(matrix, startRow, endRow, middle);
	MPI_Send(&negSum, 1, MPI_NEGSUM, 0, 0, MPI_COMM_WORLD);
}

void createNumSumDataType() {
	/* create a type for struct NegSum */
	const int nitems = 2;
	int          blocklengths[2] = { 1,1 };
	MPI_Datatype types[2] = { MPI_INT, MPI_INT };
	MPI_Aint     offsets[2];

	offsets[0] = offsetof(NegSum, topHalf);
	offsets[1] = offsetof(NegSum, botHalf);

	MPI_Type_create_struct(nitems, blocklengths, offsets, types, &MPI_NEGSUM);
	MPI_Type_commit(&MPI_NEGSUM);
}

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	int rank, groupSize;

	MPI_Comm_size(MPI_COMM_WORLD, &groupSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//The following array will be filled partially by rows via different processes 
	int matrix[M][N];

	//*****define ranges of rows per process*****
	int k;
	int startRow;
	int endRow;

	k = M / groupSize; //rows per process
	startRow = k * rank;
	endRow = k * (rank + 1);

	if (rank == groupSize - 1) { // if last process
		endRow = M; // then range will contain all remaining rows;
	}
	//******************************************

	fillMatrixRange(matrix, startRow, endRow); // fill array partially
	printMatrixRange(matrix, startRow, endRow, rank); // each process print their own lines

	//just to beautify the displaying results
	MPI_Barrier(MPI_COMM_WORLD);
	cout << "----------------" << endl;
	MPI_Barrier(MPI_COMM_WORLD);

	//replace positive values with negative
	replacePositive(matrix, startRow, endRow);
	printMatrixRange(matrix, startRow, endRow, rank);

	//LAB 2
	//divide array to two parts above and below the middle. 
	int middle = M / 2; 

	createNumSumDataType();

	if (rank != 0) {
		sendNumOfNegativeElements(matrix, startRow, endRow, middle);
	}
	else {
		NegSum sum;
		NegSum total;
		
		for (int i = 0; i < groupSize; i++) {
			if(i > 0) {
				MPI_Recv(&sum, 1, MPI_NEGSUM, i, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
			}
			else {
				sum = calculateNumOfNegativeElements(matrix, startRow, endRow, middle);
			}
			total.botHalf = total.botHalf + sum.botHalf;
			total.topHalf = total.topHalf + sum.topHalf;
		}

		cout << "Top half has " << total.topHalf << " negative elements." << endl;
		cout << "Bot half has " << total.botHalf << " negative elements." << endl;

		if (total.topHalf > total.botHalf) {
			cout << "Top half of array has more negative elements." << endl;
		}
		else if (total.topHalf < total.botHalf) {
			cout << "Bottom half of array has more negative elements." << endl;
		}
		else {
			cout << "Bottom and top parts of array have the same number of negative elements." << endl;
		}
	}


	MPI_Finalize();

	return 0;
}