# mei2avi

MEIファイル（[ERIフォーマット](https://www.entis.jp/eridev/)）を無圧縮AVIファイルに変換するプログラムです。  

## 使い方

### AVIファイルに出力する

以下の様に実行するとvideo.meiをvideo.aviに変換できます。  
出力されるAVIファイルは32ビットRGB（A）の無圧縮データなので容量にお気をつけください。  
通常はこちらではなく、FFmpegを用いて変換するのが良いでしょう。  

```bat
mei2avi.exe video.mei video.avi
```

### [FFmpeg](https://ffmpeg.org/)を用いて変換する

mei2aviでは出力ファイルに`-`を指定することで変換したデータを標準出力に出力できます。  
これを[FFmpeg](https://ffmpeg.org/)とパイプで繋げることで、ファイルを経由せずにMEIファイルを他の形式に変換できます。  

以下にはよく使うと思われる例を示しますが、ここに挙げた以外の変換も可能です。  

#### MP4に変換する

video.meiをcrf値18でvideo.mp4に変換します。  

```bat
mei2avi.exe video.mei - | ffmpeg -i pipe:0.avi -crf 18 video.mp4
```

#### 音声をWAVで取り出す

video.meiの音声をvideo.wavに取り出します。  

```bat
mei2avi.exe video.mei - | ffmpeg -i pipe:0.avi -vn -c:a copy video.wav
```

### [FFplay](https://ffmpeg.org/ffplay.html)で再生する

以下の様に実行するとvideo.meiを[FFplay](https://ffmpeg.org/ffplay.html)で再生できます。  

```bat
mei2avi.exe video.mei - | ffplay pipe:0.avi
```

## ビルド方法

### 1. リポジトリのクローン

このリポジトリをクローンします。  
その際、再帰的に（サブモジュールも含めて）かつ改行コードはLFで（gitconfigで`core.autocrlf=input`にして）クローンしてください。  

### 2. [EntisGLS](https://www.entis.jp/gls/)の導入

[EntisGLS4Build](https://github.com/SegaraRai/EntisGLS4Build)の[README.mdのビルド手順](https://github.com/SegaraRai/EntisGLS4Build#%E3%83%93%E3%83%AB%E3%83%89%E6%89%8B%E9%A0%86)を参照してリポジトリのEntisGLS4Buildディレクトリ内に[EntisGLS](https://www.entis.jp/gls/)を導入してください。  

### 3. ビルド

Visual Studio 2019でmei2avi.slnを開き、ソリューションをビルドします。  

実行ファイルはBuild/Win32/DebugまたはBuild/Win32/Releaseディレクトリに出力されます。  

## TODO

- [ ] アルファチャンネル付きデータの動作確認
- [ ] YUV色空間での出力
- [ ] パイプ等からのmeiファイルの入力
- [ ] 名前付きパイプへの出力
- [ ] Windows以外のプラットフォームへの対応
- [ ] x64版の作成

## ライセンス

mei2aviはMIT Licenseのもと配布されます。  
詳しくはLICENSE.txtをご覧ください。  

ただし、使用しているライブラリについてはそのライブラリのライセンスが適用されます。  

## ライブラリ著作権表示

mei2aviは[EntisGLS](https://www.entis.jp/gls/)を使用しています。  

> EntisGLS version 4s.05  
> Copyright (C) 1998-2014 理影, Entis soft.  
