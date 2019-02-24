
#include "pch.h"

// Запустить n процессов и найти по вариантам:
// 16.	Произведение суммы положительных элементов первой части векторов на 
// сумму отрицательных элементов второй части вектора.

using namespace std;

const int MAX = 21;

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank, grSize;

	MPI_Comm_size(MPI_COMM_WORLD, &grSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);



	srand(time(nullptr) + rank);

	int mid = MAX / 2;
	int chunk = MAX / grSize;

	int startI = chunk * rank;
	int endI = chunk * (rank + 1);

	if (rank == grSize - 1) {
		endI = MAX;
	}

	int vector[MAX];
	int positiveSum = 0;
	int negativeSum = 0;


	bool haveNegative = false;
	bool havePositive = false;


	for (int i = startI; i < endI; i++) {
		vector[i] = (rand() % 20) - 10;
		cout << "v[" << i << "]=" << vector[i] << endl;

	}

	for (int i = startI; i < endI; i++) {
		if (i > mid) {
			if (vector[i] < 0) {
				negativeSum += vector[i];
				haveNegative = true;
			}
		}
		else {
			if (vector[i] > 0) {
				positiveSum += vector[i];
				havePositive = true;
			}
		}
	}

	if (rank != 0) {
		if (negativeSum != 0) {
			MPI_Send(&negativeSum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
		if (positiveSum != 0) {
			MPI_Send(&positiveSum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}
	else if (rank == 0) {

		int check = mid % chunk;

		int messages = (MAX / chunk) - 1;

		if (check != 0 && (!havePositive && !haveNegative)) {
			messages++;
		}

		int buf;
		int negativeFinalSum = negativeSum;
		int positiveFinalSum = positiveSum;

		MPI_Status status;

		for (int i = 0; i < messages; i++) {
			MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if (buf > 0) {
				positiveFinalSum += buf;
			}
			else if (buf < 0) {
				negativeFinalSum += buf;
			}
		}


		cout << "Left sumR : " << positiveFinalSum << endl;
		cout << "Right sumL : " << negativeFinalSum << endl;
		cout << "sumR*sumL = " << positiveFinalSum * negativeFinalSum << endl;


	}

	MPI_Finalize();
}


