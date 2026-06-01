#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double w, b;

void errorpredictionfunction(double *ptr, double *ptr1, double *ptr2, double *ptr3, int n)
{

    for (int i = 0; i < n; i++)
{
    ptr2[i] = w * ptr[i] + b;

    double error = ptr1[i] - ptr2[i];

    ptr3[i] = fabs((error / ptr1[i]) * 100);

    w = w + 0.01 * error;
    b = b + 0.01 * error;
    printf("w = %.3f b = %.3f\n", w, b);

}

    printf("prediction_array : ");
    for (int i = 0; i < (n); i++)
    {
        printf("%.3f ", ptr2[i]);
    }

    printf("\n");

    printf("errorpercentage_array : ");
    for (int i = 0; i < (n); i++)
    {
        printf("%.3f ", ptr3[i]);
    }
    printf("\n");

}

int main()
{
    double array[] = {2, 4, 6, 8, 10};
    double answer[] = {5, 9, 13, 17, 21};
    double prediction[5] = {0, 0, 0, 0, 0};
    double errorpercentage[5] = {100, 100, 100, 100, 100};
    int n = sizeof(array) / sizeof(double);

    printf("enter your w and b for testing followed by a space :\n ");
    scanf("%lf %lf", &w, &b);

    printf("prediction_array : ");
    for (int i = 0; i < (n); i++)
    {
        printf("%.3f ", prediction[i]);
    }

    printf("\n");

    printf("error_array : ");
    for (int i = 0; i < (n); i++)
    {
        printf("%.3f ", errorpercentage[i]);
    }
    printf("\n");

    for (int i = 1; i < 1000; i++)
    {
        errorpredictionfunction(array, answer, prediction, errorpercentage, n);

        int all_zero = 1;
        for (int j = 0; j < n; j++)
        {
            if (errorpercentage[j] !=0.0)
            {
                all_zero = 0;
                break;
            }
        }
        if (all_zero)
        {
            break;
        }
    }
}