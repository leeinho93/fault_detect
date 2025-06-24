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

<img width="187" alt="image" src="https://github.com/user-attachments/assets/5d89772a-1292-454f-b3cf-99ab0f2c8ea7" />

