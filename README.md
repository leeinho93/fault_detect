# fault_detect


## 파일 구성 정보
1. cied_3_phase_fault_detect_scale_modify
 3상의 전류와 전압 신호를 분석하여 fault 감지

2. sample.cfg
 3상의 전류 전압 구성 파일
  
3. unjeong_modify_9599.dat
 전류, 전압 실제 데이터 파일



## 측정 방법

  <img width="259" alt="image" src="https://github.com/user-attachments/assets/c3f1f324-cb7b-4f39-8ea5-03ecca2b102b" />

N 의 수를 dat 파일의 데이터 개수와 일치시킨 후 컴파일 진행.

<img width="302" alt="image" src="https://github.com/user-attachments/assets/64b18d28-88b7-4dce-9d0a-19db2bf54e78" />

sample.cfg 파일의 전력, 전압 데이터의 4번째 설정 항목에 scale factor 값을 지정.

<img width="166" alt="image" src="https://github.com/user-attachments/assets/1634e038-213a-477c-9eea-6b49a41c7a87" />

3상 전력 데이터, 3상 전압 데이터 순으로 총 6개의 데이터 확인.


## sample 예시 결과

--- MMXU 계측 결과 ---
I_A: 2.47 A @ -2.4 deg
V_A: 0.0 V @ 0.0 deg
I_B: 3.60 A @ 162.0 deg
V_B: 0.0 V @ 0.0 deg
I_C: 4.04 A @ 139.4 deg
V_C: 0.0 V @ 0.0 deg
Freq : 0.00 Hz
ZeroSeq I : 3.37 A
Voltage Unbalance : -nan %

[고조파 분석 - iA (1~9차)]
 1차: 2.474 A
 2차: 2.288 A
 3차: 2.351 A
 4차: 2.944 A
 5차: 1.759 A
 6차: 2.343 A
 7차: 1.306 A
 8차: 2.792 A
 9차: 2.644 A

[고조파 분석 - vA (1~9차)]
 1차: 0.00 V
 2차: 0.00 V
 3차: 0.00 V
 4차: 0.00 V
 5차: 0.00 V
 6차: 0.00 V
 7차: 0.00 V
 8차: 0.00 V
 9차: 0.00 V
