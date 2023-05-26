>  Thread로 병렬처리하는 문제는 mandelbrot set으로 각 픽셀 값은 복소 평면에서의 좌표에 따라 독립적으로 계산할 수 있기 때문에 종속성이 없습니다.

</br>

Parallel + Always Spawn을 구현할 때 생겼던 문제들입니다.

</br>

### 병렬처리가 되지 않고 순차적으로 진행되는 문제

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/5a724c40-b5e7-4f56-9bc9-08b4f13361b0)

주어진 입력만큼 Thread를 생성하고 작업을 할당했지만 기존 Serial 코드보다 더 느려졌습니다. ( Parallel: 389.440 ms, Serial: 346.286 ms)

</br>

- 원인: for loop 사용

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/e7202dd7-fee1-41c3-9e4a-9ee4bcf90e1f)

Open MP와 같은 라이브러리와 다르게 C++ STL은 for loop는 순차적으로 실행시켜 한 번에 한 개의 loop만을 실행하지만 While은 조건을 만족할 경우 계속 실행되어 Parallel하게 동작하기 때문에 발생했던 문제입니다.

</br>
</br>

## Result

![result](https://github.com/audrb1999/CS149_assignment/assets/68139415/f9028005-7142-41f8-b72d-95aae7b59afa)


</br>
</br>

왜 thread pool을 적용했을 때 시간이 더 늦어졌을지 분석해봤습니다.

우선 Thread pool을 적용한 코드와 적용하지 않은 코드를 비교해봤을 때 Thread pool이 적용된 코드는 다음과 같은 부분이 추가되었습니다.

</br>
</br>

### loop로 인한 지연

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/2af3b1fd-05cb-4a99-b006-c6202fbc17fe)

다음과 같이 loop를 사용하여 지속적으로 작업 완료 여부를 확인하는 부분이 추가되어서 느려진 것 같습니다. 

</br>
</br>

### Thread 교체가 없음

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/cc4b14fc-011f-41af-af02-57e2e4e8a95b)

mandelbrot 문제를 할당받는 Thread들은 따로 교체없이 프로그램이 완료될 때까지 진행되어 Thread pool이 속도 향상에 영향을 주기 힘든 구조입니다.(첫 생성된 Thread들이 128번 연산하고 join된 후 프로그램이 종료됩니다.)





