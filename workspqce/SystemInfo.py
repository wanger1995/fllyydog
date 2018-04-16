#-*-coding:utf-8 -*-
# 获取系统的名称，版本号，cpu核数，cpu使用率，内存使用率
# released by hjf
# 2018/4/16

import psutil
import platform
import datetime

os, name, version, _, _, _ = platform.uname()   
#利用platform获取到操作系统的类型和用户名、以及系统版本，后面三项忽略

version = version.split('-')[0] 
#获取到的版本号截取到纯数字的版本号

core = psutil.cpu_count(logical=False)
#获取到cpu的物理核数

cpu_use = psutil.cpu_percent()
#获取cpu的使用率

memory_info = psutil.virtual_memory()
#获取内存信息

memory_use = memory_info[2]
#获取到第三个项，即使用率，前两个分别为总量和使用量

disk_use = psutil.disk_usage('/')[3]
#根目录的的使用率

disk_name = psutil.disk_partitions()[0][0]
#磁盘设备的名字

disk_mount = psutil.disk_partitions()[0][1]
#磁盘设备的挂载点

print "OS:%s, VERSION:%s"%(os,version)
print 'NAME:%s, CPU:%s'%(name,core)+"%"
print "CPU USED:%s"%cpu_use+"%"
print "MEMORY USED:%s"%memory_use+"%"
print "DISK USED:%s"%disk_use+"%"
print "DERVICE:%s"%disk_name
print "MOUNT:%s"%disk_mount
 
