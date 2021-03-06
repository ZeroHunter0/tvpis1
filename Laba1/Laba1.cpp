#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <random>
#include <condition_variable>

using namespace std;

using Line = vector<double>;
using Matrix = vector<Line>;
vector<thread> threads;

double calcCell(const Matrix &a, pair<int, int> index);
void threadFunc(vector<pair<int, int>> indexes, const Matrix &base, Matrix &result);
condition_variable startCondition;
mutex startMutex;

int activeThreadCount;
mutex threadCountMutex;
condition_variable finishCondition;

int main()
{
	int threadCount = 0;
	int matrixSize = 0;
	cout << "Please enter size of matrix: " << endl;
	cin >> matrixSize;
	cout << "Please enter thread count: " << endl;
	cin >> threadCount;
	Matrix a,res;
	default_random_engine re{};
	normal_distribution<> rng{1, 42};

	a   = Matrix(matrixSize);
	res = Matrix(matrixSize);
	for (int i = 0; i < matrixSize; i++) {
		a[i]   = Line(matrixSize);
		res[i] = Line(matrixSize);
		for (int j = 0; j < matrixSize; j++) {
			a[i][j] = rng(re);
			res[i][j] = 0;
		}
	}
	
	for (int i = 1; i < threadCount; i++) {
		threads.clear();
		threads = vector<thread>(i);
		vector<pair<int, int>> admission;
		int admissionSize = matrixSize / i + 1;
		for (int j = 0; j < i; j++) {
			for (int k = 0; k < admissionSize; k++) {
				auto pos = (j + k * i);
				pair<int, int> index{ 0,0 };
				if (pos < matrixSize) {
					index.first	= pos / matrixSize;
					index.second = pos % matrixSize;
					admission.push_back(index);
				}
			}
			threads[j] = thread{ threadFunc, admission, a, res };
		}
		startCondition.notify_all();
	}
    return 0;
}


double calcCell(const Matrix &a, pair<int, int> index) {
	double result = 0;
	for (int i = 0; i < a.size(); i++) {
		result += a[index.first][i] * a[i][index.second];
	}
	return result;
}

void threadFunc(vector<pair<int, int>> indexes, const Matrix &base, Matrix &result) {
	unique_lock<mutex> lock(startMutex);
	startCondition.wait(lock);
	for (auto i : indexes) {
		result[i.first][i.second] = calcCell(base, i);
	}
	threadCountMutex.lock();
	activeThreadCount--;
	if (activeThreadCount == 0) {
		finishCondition.notify_one();
	}
	threadCountMutex.unlock();
}

