#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib/svpng.inc"

#define FILENAME "SSS.png" //输出文件的名字
#define PI 180.0                // PI 为90 时是在半圆上进行积分，也可以在PI = 180在整个圆上积分
#define SIZE 512                //图片像素 长/宽
#define ITERATION 0.05          //迭代参数，越小越精确，一般在0.05左右
#define LINAR 0                 //是否线性空间在输出
#define REMOVE_BLUE 1           //是否进行除青

struct float3 {
    double x;
    double y;
    double z;
};

double saturate(double num) {
    if (num < 0) return 0;
    if (num > 1) return 1;
    return num;
}

double Gaussian(double v, double r) {
    v *= 1.414;
    return 1.0 / sqrt(2.0 * PI * v) * exp(-(r * r) / (2 * v));
}

void Scatter(double sampleDist, struct float3* weight) {

    weight->x = 0; weight->y = 0; weight->z = 0;
    double gaussian = Gaussian(0.0064, sampleDist);
    weight->x += 0.233 * gaussian; weight->y += 0.455 * gaussian; weight->z += 0.649 * gaussian;
    gaussian = Gaussian(0.0484, sampleDist);
    weight->x += 0.100 * gaussian; weight->y += 0.336 * gaussian; weight->z += 0.344 * gaussian;
    gaussian = Gaussian(0.1870, sampleDist);
    weight->x += 0.118 * gaussian; weight->y += 0.198 * gaussian; weight->z += 0.000 * gaussian;
    gaussian = Gaussian(0.5670, sampleDist);
    weight->x += 0.113 * gaussian; weight->y += 0.007 * gaussian; weight->z += 0.007 * gaussian;
    gaussian = Gaussian(1.9900, sampleDist);
    weight->x += 0.358 * gaussian; weight->y += 0.004 * gaussian; weight->z += 0.000 * gaussian;
    gaussian = Gaussian(7.4100, sampleDist);
    weight->x += 0.078 * gaussian; weight->y += 0.000 * gaussian; weight->z += 0.000 * gaussian;
}

struct float3 integrateDiffuseScatteringOnRing(double cosTheta, double skinRadius) {
    double theta = acos(cosTheta);
    struct float3 totalWeights; totalWeights.x = 0; totalWeights.y = 0; totalWeights.z = 0;
    struct float3 totalLight; totalLight.x = 0; totalLight.y = 0; totalLight.z = 0;

    double a = -(PI);
    while (a <= (PI)) 
        while (a <= (PI)) {
            double sampleAngle = theta + a;
            double diffuse = saturate(cos(sampleAngle));
            double sampleDist = (2.0 * skinRadius * sin(a * 0.5));

            struct float3 weight;
            Scatter(sampleDist, &weight);

            totalWeights.x += weight.x; totalWeights.y += weight.y; totalWeights.z += weight.z;
            totalLight.x += diffuse * weight.x; totalLight.y += diffuse * weight.y; totalLight.z += diffuse * weight.z;
            a += ITERATION;
        }

    struct float3 res; 
    res.x = totalLight.x/totalWeights.x; res.y = totalLight.y/totalWeights.y; res.z = totalLight.z/totalWeights.z;
    return res;
}


unsigned char rgb[SIZE * SIZE * 3], * p = rgb;

void SSS(void) {
    struct float3 res;
    FILE* fp = fopen(FILENAME, "wb");
    for (double y = SIZE; y >= 0; y--) {
        for (double x = 0; x < SIZE; x++) {
            //printf("%f %f\n", x / size, y / size);
            res = integrateDiffuseScatteringOnRing((x / SIZE) * 2 - 1, 1.0 / (y / SIZE + 0.001));

            if (!LINAR) {
                //非线性情况
                // res.z > res.x 的 if 语句用来除青
                if (REMOVE_BLUE && res.z > res.x) {
                    *p++ = (unsigned char)(255 * pow(res.z, 1.0 / 2.2));    /* R */
                    *p++ = (unsigned char)(255 * pow(res.z, 1.0 / 2.2));    /* G */
                } else {
                    *p++ = (unsigned char)(255 * pow(res.x, 1.0 / 2.2));    /* R */
                    *p++ = (unsigned char)(255 * pow(res.y, 1.0 / 2.2));    /* G */
                }
                *p++ = (unsigned char)(255 * pow(res.z, 1.0 / 2.2));    /* B */

            } else {
                //线性情况
                // res.z > res.x 的 if 语句用来除青
                if (REMOVE_BLUE && res.z > res.x) {
                    *p++ = (unsigned char)(255 * res.z);    /* R */
                    *p++ = (unsigned char)(255 * res.z);    /* G */
                } else {
                    *p++ = (unsigned char)(255 * res.x);    /* R */
                    *p++ = (unsigned char)(255 * res.y);    /* G */
                }
                *p++ = (unsigned char)(255 * res.z);    /* B */
            }
        }
        printf("\rprocess: %f %%", (1.0 - y / SIZE) * 100);
    }
    printf("\nCOMPLETED!!!");
    svpng(fp, SIZE, SIZE, rgb, 0);
    fclose(fp);
}

int main(void) {
    printf("-----------------------------------\n");
    printf("FILE NAME   : %s\n", FILENAME);
    printf("   SIZE     : %d\n", SIZE);
    printf("   LINAR    : %s\n", LINAR ? "YES" : "NO");
    printf("REMOVE BLUE : %s\n", REMOVE_BLUE ? "YES" : "NO");
    printf(" ITERATION  : %f\n", ITERATION);
    printf("    PI      : %f\n", PI);
    printf("-----------------------------------\n");
    SSS();
    printf("\n");
    system("Pause");
    return 0;
}