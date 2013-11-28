Themes
======

EmulationStation allows each system to have its own "theme." A theme is a collection of resources defined in an XML document.

Themes are loaded like this:

1. Initialize to default values.
2. If `$HOME/.emulationstation/es_theme_default.xml` exists, load it.
3a. If there is a `theme.xml` present in the root of a system's `path` directory, load it.
3b.   IF NOT, If `$HOME/.emulationstation/%SYSTEMNAME%/theme.xml` exists, load it.

Example
=======

```
<theme>
	<listPrimaryColor>0000FF</listPrimaryColor>
	<listSecondaryColor>00FF00</listSecondaryColor>

	<listFont>
		<path>./../all_themes/font.ttf</path>
		<size>0.045</size>
	</listFont>

	<descriptionFont>
		<path>./../all_themes/font.ttf</path>
		<size>0.035</size>
	</descriptionFont>

	<backgroundImage>
		<path>./theme/background.png</path>
		<tile>true</tile>
	</backgroundImage>

	<headerImage>
		<path>./theme/logo.png</path>
		<tile>false</tile>
	</headerImage>

	<scrollSound>./../all_themes/scrollSound.wav</scrollSound>
</theme>
```

Themes must be enclosed in a `<theme>` tag.

All paths automatically expand `./` to the folder containing the theme.xml.
All paths automatically expand `~/` to the home directory ($HOME on Linux, %HOMEPATH% on Windows).

Stuff you can define
====================

Fonts
=====

Fonts are defined like so:
```
<resourceName>
	<!-- Path is optional. -->
	<path>./some/path/here.ttf</path>
	<!-- Size is a percentage of screen height. Optional. -->
	<size>0.035</size>
</resourceName>
```

`<listFont>` - Default size: 0.045.
`<descriptionFont>` - Default size: 0.035.
`<fastSelectLetterFont>` - Default size: 0.15.

Colors
======

Colors are defined in hex, like this:

`<resourceName>00FF00FF</resourceName>`
or
`<resourceName>00FF00</resourceName>`
(without the alpha channel specified - will assume FF)

`<listPrimaryColor>` - Default: 0000FFFF.
`<listSecondaryColor>` - Default: 00FF00FF.
`<listSelectorColor>` - Default: 000000FF.
`<listSelectedColor>` - Default: 00000000.
`<descriptionColor>` - Default: 48474DFF.
`<fastSelectLetterColor>` - Default: FFFFFFFF.
`<fastSelectTextColor>` - Default: DDDDDDFF.

Images
======

Images are defined like this:
```
<resourceName>
	<path>./some/path/here.png</path>
	<!-- Can be true or false. -->
	<tile>true</tile>
</resourceName>
```
Pretty much any image format is supported.

`<backgroundImage>` - No default.
`<headerImage>` - No default.
`<infoBackgroundImage>` - No default.
`<verticalDividerImage>` - No default.
`<fastSelectBackgroundImage>` - Nine patch.  Default is the "button.png" resource.

Sounds
======

Sounds are defined like this:
`<resourceName>./some/path/here.wav</resourceName>`
Only .wav files are supported.

`<scrollSound>` - No default. Played when a list scrolls.
`<gameSelectSound>` - No default. Played when a game is launched.
`<backSound>` - No default. Played when leaving a folder in the game list.
`<menuOpenSound>` - No default. Played when the menu is opened.
`<menuCloseSound>` - No default. Played when the menu is closed.


Nine Patches
============

EmulationStation borrows the concept of "nine patches" from Android (or "9-Slices"). Currently the implementation is very simple and hard-coded to only use 48x48px images (16x16px for each "patch"). Check the `data/resources` directory for some examples (button.png, frame.png).

-Aloshi
http://www.aloshi.com
