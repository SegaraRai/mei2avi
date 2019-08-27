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

### 1. [EntisGLS](https://www.entis.jp/gls/)の導入

[EntisGLS](https://www.entis.jp/gls/)は無許可での再配布を禁じているため、リポジトリに含めていません。  
最初にEntisGLS version 4s.05をダウンロードし、EntisGLS4s.05ディレクトリをソースツリーのルートに置いてください。  

また、コンパイルを通すためにEntisGLS4s.05内のSource/common/sakura/ssys_std_ui.cppファイルについて、`ConvertFileDialogFilter`関数の宣言と定義の戻り値を`char *`から`const char *`に変更してください（357行目と506行目）。  

### 2. ビルド

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
