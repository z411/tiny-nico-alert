# ニコ生アラート（小型）
ニコ生アラート小型CLIクライエントです。
今はUNIXやLibreSSL目的で作られましたがWindowsやOpenSSLも対応予定です。

試してみたいならlibtlsをインストールしてmakeでコンパイルしてください。
そしたらconfigファイルにログイン情報を入れてnicoで起動させてください。
初期設定ならアラートが発生した時にlibnotifyで表示されます。

## アカウント設定
* mail: ニコ生ログインメールアドレス
* pass: ニコ生ログインパスワード
* cmd: アラートが発生した時、このコマンドが起動させます。番組の情報は$URL $TITLE $THUMBという環境変数を使ってどうぞ。
