Themes
======

EmulationStation allows each system to have its own "theme." A theme is a collection of display settings and images defined in an XML document.

ES will check 3 places for a theme, in the following order:
 - a theme.xml file in the root of a system's %PATH% directory.
 - $HOME/.emulationstation/%NAME%/theme.xml
 - $HOME/.emulationstation/es_theme.xml

Almost all positions, dimensions, origins, etc. work in percentages - that is, they are a decimal between 0 and 1, representing the percentage of the screen on that axis to use. This ensures that themes look similar at every resolution.

Colors are hex values, either 6 or 8 characters, defined as RRGGBB or RRGGBBAA. If alpha is not included, a default value of FF will be assumed (not transparent).


Example
=======

Here's a theme that defines some colors, displays a background, and displays a logo in the top left corner:
```
<theme>
	<listPrimaryColor>0000FF</listPrimaryColor>
	<listSecondaryColor>00FF00</listSecondaryColor>

	<component>
		<type>image</type>
		<path>./theme/background.png</path>
		<pos>0 0</pos>
		<dim>1 1</dim>
		<origin>0 0</origin>
	</component>

	<component>
		<type>image</type>
		<path>./theme/logo.png</path>
		<pos>0 0</pos>
		<dim>0.4 0</dim>
		<origin>0 0</origin>
	</component>
</theme>

<!-- You can also optionally define a "basic" theme, which is used instead if ES is in the "basic" view (no box art) -->
<basicTheme>
	<listPrimaryColor>0000FF</listPrimaryColor>
	<listSecondaryColor>00FF00</listSecondaryColor>

	<component>
		<type>image</type>
		<path>./theme/background.png</path>
		<pos>0 0</pos>
		<dim>1 1</dim>
		<origin>0 0</origin>
	</component>

</basicTheme>
```

Themes can be defined with two tags: `<theme>` and `<themeBasic>`. 
You can define both a normal and basic theme in the same file.

If EmulationStation is running in "basic" mode, it will try to use `<themeBasic>`. If that doesn't exist or ES is running in "detailed" mode (a gamelist.xml is present), `<theme>` will be used.


Components
==========

A theme is made up of components, which have various types. At the moment, only types `image` and `text` are supported. Components are rendered in the order they are defined - that means you'll want to define the background first, a header image second, etc.

Components may use the following variables to setup system dependend paths or texts:
`%SYSTEM_NAME%` - replaced with the name of the current system (e.g. "nes")
`%SYSTEM_FULLNAME%` - replaced with the full name of the current system (e.g. "Nintento Entertainment System")
`%SYSTEM_GAMECOUNT%` - number of games available.

The "image" component
=====================

Used to display an image.

`<path>` - path to the image file. Most common file types are supported, and . and ~ and %VAR% are properly expanded.

`<pos>` - the position, as two screen percentages, at which to display the image.

`<dim>` - the dimensions, as two screen percentages, that the image will be resized to. Make one axis 0 to keep the aspect ratio.

`<origin>` - the point on the image that `<pos>` defines, as an image percentage. "0.5 0.5", the center of the image, by default.

`<tiled />` - if present, the image is tiled instead of resized.

`<upscale/>` - if present the image will be resized larger than the original image if necessary.

The "text" component
====================

`<content>` - The actual text to display (may contain variables).

`<pos>` - the position, as two screen percentages, at which to display the image.

`<dim>` - the dimensions, as two screen percentages, that the image will be resized to. Make one axis 0 to keep the aspect ratio.

`<font>` - specifies the font to be used for the text (see definition of fontTag below).

`<color>` - specifies the color for the text. Default is listPrimaryColor.

`<center>` - if present the text is centered.

Display tags
============

Display tags define some "meta" display attributes about your theme. Display tags must be at the root of the `<theme>` tree - for example, they can't be inside a component tag. They are not required.


**Game list attributes:**

`<listPrimaryColor>` - the hex font color to use for games on the GuiGameList.

`<listSecondaryColor>` - the hex font color to use for folders on the GuiGameList.

`<descColor>` - the hex font color to use for the description on the GuiGameList.

`<listSelectorColor>` - the hex color to use for the "selector bar" on the GuiGameList. Default is `000000FF`.

