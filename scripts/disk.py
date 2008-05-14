#!/usr/bin/python -tt
# -*- coding: utf-8 -*-

__author__    = 'Horst Tritremmel'
__copyright__ = '(C) 2008 Horst Tritremmel <hjt@sidux.com>'
__license__   = 'GPLv2 or any later version'

import os
from optparse import OptionParser
from subprocess import *

PROC_PARTITIONS = '/proc/partitions'
SYS_BLOCK       = '/sys/block'
UDEVINFO_CMD    = 'udevinfo --query=env --name='


class Diskinfo(object):
    def partitions(self):
        '''
        open and read procfile
        get procnames = ['sda', 'sda1', 'sdb', 'sdb1', ... ]
        '''
        self.procfile = open(PROC_PARTITIONS, 'r')
        self.procnames = [ '/dev/%s' % (self.p.split()[3]) for self.p in self.procfile.readlines()[2:] ]
        self.procfile.close()
        self.procnames.remove("/dev/loop0")

        return self.procnames


    def udevinfo(self, device):
        '''
        get disk or partition info
        INFO: get all keys from udevinfo      Diskinfo().udevinfo('/dev/sda1').keys()
        '''
        self.device = device.split('/')[2]
        self.dict_udevinfo = {}
        self.dict_udevinfo.clear()

        ''' top-level device path /sys/block (for disk or partiton) '''
        self.sysblock = os.listdir(SYS_BLOCK)


        if self.device in self.sysblock:
            ''' disk '''
            self.dict_udevinfo['TYP'] = 'disk'
        else:
            ''' partition '''
            self.dict_udevinfo['TYP'] = 'partition'

        ''' start udevinfo comand '''
        self.cmd = ("%s%s" % (UDEVINFO_CMD, self.device)).split()
        self.c = Popen(self.cmd, stdout = PIPE, stderr = STDOUT, close_fds = True)
        self.udevinfo = self.c.communicate()[0].split('\n')
        if not self.c.returncode == 0:
            print 'Error: %s' % ( ' '.join(self.cmd) )


        ''' split udevinfo and create dict '''
        for self.u in self.udevinfo:
            if '=' in self.u:
                self.v = self.u.split('=')
                self.dict_udevinfo[self.v[0]] = self.v[1]
            else:
                continue

        return self.dict_udevinfo


    def partition_count(self):
        '''
        count partitions
        '''
        self.count = 0

        for self.dev in self.partitions():
            if Diskinfo().udevinfo(self.dev).get('TYP') == 'partition':
                self.count = self.count + 1
                print self.dev

        return self.count


if __name__ == '__main__':
    ''' option parser'''
    parser = OptionParser("disk.py [Option]")
    parser.add_option("-d", "--disk", action="store_true", 
                  dest="disk", default=False,
                  help="print all disk devices")

    parser.add_option("-p", "--partition", action="store_true", 
                  dest="partition", default=False,
                  help="print all partition devices")

    parser.add_option("-n", "--nousb", action="store_true", 
                  dest="nousb", default=False,
                  help="print all disk devices without usb")

    parser.add_option("-u", "--usb", dest="usb",
                      type = 'string', metavar = '<device>',
                      help="[--usb=sdX], is the <disk device> a usb device")

    (options, args) = parser.parse_args()

    opt_disk     = options.disk
    opt_partiton = options.partition
    opt_nousb    = options.nousb
    opt_usb      = options.usb


    # opt_partition as default
    if opt_disk     == False and \
       opt_partiton == False and \
       opt_usb      == None  and \
       opt_nousb    == False:
            opt_partiton = True


    ''' start main '''
    partitions = Diskinfo().partitions()

    ''' print all disk devices '''
    if opt_disk == True:
        for p in partitions:
            if Diskinfo().udevinfo(p).get('TYP') == 'disk':
                print '%s' % (p)

    ''' print all partition devices '''
    if opt_partiton == True:
        for p in partitions:
            if Diskinfo().udevinfo(p).get('TYP') == 'partition' and \
            Diskinfo().udevinfo(p).get('ID_FS_TYPE') != 'swap':
                print '%s,%s,%s' % (p, Diskinfo().udevinfo(p).get('ID_FS_TYPE'), Diskinfo().udevinfo(p).get('ID_BUS'))

    ''' print all disk devices without usb '''
    if opt_nousb == True:
        for p in partitions:
            if Diskinfo().udevinfo(p).get('TYP') == 'disk' and \
            Diskinfo().udevinfo(p).get('ID_BUS') != 'usb':
                print '%s' % (p)

    if opt_usb != None:
        p = '/dev/%s' % (opt_usb)
        if Diskinfo().udevinfo(p).get('TYP') == 'disk' and \
        Diskinfo().udevinfo(p).get('ID_BUS') == 'usb':
                print '%s' % (p)