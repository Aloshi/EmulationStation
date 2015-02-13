# System Options

Each system specified in es_systems.cfg can have additional settings which are settable per-game.

Here is an example providing additional options for each ZX Spectrum game, which changes which ZX Spectrum model the emulator (in this case FUSE) emulates:

```xml
    <system>

        <name>zxspectrum</name>
        <fullname>ZX Spectrum</fullname>
        <path>~/roms/zxspectrum</path>
        <extension>.tzx</extension>
        <command>fuse %OPTIONS% %ROM%</command>
        <platform>zxspectrum</platform>
        <theme>zxspectrum</theme>

        <option>
            <id>machine</id>
            <replace>%OPTIONS%</replace>
            <desc>Spectrum model</desc>
            <default>48</default>
            <value>
                <id>16</id>
                <desc>ZX Spectrum 16K</desc>
                <code>--machine 16</code>
            </value>
            <value>
                <id>48</id>
                <desc>ZX Spectrum 48K</desc>
                <code>--machine 48</code>
            </value>
            <value>
                <id>48_ntsc</id>
                <desc>ZX Spectrum 48K (NTSC)</desc>
                <code>--machine 48_ntsc</code>
            </value>
            <value>
                <id>128</id>
                <desc>ZX Spectrum 128K</desc>
                <code>--machine 128</code>
            </value>
            <value>
                <id>plus2</id>
                <desc>ZX Spectrum +2</desc>
                <code>--machine plus2</code>
            </value>
            <value>
                <id>plus2a</id>
                <desc>ZX Spectrum +2A</desc>
                <code>--machine plus2a</code>
            </value>
            <value>
                <id>plus3</id>
                <desc>ZX Spectrum +3</desc>
                <code>--machine plus3</code>
            </value>
            <value>
                <id>plus3e</id>
                <desc>ZX Spectrum +3E</desc>
                <code>--machine plus3</code>
            </value>
            <value>
                <id>se</id>
                <desc>ZX Spectrum SE</desc>
                <code>--machine se</code>
            </value>
            <value>
                <id>pentagon</id>
                <desc>Pentagon</desc>
                <code>--machine pentagon</code>
            </value>
            <value>
                <id>pentagon512</id>
                <desc>Pentagon "512"</desc>
                <code>--machine pentagon512</code>
            </value>
            <value>
                <id>pentagon1024</id>
                <desc>Pentagon 1024</desc>
                <code>--machine pentagon1024</code>
            </value>
            <value>
                <id>scorpion</id>
                <desc>Scorpion ZS 256</desc>
                <code>--machine scorpion</code>
            </value>
            <value>
                <id>2048</id>
                <desc>TC2048</desc>
                <code>--machine 2048</code>
            </value>
            <value>
                <id>2068</id>
                <desc>TC2068</desc>
                <code>--machine 2068</code>
            </value>
            <value>
                <id>ts2068</id>
                <desc>TS2068</desc>
                <code>--machine ts2068</code>
            </value>
        </option>

    </system>
```

Reference
=========

Each <option> has the following:

* `id` - a unique ID identifying the option - this is used to save the data
* `replace` - the text from the command to replace
* `desc` - a user-friendly description for this option
* `default` - the ID of the value to be used by default
* `value` - one per value

Each <value> has the following:

* `id` - a unique ID identifying the value - this is used to save the data
* `desc` - a user-friendly description for this value
* `code` - the text which will be put in the command line in the place of the option's `replace`

