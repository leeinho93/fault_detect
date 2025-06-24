/*
 * CIED 3상 계측 + GOOSE 연계 + 지락/불평형 검출 포함 IEC 61850 기반 계측 로직
 * COMTRADE .cfg 파일 연계 + DC 옵셋 제거 + 저역통과필터 + 고조파 분석 포함
 * 작성자: Dongwoo R&D
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 9599
#define PI 3.141592
#define MAX_LINE 256
#define ALPHA 0.9
#define MAX_HARMONICS 9

// SV 버퍼
double iA[N], iB[N], iC[N];
double vA[N], vB[N], vC[N];
double iA_f[N], iB_f[N], iC_f[N];
double vA_f[N], vB_f[N], vC_f[N];

double iA_scale[3], iV_scale[3];

typedef struct {
    double I_rms[3], V_rms[3];
    double I_angle[3], V_angle[3];
    double freq;
    double zero_seq_I;
    double unbalance_V;
    double iA_harmonics[MAX_HARMONICS];
    double vA_harmonics[MAX_HARMONICS];
} MMXU;

MMXU mmxu;

void dft_harmonic(double x[], int k, double* mag) {
    double re = 0, im = 0;
    for (int n = 0; n < N; n++) {
        double theta = 2 * PI * k * n / N;
        re += x[n] * cos(theta);
        im -= x[n] * sin(theta);
    }
    *mag = sqrt(re*re + im*im) / sqrt(2);
}

void dft(double x[], double* rms, double* angle_deg) {
    dft_harmonic(x, 1, rms);
    double re = 0, im = 0;
    for (int n = 0; n < N; n++) {
        double theta = 2 * PI * n / N;
        re += x[n] * cos(theta);
        im -= x[n] * sin(theta);
    }
    *angle_deg = atan2(im, re) * 180 / PI;
}

double estimate_freq(double x[]) {
    int cross = 0;
    for (int n = 1; n < N; n++)
        if (x[n-1] <= 0 && x[n] > 0) cross++;
    return (double)cross * 4000.0 / (2.0 * N);
}

double calc_unbalance(double va, double vb, double vc) {
    double avg = (va + vb + vc) / 3.0;
    double max_dev = fmax(fabs(va - avg), fmax(fabs(vb - avg), fabs(vc - avg)));
    return (max_dev / avg) * 100.0;
}

double calc_zero_seq(double ia, double ib, double ic) {
    return (ia + ib + ic) / 3.0;
}

void goose_publish_trip(double cause) {
    printf("[GOOSE] 보호 Trip 송신: 원인코드 %.1f\n", cause);
}

void apply_filters(double *in, double *out) {
    double sum = 0;
    for (int i = 0; i < N; i++) sum += in[i];
    double mean = sum / N;
    out[0] = in[0] - mean;
    for (int i = 1; i < N; i++)
        out[i] = ALPHA * out[i-1] + (1 - ALPHA) * (in[i] - mean);
}

void harmonic_analysis(double* x, double* harmonics) {
    for (int k = 1; k <= MAX_HARMONICS; k++) {
        dft_harmonic(x, k, &harmonics[k-1]);
    }
}

void mmxu_measure() {
    apply_filters(iA, iA_f); apply_filters(iB, iB_f); apply_filters(iC, iC_f);
    apply_filters(vA, vA_f); apply_filters(vB, vB_f); apply_filters(vC, vC_f);

    dft(iA_f, &mmxu.I_rms[0], &mmxu.I_angle[0]);
    dft(iB_f, &mmxu.I_rms[1], &mmxu.I_angle[1]);
    dft(iC_f, &mmxu.I_rms[2], &mmxu.I_angle[2]);
    dft(vA_f, &mmxu.V_rms[0], &mmxu.V_angle[0]);
    dft(vB_f, &mmxu.V_rms[1], &mmxu.V_angle[1]);
    dft(vC_f, &mmxu.V_rms[2], &mmxu.V_angle[2]);

    harmonic_analysis(iA_f, mmxu.iA_harmonics);
    harmonic_analysis(vA_f, mmxu.vA_harmonics);

    mmxu.freq = estimate_freq(vA_f);
    mmxu.zero_seq_I = calc_zero_seq(mmxu.I_rms[0], mmxu.I_rms[1], mmxu.I_rms[2]);
    mmxu.unbalance_V = calc_unbalance(mmxu.V_rms[0], mmxu.V_rms[1], mmxu.V_rms[2]);

    if (mmxu.zero_seq_I > 5.0) goose_publish_trip(87.1);
    if (mmxu.unbalance_V > 3.0) goose_publish_trip(46.2);
}

void print_mmxu() {
    printf("--- MMXU 계측 결과 ---\n");
    for (int i = 0; i < 3; i++) {
        printf("I_%c: %.2f A @ %.1f deg\n", 'A'+i, mmxu.I_rms[i], mmxu.I_angle[i]);
        printf("V_%c: %.1f V @ %.1f deg\n", 'A'+i, mmxu.V_rms[i], mmxu.V_angle[i]);
    }
    printf("Freq : %.2f Hz\n", mmxu.freq);
    printf("ZeroSeq I : %.2f A\n", mmxu.zero_seq_I);
    printf("Voltage Unbalance : %.2f %%\n", mmxu.unbalance_V);

    printf("\n[고조파 분석 - iA (1~9차)]\n");
    for (int k = 0; k < MAX_HARMONICS; k++)
        printf("%2d차: %.3f A\n", k+1, mmxu.iA_harmonics[k]);
    printf("\n[고조파 분석 - vA (1~9차)]\n");
    for (int k = 0; k < MAX_HARMONICS; k++)
        printf("%2d차: %.2f V\n", k+1, mmxu.vA_harmonics[k]);
}

int parse_cfg_file(const char* cfg_filename, int* i_idx, int* v_idx) {
    FILE* fp = fopen(cfg_filename, "r");
    if (!fp) { perror(".CFG 파일 열기 실패"); return -1; }
    char line[MAX_LINE]; int ana_count = 0; int found = 0;
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    sscanf(line, "%*d,%d", &ana_count);       

    int scale_i_index = 0, scale_v_index = 0;
    for (int i = 0; i < ana_count; i++) {
        fgets(line, sizeof(line), fp);


        if (strstr(line, "I")) {
            char* token;
            char* rest = line;
            int field = 0;
            while ((token = strtok_r(rest, ",", &rest))) {
                field++;
                if (field == 4) {
                    iA_scale[scale_i_index++] = atof(token);
                    break;
                }
            }
            *i_idx = i;

        }
        if (strstr(line, "V")) {
            char* token;
            char* rest = line;
            int field = 0;
            while ((token = strtok_r(rest, ",", &rest))) {
                field++;
                if (field == 4) {
                    iV_scale[scale_v_index++] = atof(token);
                    break;
                }
            }
            *v_idx = i;
        }
         
        if (*i_idx >= 0 && *v_idx >= 0) { found = 1; }
    }

    //printf("scale factor : %.10f Ia, %.10f Ib, %.10f Ic, %.10f Va, %.10f Vb, %.10f Vc,\n ", iA_scale[0], iA_scale[1], iA_scale[2], iV_scale[0], iV_scale[1], iV_scale[2]);

    fclose(fp);
    return found ? 0 : -1;
}

int read_comtrade_input(const char* dat_filename) {
    FILE* fp = fopen(dat_filename, "r");
    if (!fp) { perror("DAT 파일 열기 실패"); return -1; }
    char line[MAX_LINE]; int n = 0;
    while (fgets(line, sizeof(line), fp) && n < N) {
        sscanf(line, "%*d,%lf,%lf,%lf,%lf,%lf,%lf",
               &iA[n], &iB[n], &iC[n], &vA[n], &vB[n], &vC[n]);
        n++;
    }
    fclose(fp);


    for (int i=0; i<N; i++) {
        iA[i] *= iA_scale[0];
        iB[i] *= iA_scale[1];
        iC[i] *= iA_scale[2];
        vA[i] *= iV_scale[0];
        vB[i] *= iV_scale[1];
        vC[i] *= iV_scale[2]; 
    }


    return (n == N) ? 0 : -2;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("사용법: %s <파일.cfg> <파일.dat>\n", argv[0]);
        return 1;
    }
    int i_idx = -1, v_idx = -1;
    if (parse_cfg_file(argv[1], &i_idx, &v_idx) != 0) {
        printf(".CFG 채널 파싱 실패!\n"); return 1;
    }
    if (read_comtrade_input(argv[2]) != 0) {
        printf("DAT 데이터 파일 읽기 실패!\n"); return 1;
    }
    mmxu_measure();
    print_mmxu();
    return 0;
}
