cc branch:

2016-12-17 12:51
1.更改了一些列bug
2.修改level control 代码，使其操作和开关类似。
3.放开了send_basic_request函数，并做了一定的修改。
4.把zcl_SendRead函数的第二个参数由原来的1改为epnum,解决了飞比设备离线的bug
5.将send_read_onoff改名为send_read_state

2016-10-21 16:48
该分支采用了新轮询，过滤无关设备的上报信息，加大锁的方式。

