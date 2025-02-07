
```
DenseMapSimd, build: 1015 (optimal), Date: Feb  7 2025 14:33:06
Intel(R) Core(TM) i7-10700K CPU @ 3.80GHz, Comet Lake (Core i7), 8/16, 3.792GHz

DenseMapSimd.exe run [max [min [step]]] [-32|-64] [-all|-map|-ht|-hi] [-test]
-map DenseHashMap, -ht DenseHashTable, -hi DenseHashIndex
-32, -64 key size in bits, -64 by default
-test comparison dataset with a set of keys

DenseMapSimd.exe rnd [32|64|128|256] [-seq]
32|64|128|256  dataset size in MB, 128 by default
-seq, sequential set of numbers
```