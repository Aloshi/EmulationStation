#!/usr/bin/env python3
"""
Utility to parse a MAME compatible DAT file and produce the EmulationStation's resource files:
* mamebioses.xml - list of BIOS romsets
* mamedevices.xml - list of MAME device-type romsets
* mamenames.xml - list of MAME compatible romsets, with their description

Any parameter given is considered a DAT file and parsed.
The order of the DAT file is significant for the 'mamenames.xml' output, since the first description of a romset takes precendence.

Format notes:
 - The files must be in XML format, older DAT format files are not supported
 - (upstream) MAME uses 'machine' as main element for a romset, with 'isbios' and 'isdevice' attributes used to mark it as a BIOS/device
 - MAME2003 and FBNeo don't have the concept of 'devices', they use 'isbios' or 'runnable=no' attributes to mark a BIOS romset

"""
from xml.sax.saxutils import escape
from datetime import datetime
import xml.etree.ElementTree as et
import sys
import os

games = {}
bioses = []
devices = []

files = []

if len(sys.argv) < 2:
    print(f"Dat files missing, please add some.\nUsage: {sys.argv[0]} <DatFile1> .. <DatFileN>", file=sys.stderr)
    sys.exit(1)

for dat in sys.argv[1:]:
    print(f"Reading file '{dat}'")
    if not os.path.isfile(dat):
        print(f"File {dat} not found, skipping", file=sys.stderr)
        continue

    try:
        xml = et.parse(dat)
        files.append(os.path.basename(dat))
    except Exception:
        print(f"File {dat} cannot be parsed as XML!", file=sys.stderr)
        continue

    """
    FBNeo and older MAME dat files use 'game' as main element
    """
    for game in xml.findall('.//game'):
        if ('runnable' in game.attrib and game.attrib['runnable'] == "no") or \
                ('isbios' in game.attrib and game.attrib['isbios'] == "yes"):
            bioses.append(game.attrib['name'])
            continue

        name = game.attrib['name']
        desc = escape(game.find('description').text)
        if name not in games:
            games[name] = desc

    """
    Mame DAT file use 'machine' as main element
    """
    for game in xml.findall('.//machine'):
        if ('isbios' in game.attrib and game.attrib['isbios'] == "yes"):
            bioses.append(game.attrib['name'])
            continue

        if ('isdevice' in game.attrib and game.attrib['isdevice'] == "yes"):
            # Don't add the device unless it has at least one ROM file
            if len(game.findall('./rom')) > 0:
                devices.append(game.attrib['name'])
            continue

        name = game.attrib['name']
        desc = escape(game.find('description').text)
        if name not in games:
            games[name] = desc

print(f"Found {len(games)} games, {len(sorted(set(bioses)))} BIOSes and {len(sorted(set(devices)))} devices")
ident_info = f"<!-- Generated on {datetime.utcnow().strftime('%F')}, from {', '.join(files)} -->"

if len(games) > 0:
    with open('mamenames.xml', 'w') as f:
        print(ident_info,file=f)
        for game in sorted(games):
            print(f"<game>\n\t<mamename>{game}</mamename>\n\t<realname>{games[game]}</realname>\n</game>", file=f)
else:
    print("No games found, skipped writing 'mamenames.xml'", file=sys.stderr)

if len(bioses) > 0:
    with open('mamebioses.xml', 'w') as f:
        print(ident_info,file=f)
        for dev in sorted(set(bioses)):
            print(f"<bios>{dev}</bios>", file=f)
else:
    print("No BIOSes found, skipped writing 'mamebioses.xml'", file=sys.stderr)

if len(devices) > 0:
    with open('mamedevices.xml', 'w') as f:
        print(ident_info,file=f)
        for bios in sorted(set(devices)):
            print(f"<device>{bios}</device>", file=f)
else:
    print("No devices found, skipped writing 'mamedevices.xml'", file=sys.stderr)
