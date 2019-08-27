// このファイルは利便性のためにmei2aviのプロジェクトに追加してありますが、mei2aviからは使用されません
// EntisGLS4のコンパイルにおいてのみ使用されます（強制的に読ませます）
// x64版のコンパイルを通すためのハック用です

#ifdef _WIN64

#define _InterlockedAdd InterlockedAdd

#endif
