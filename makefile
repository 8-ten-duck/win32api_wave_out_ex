#サフィックスルールの無効にする
MAKEFLAGS += --no-builtin-rules

#生成するプログラム
PROGRAM:=simple_wav_player
#生成に必要なコード類
CODE:=main wav_decode wav_out

#コンパイルに使用するコマンド
CXX:=g++
#コンパイル時共通フラグ
CXXFLAGS:=-Wall -std=c++17 -I./
LDFLAGS:=-lwinmm -lksuser

.PHONY: all
all: ${PROGRAM}.exe

${PROGRAM}.exe: $(addsuffix .o,${CODE})
	${CXX} ${CXXFLAGS} -o $@ $^ ${LDFLAGS}

%.o : %.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $< ${LDFLAGS}

.PHONY: clean
clean:
	-@rm *.o
	-@rm *.exe