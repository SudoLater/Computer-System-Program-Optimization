# Computer-System-Program-Optimization
main实现了四线程的分块计算，main_o1与main_o2实现了分块计算与sse/avx指令集的优化，注意在编译时需要使用-msse与-mavx编译选项，但是运行速度似乎出现负优化。
