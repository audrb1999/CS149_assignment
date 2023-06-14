>  Thread로 병렬처리하는 문제는 mandelbrot set으로 각 픽셀 값은 복소 평면에서의 좌표에 따라 독립적으로 계산할 수 있기 때문에 종속성이 없습니다.

</br>

Parallel + Always Spawn을 구현할 때 생겼던 문제들입니다.

</br>

## 병렬처리가 되지 않고 순차적으로 진행되는 문제
</br>


![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/5a724c40-b5e7-4f56-9bc9-08b4f13361b0)

</br>
</br>

주어진 입력만큼 Thread를 생성하고 작업을 할당했지만 기존 Serial 코드보다 더 느려졌습니다. ( Parallel: 389.440 ms, Serial: 346.286 ms)

</br>
</br>

- 원인: for loop 사용

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/e7202dd7-fee1-41c3-9e4a-9ee4bcf90e1f)

</br>
</br>

Open MP와 같은 라이브러리와 다르게 C++ STL은 for loop는 순차적으로 실행시켜 한 번에 한 개의 loop만을 실행하지만 While은 조건을 만족할 경우 계속 실행되어 Parallel하게 동작하기 때문에 발생했던 문제입니다.

</br>

추가 정리 URL

> https://north-stranger-00a.notion.site/Programming-Assignment-2-d01b7f1762504c5ea5268dce7fe314ea?pvs=4
</br>

## Thread pool의 속도 
</br>

![result](https://github.com/audrb1999/CS149_assignment/assets/68139415/f9028005-7142-41f8-b72d-95aae7b59afa)


</br>
</br>

왜 thread pool을 적용했을 때 시간이 더 늦어졌을지 분석해봤습니다.

우선 Thread pool을 적용한 코드와 적용하지 않은 코드를 비교해봤을 때 Thread pool이 적용된 코드는 다음과 같은 부분이 추가되었습니다.

</br>
</br>

### loop로 인한 지연

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/2af3b1fd-05cb-4a99-b006-c6202fbc17fe)
</br>
</br>

다음과 같이 loop를 사용하여 지속적으로 작업 완료 여부를 확인하는 부분이 추가되어서 느려진 것 같습니다.
- 이 추가된 코드부분에서 CPU 사용량이 발생했습니다.

</br>
</br>

### Thread 교체가 없음

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/cc4b14fc-011f-41af-af02-57e2e4e8a95b)
</br>
</br>

mandelbrot 문제를 할당받는 Thread들은 따로 교체없이 프로그램이 완료될 때까지 진행되어 Thread pool이 속도 향상에 영향을 주기 힘든 구조입니다.(첫 생성된 Thread들이 128번 연산하고 join된 후 프로그램이 종료됩니다.)

</br>
</br>

loop로 진행할 작업을 지속적으로 확인하는 spin 기능은 과제에서 요구하는 기능이기 때문에 Parallel + Thread pool + Spin 부분에서 이로 인한 지연은 해결할 수 없습니다.

</br>

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/d8f2adf0-d436-45d5-a274-3ef2f5996d95)

</br>
</br>

이 spin으로 인한 시간 지연은 다음 과제인 Parallel + Thread pool + Sleep에서 sleep 기능을 통해서 lock을 얻지 못한 Thread를 절전지켜 CPU 리소스 소비를 줄여서 해결한 것 같습니다.

</br>
</br>

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/b9f584ff-8cde-4890-a0cf-a93150a3daaf)

</br>

결론

- spin은 새 작업을 진행할 수 있을 때까지 지속적으로 확인을 하면서 대기하기 때문에 작업을 진행할 수 없는 기간이 길어지면 CPU 시간이 낭비되지만 sleep은 conditional variable로 새 작업을 사용할 수 있다는 알림을 받을 때만 확인을 하기 때문에 낭비되는 시간이 적습니다.

- 구현은 spin이 더 쉽지만 효율적이지 못합니다.

- Thread pool을 구현하고 속도 지연을 sleep으로 해결하는 과정을 요구하는 과제인 것 같습니다.

</br>

+ 추가 

![image](https://github.com/audrb1999/CS149_assignment/assets/68139415/3291ee7c-8a87-4fc3-8a17-5f38f1228c8c)

</br>

과제에서 올바른 예시로 비교해보라고 제공해준 out 파일인데 여기서도 Parallel + Thread pool + Spin이 느리게 나왔습니다.








