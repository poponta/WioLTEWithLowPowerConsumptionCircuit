# WioLTEWithLowPowerConsumptionCircuit

https://technologicaladvance.blog.fc2.com/blog-entry-161.html
で紹介しているWio LTE用低消費電力回路を動作させるためのソースコードです。
センサーから取得したデータをMQTTで送信するので、CommonResource.h内の定数に、MQTTブローカのURIやポート番号、トピック名などを定義してください。
