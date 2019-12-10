#include <fstream>
#include <math.h>
#include <iomanip>
#include <iostream>
using namespace std;

#define N 3
#define PI 3.141592653

int main()
{
    double a[2 * N + 1][2 * N + 1];    // 高斯模板;
    double r = 0.5;                     // 高斯半径;
    double A = 1 / (2 * PI * r * r);

	cout << "方差半径r：" << r <<endl;

        for (int i = -1 * N; i <= N; i++)
        {

            for (int j = -1 * N; j <= N; j++)
            {
                a[i + N][j + N] = A*exp((-1)*(i*i + j*j) / (2 * r*r));
                cout << setiosflags(ios::fixed) << setprecision(6) <<  a[i + N][j + N] << "   ";
            }
            cout << endl;
        }
    return 0;
}
