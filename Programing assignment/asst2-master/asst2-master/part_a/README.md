![result](https://github.com/audrb1999/CS149_assignment/assets/68139415/f9028005-7142-41f8-b72d-95aae7b59afa)

- 현재 thread로 병렬처리하는 문제는 mandelbrot set으로 각 픽셀 값은 복소 평면에서의 좌표에 따라 독립적으로 계산할 수 있기 때문에 종속성이 없습니다.

왜 thread pool을 적용했는데 시간이 더 늦어졌을지 분석해봤습니다.

우선 Thread pool을 적용한 코드와 적용하지 않은 코드를 비교해봤을 때 Thread pool이 적용된 코드는 다음과 같은 부분이 추가되었습니다.


- loop로 인한 지연

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/2af3b1fd-05cb-4a99-b006-c6202fbc17fe)

다음과 같이 loop를 사용하여 지속적으로 작업 완료 여부를 확인하는 부분이 추가되어서 속도가 느려진 것 같습니다. 


- Thread 교체가 없음

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/cc4b14fc-011f-41af-af02-57e2e4e8a95b)

mandelbrot 문제를 할당받는 Thread들은 따로 교체없이 프로그램이 완료될 때까지 진행되어 Thread pool의 속도 향상이 크게 영향을 주기 힘든 구조입니다.(첫 생성된 Thread들이 join된 후 프로그램이 종료됨)





