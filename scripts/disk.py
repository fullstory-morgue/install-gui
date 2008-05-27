#!/usr/bin/python -tt
# -*- coding: utf-8 -*-

__author__    = 'Horst Tritremmel'
__copyright__ = '(C) 2008 Horst Tritremmel <hjt@sidux.com>'
__license__   = 'GPLv2 or any later version'

import os
import glob
import volumeid
import logging
from stat import *
from optparse import OptionParser
from subprocess import *

PROC_PARTITIONS = '/proc/partitions'
SYS_BLOCK       = '/sys/block'
UDEVINFO_CMD    = 'udevinfo --query=env --name='
LOG_FILE        = '/tmp/install-gui.log'


class Diskinfo(object):
    def __init__(self):
        '''setup logging'''
        self.log = logging.getLogger('disk.py in install-gui')
        self.log.setLevel(logging.DEBUG)
        logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(name)s %(levelname)s %(message)s',
                    datefmt='%m-%d %H:%M',
                    filename=LOG_FILE,
                    filemode='w')


    def partitions(self):
        '''
        open and read procfile
        get procnames = ['sda', 'sda1', 'sdb', 'sdb1', ... ]
        '''
        self.procfile = open(PROC_PARTITIONS, 'r')
        self.procnames = [ '/dev/%s' % (self.p.split()[3]) for self.p in self.procfile.readlines()[2:] ]
        self.procfile.close()

        self.log.debug('def partitions, procnames: %s\n' % (self.procnames) )
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
            self.log.debug('Error: %s' % ( ' '.join(self.cmd) ))
            pass


        ''' split udevinfo and create dict '''
        for self.u in self.udevinfo:
            if '=' in self.u:
                self.v = self.u.split('=')
                self.dict_udevinfo[self.v[0]] = self.v[1]
            else:
                continue

        self.log.debug('def udevinfo, device: %s\n dict_udevinfo: %s\n' % (self.device, self.dict_udevinfo) )
        return self.dict_udevinfo

    """
    def lvm(self):
        lvm = glob.glob('/dev/mapper/*')
        for l in lvm:
            try:
                mode = os.stat(l)[ST_MODE]
                if S_ISBLK(mode) == 0:
                    continue

                vid = volumeid.VolId(l)
                if vid.type() != 'swap' and \
                   vid.usage() == 'filesystem':
                    print '%s,%s,%s,%s' % ( 
                        vid.dev,
                        vid.type(),
                        vid.usage(),
                        vid.uuid_enc(),
                    )
            except volumeid.error, e:
                print 'E: %s' % e
                pass
    """

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


    def fdisk(self, partition):
        ''' return len of <fdisk -l> call '''
        self.cmd = [ 'fdisk', '-l', partition ]
        self.c = Popen(self.cmd, stdout = PIPE, stderr = STDOUT, close_fds = True)
        self.callback = self.c.communicate()[0]
        if not self.c.returncode == 0:
            self.log.debug( 'Error: %s' % ( ' '.join(self.cmd) ) )

        self.log.debug('def fdisk, callback: %s\n' % (self.callback) )
        return len(self.callback)


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

    ''' print all disk devices, option: -d '''
    if opt_disk == True:
        for p in partitions:
            if Diskinfo().udevinfo(p).get('TYP') == 'disk':
                if Diskinfo().udevinfo(p).get('ID_BUS') != None or \
                   'dm-' in p:
                    print '%s' % (p)


    ''' print all partition devices, option: -p '''
    if opt_partiton == True:
        for p in partitions:
            if Diskinfo().udevinfo(p).get('TYP') == 'partition' and \
            Diskinfo().udevinfo(p).get('ID_FS_TYPE') != 'swap'  and \
            Diskinfo().udevinfo(p).get('ID_FS_USAGE') == 'filesystem':
                len_of_fdisk_call = Diskinfo().fdisk(p)
                if len_of_fdisk_call > 0:
                    print '%s,%s,%s,%s,norm' % (
                            p,
                            Diskinfo().udevinfo(p).get('ID_FS_TYPE'),
                            Diskinfo().udevinfo(p).get('ID_FS_USAGE'),
                            Diskinfo().udevinfo(p).get('ID_FS_UUID')
                        )

            # lvm devices
            if 'dm-' in p:
                vid = volumeid.VolId(p)
                if vid.type() != 'swap' and \
                   vid.usage() == 'filesystem':
                    print '%s,%s,%s,%s,lvm' % ( 
                        vid.dev,
                        vid.type(),
                        vid.usage(),
                        vid.uuid_enc(),
                    )

        # output lvm devices
        #Diskinfo().lvm()


    ''' print all disk devices without usb, option: -n '''
    if opt_nousb == True:
        for p in partitions:
            if Diskinfo().udevinfo(p).get('TYP') == 'disk' and \
            Diskinfo().udevinfo(p).get('ID_BUS') != 'usb':
                if Diskinfo().udevinfo(p).get('ID_BUS') != None:
                    print '%s' % (p)

    ''' is the <disk device> a usb device, option: -u <device> '''
    if opt_usb != None:
        p = '/dev/%s' % (opt_usb)

        if Diskinfo().udevinfo(p).get('TYP') == 'disk' and \
           Diskinfo().udevinfo(p).get('ID_BUS') == 'usb':
                print '%s' % (p)
