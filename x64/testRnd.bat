@echo off
:SET TEST=-test
DenseMapSimd.exe rnd 128
DenseMapSimd.exe run -ht -64 %TEST%
DenseMapSimdF1.exe run -ht -64 %TEST%
DenseMapSimdF2.exe run -ht -64 %TEST%
DenseMapSimd.exe run -ht -32 %TEST%
DenseMapSimdF1.exe run -ht -32 %TEST%
DenseMapSimdF2.exe run -ht -32 %TEST%