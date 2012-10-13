Themes
======

EmulationStation allows each system to have its own "theme." A theme is a collection of display settings and images defined in an XML document.

ES will check two places for a theme: first, the system's search directory for theme.xml. Then, if that's not found, $HOME/.emulationstation/es_theme.xml.

Almost all positions, dimensions, origins, etc. work in percentages - that is, they are a decimal between 0 and 1, representing the percentage of the screen on that axis to use. This ensures that themes look similar at every resolution.


Example
=======

Here's a simple theme that defines some colors and displays a background:
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
</theme>
```

All themes must be enclosed in a `<theme>` tag.


Components
==========
A theme is made up of components, which have various types. At the moment, the only type is `image`. Components are rendered in the order they are defined - that means you'll want to define the background first, a header image second, etc.


The "image" component
=====================
Used to display an image.

`<path>` - path to the image file. Most common file types are supported, and . and ~ are properly expanded.

`<pos>` - the position, as two screen percentages, at which to display the image.

`<dim>` - the dimensions, as two screen percentages, that the image will be resized to. Make one axis 0 to keep the aspect ratio.

`<origin>` - the point on the image that `<pos>` defines, as an image percentage. "0.5 0.5", the center of the image, by default.

`<tiled />` - if present, the image is tiled instead of resized.


Display tags
============
Display tags must be at the root of the <theme> tree - for example, they can't be inside a component tag. They are not required.

`<listPrimaryColor>` - the hex font color to use for games on the GuiGameList.

`<listSecondaryColor>` - the hex font color to use for folders on the GuiGameList.

`<descColor>` - the hex font color to use for the description on the GuiGameList.

`<listSelectorColor>` - the hex color to use for the "selector bar" on the GuiGameList. Default is `000000`.

`<listSelectedColor>` - the hex color to use for selected text on the GuiGameList. Default is -1, which will not change the color.

`<listLeftAlign />` - if present, the games list names will be left aligned to the value of `<listOffsetX>` (default 0.5).

`<hideHeader />` - if present, the system name header won't be displayed (useful for replacing it with an image). If you're making a complete custom theme, you probably want to use this.

`<hideDividers />` - if present, the divider between games on the detailed GuiGameList won't be displayed.

`<listOffsetX>` - the percentage to offset the list by. Default is 0.5 (half the screen). **Will also move the selector bar**.

`<listTextOffsetX>` - the percentage to offset the text in the list by. Default is 0.005. Only works in combination with `<listLeftAlign />`.

~~`<gameImageOffsetY>` - the percentage to offset the displayed game image by. Default is the height of the header font.~~

`<gameImagePos>` - two values for the position of the game art, in the form of `[x] [y]`, as a percentage. Default is `$infoWidth/2 $headerHeight`. 

`<gameImageDim>' - two values for the dimensions of the game art, in the form of `[width] [height]`, as a percentage of the screen. Default is `0 0` (not resized). The image will only be resized if at least one axis is nonzero *and* exceeded by the image's size. You should always leave at least one axis as zero to preserve the aspect ratio.

`<gameImageOrigin>` two values for the origin of the game art, in the form of `[x] [y]`, as a percentage. Default is `0.5 0`.



**The Fast Select box can be themed with these tags:**

`<fastSelectColor>` - the hex color to use for the letter display on the Fast Select box.

`<boxBackground>` - path to a background image file. ~ and . are expanded.

`<boxBackgroundTiled />` - if present, the background will be tiled instead of stretched.

`<boxHorizontal>` - path to the "left" border image file. It will be flipped for the right border. ~ and . are expanded.

`<boxHorizontalTiled />` - if present, the horizontal image will be tiled instead of stretched downwards.

`<boxVertical>` - path to the "top" border image file. It will be flipped for the bottom border. ~ and . are expanded.

`<boxVerticalTiled />` - if present, the vertical image will be tiled instead of stretched to the right.

`<boxCorner>` - path to the "top left corner" image file. It will be flipped for the top right, bottom right, and bottom left corners. ~ and . are expanded.


Audio
=====

Themes can also define menu sounds. Sounds should be in the .wav format.

`<menuScrollSound>` - path to the sound to play when the game list or fast select menu is scrolling.

`<menuSelectSound>` - path to the sound to play when the user selects something from the game list.

`<menuBackSound>` - path to the sound to play when the user "goes up" from a folder in the game list.

`<menuOpenSound>` - path to the sound to play when the user opens a menu (either the "main menu" or the fast select menu).


List of variables
=================

Variables can be used in position and dimension definitions. They can be added, subtracted, multiplied, and divided. Parenthesis are valid. They are a percentage of the screen.

For example, if you wanted to place an image that covered the left half of the screen, up to the game list, you could use `<dim>$infoWidth 1</dim>`.

`$headerHeight` - height of the system name header.

`$infoWidth` - where the left of the game list begins. Will follow `<listOffsetX>`.


-Aloshi
http://www.aloshi.com
