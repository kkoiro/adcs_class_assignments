# ADCS_class_assignments
---

## distributed_sorting.c

```
<1からNまでの整数の分散ソート>
Distributed Sort for the numbers between 1 and N
Initial value, 100 cards, N=1,000
Processes: P1-P10
各プロセスは、P1から順に横一列に並んでおり、自分の隣接しているプロセスとのみ交信出来るものと仮定する。
<問題>
1) 1からP10までの各プロセスにランダムに選ばれた10個の整数が初期の値として与えられた時、どのようにしてソートすることができるか?
ただし、P1からP10へは、小さい数から大きい数の順番でソートすること。
2) 各プロセスは、どのような条件で、ソートが終了したことを判定できるか?
```
