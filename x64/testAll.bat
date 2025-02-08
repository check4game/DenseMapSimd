@echo off
SET SEQ=XXXX.128.seq.ht.log
SET RND=XXXX.128.rnd.ht.log
:SET TEST=-test

DenseMapSimd.exe rnd 128 -seq > %SEQ%
DenseMapSimd.exe run -ht -64 %TEST%  >> %SEQ%
DenseMapSimdF1.exe run -ht -64 %TEST%  >> %SEQ%
DenseMapSimdF2.exe run -ht -64 %TEST%  >> %SEQ%
DenseMapSimd.exe run -ht -32 %TEST%  >> %SEQ%
DenseMapSimdF1.exe run -ht -32 %TEST%  >> %SEQ%
DenseMapSimdF2.exe run -ht -32 %TEST%  >> %SEQ%

DenseMapSimd.exe rnd 128 > %RND%
DenseMapSimd.exe run -ht -64 %TEST% >> %RND%
DenseMapSimdF1.exe run -ht -64 %TEST% >> %RND%
DenseMapSimdF2.exe run -ht -64 %TEST% >> %RND%
DenseMapSimd.exe run -ht -32 %TEST% >> %RND%
DenseMapSimdF1.exe run -ht -32 %TEST% >> %RND%
DenseMapSimdF2.exe run -ht -32 %TEST% >> %RND%