#!/usr/bin/env python
# Copyright (c) 2013-2016 The Gulden Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
from __future__ import division,print_function,unicode_literals
import biplist
from ds_store import DSStore
from mac_alias import Alias
import sys

output_file = sys.argv[1]
package_name_ns = sys.argv[2]

ds = DSStore.open(output_file, 'w+')
ds['.']['bwsp'] = {
    'ShowStatusBar': False,
    'WindowBounds': b'{{300, 280}, {500, 343}}',
    'ContainerShowSidebar': False,
    'SidebarWidth': 0,
    'ShowTabView': False,
    'PreviewPaneVisibility': False,
    'ShowToolbar': False,
    'ShowSidebar': False,
    'ShowPathbar': True
}

icvp = {
    'gridOffsetX': 0.0,
    'textSize': 12.0,
    'viewOptionsVersion': 1,
    'backgroundImageAlias': b'\x00\x00\x00\x00\x02\x1e\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd1\x94\\\xb0H+\x00\x05\x00\x00\x00\x98\x0fbackground.tiff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x99\xd19\xb0\xf8\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\r\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0b.background\x00\x00\x10\x00\x08\x00\x00\xd1\x94\\\xb0\x00\x00\x00\x11\x00\x08\x00\x00\xd19\xb0\xf8\x00\x00\x00\x01\x00\x04\x00\x00\x00\x98\x00\x0e\x00 \x00\x0f\x00b\x00a\x00c\x00k\x00g\x00r\x00o\x00u\x00n\x00d\x00.\x00t\x00i\x00f\x00f\x00\x0f\x00\x02\x00\x00\x00\x12\x00\x1c/.background/background.tiff\x00\x14\x01\x06\x00\x00\x00\x00\x01\x06\x00\x02\x00\x00\x0cMacintosh HD\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xce\x97\xab\xc3H+\x00\x00\x01\x88[\x88\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02u\xab\x8d\xd1\x94\\\xb0devrddsk\xff\xff\xff\xff\x00\x00\t \x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x07gulden\x00\x00\x10\x00\x08\x00\x00\xce\x97\xab\xc3\x00\x00\x00\x11\x00\x08\x00\x00\xd1\x94\\\xb0\x00\x00\x00\x01\x00\x14\x01\x88[\x88\x00\x16\xa9\t\x00\x08\xfaR\x00\x08\xfaQ\x00\x02d\x8e\x00\x0e\x00\x02\x00\x00\x00\x0f\x00\x1a\x00\x0c\x00M\x00a\x00c\x00i\x00n\x00t\x00o\x00s\x00h\x00 \x00H\x00D\x00\x13\x00\x01/\x00\x00\x15\x00\x02\x00\x14\xff\xff\x00\x00\xff\xff\x00\x00',
    'backgroundColorBlue': 1.0,
    'iconSize': 96.0,
    'backgroundColorGreen': 1.0,
    'arrangeBy': 'none',
    'showIconPreview': True,
    'gridSpacing': 100.0,
    'gridOffsetY': 0.0,
    'showItemInfo': False,
    'labelOnBottom': True,
    'backgroundType': 2,
    'backgroundColorRed': 1.0
}
alias = Alias.from_bytes(icvp['backgroundImageAlias'])
alias.volume.name = package_name_ns
alias.volume.posix_path = '/Volumes/' + package_name_ns
alias.volume.disk_image_alias.target.filename = package_name_ns + '.temp.dmg'
alias.volume.disk_image_alias.target.carbon_path = 'Macintosh HD:Users:\x00guldenuser:\x00Documents:\x00gulden:\x00gulden:\x00' + package_name_ns + '.temp.dmg'
alias.volume.disk_image_alias.target.posix_path = 'Users/guldenuser/Documents/gulden/gulden/' + package_name_ns + '.temp.dmg'
alias.target.carbon_path = package_name_ns + ':.background:\x00background.tiff'
icvp['backgroundImageAlias'] = biplist.Data(alias.to_bytes())
ds['.']['icvp'] = icvp

ds['.']['vSrn'] = ('long', 1)

ds['Applications']['Iloc'] = (370, 156)
ds['Gulden.app']['Iloc'] = (128, 156)

ds.flush()
ds.close()