`<listSelectedColor>` - the hex color to use for selected text on the GuiGameList. Default is zero, which means no change.

`<listLeftAlign />` - if present, the games list names will be left aligned to the value of `<listOffsetX>` + `<listTextOffsetX>`. On by default for detailed themes.

`<hideHeader />` - if present, the system name header won't be displayed (useful for replacing it with an image). If you're making a complete custom theme, you probably want to use this.

`<hideDividers />` - if present, the divider between games on the detailed GuiGameList won't be displayed.

`<listOffsetX>` - the percentage to offset the list by. Default is 0.5 (half the screen). **Will also move the selector bar**.

`<listTextOffsetX>` - the percentage to offset the text in the list by. Default is 0.005. Only works in combination with `<listLeftAlign />`.



**Game image attributes:**

`<gameImagePos>` - two values for the position of the game art, in the form of `[x] [y]`, as a percentage. Default is `$infoWidth/2 $headerHeight`. 

`<gameImageSpace>` - Defines the border space between two images. Default is 0.01.

`<gameImageDim>` - two values for the dimensions of the game art, in the form of `[width] [height]`, as a percentage of the screen. Default is `$infoWidth 0` (width fits within the info column). All images will be placed inside this box (below each other). Images will be centered horizontally inside the box and resized if they are too large for the specified with. If gameImageUpscale has been defined than smaller images will be upscale to the with defined for gameImageDim. If not all images fit inside the area it will start scrolling.

`<gameImageUpscale>` - if present, game images smaller than the defined gameImageDim will be upscaled.

`<gameImageOrigin>` - two values for the origin of the game art, in the form of `[x] [y]`, as a percentage. Default is `0.5 0` (top-center of the image).

`<gameImageNotFound>` - path to the image to display if a game's image is missing. '.' and '~' are expanded.



**Fast Select box attributes:**

`<fastSelectColor>` - the hex color to use for the letter display on the fast select box.

`<fastSelectFrame>` - the path to a "nine patch" image to use for the "background" of the fast select box. See the "Nine Patches" section for more info.

`<fastSelectFont>`  - font definition to use for the fast select letter. See the "Fonts" section for more info.


Fonts
=====

Fonts are defined like so:

```
<fontTag>
	<path>./path/to/font</path>
	<size>0.05</size>
</fontTag>
```

You can leave off any tags you don't want to use, and they'll use the default. Size is defined as a percentage of the screen height. "." and "~" are expanded for paths.

NOTE: If your font size is too big, it'll overrun the maximum OpenGL texture size. ES will attempt to rasterize it in progressively smaller sizes until one fits, then upscale it.

**Font tags:**

`<listFont>` - font to use for the game list.

`<descriptionFont>` - font to use for description text.

`<fastSelectFont>` - font to use for the fast select letter.


Audio
=====

Themes can also define menu sounds. These tags go in the root of the `<theme>` tree, just like Display tags. Sounds should be in the .wav format. The relative path operator (.) and home operator (~) are properly expanded.

`<menuScrollSound>` - path to the sound to play when the game list or fast select menu is scrolling.

`<menuSelectSound>` - path to the sound to play when the user selects something from the game list.

`<menuBackSound>` - path to the sound to play when the user "goes up" from a folder in the game list.

`<menuOpenSound>` - path to the sound to play when the user opens a menu (either the "main menu" or the fast select menu).


Nine Patches
============

EmulationStation borrows the concept of "nine patches" from Android (or "9-Slices"). Currently the implementation is very simple and hard-coded to only use 48x48px images (16x16px for each "patch"). Check the `data/resources` directory for some examples (button.png, frame.png).


List of variables
=================

Variables can be used in position and dimension definitions. They can be added, subtracted, multiplied, and divided. Parenthesis are valid. They are a percentage of the screen.

For example, if you wanted to place an image that covered the left half of the screen, up to the game list, you could use `<dim>$infoWidth 1</dim>`.

`$headerHeight` - height of the system name header.

`$infoWidth` - where the left of the game list begins. Will follow `<listOffsetX>`.


-Aloshi
http://www.aloshi.com
