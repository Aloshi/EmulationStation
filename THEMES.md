Themes
======

EmulationStation allows each system to have its own "theme." A theme is a collection of display settings and images defined in an XML document.

ES will check two places for a theme: first, the system's search directory for theme.xml. Then, if that's not found, $HOME/.emulationstation/es_theme.xml.

Almost all positions, dimensions, etc. work in percentages - that is, they are a decimal between 0 and 1, representing the percentage of the screen on that axis to use.
This ensures that themes look similar at every resolution.


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
A theme is made up of components, which have various types. At the moment, the only type is `image`. Components can be nested for your own organization. Components are rendered in the order they are defined - that means you'll want to define the background first, a header image second, etc.


The "image" component
=====================
Used to display an image.

`<path>` - path to the image file. Most common file types are supported, and . and ~ are properly expanded.

`<pos>` - the position, as two screen percentages, at which to display the image.

`<dim>` - the dimensions, as two screen percentages, that the image will be resized to. Make one axis 0 to keep the aspect ratio.

`<origin>` - the point on the image that `<pos>` defines, as an image percentage. "0.5 0.5", the center of the image, by default.

`<tiled />` - if present, the image is tiled instead of resized. Tiling isn't exact at the moment, but good enough for backgrounds.

`<useAlpha />` - if present, the image will not be stripped of its alpha channel. It will render much slower, but should have transparency.


Display tags
============
Display tags must be at the root of the <theme> tree - for example, they can't be inside a component tag. They are not required.

`<listPrimaryColor>` - the hex font color to use for games on the GuiGameList.

`<listSecondaryColor>` - the hex font color to use for folders on the GuiGameList.

`<descColor>` - the hex font color to use for the description on the GuiGameList.

`<listSelectorColor>` - the hex color to use for the "selector bar" on the GuiGameList.

`<listLeftAlign />` - if present, the games list names will be left aligned to $infoWidth.

`<hideHeader />` - if present, the system name header won't be displayed (useful for replacing it with an image).

`<hideDividers />` - if present, the divider between games on the detailed GuiGameList won't be displayed.


List of variables
=================

Variables can be used in position and dimension definitions. They can be added, subtracted, multiplied, and divided. Parenthesis are valid. They are a percentage of the screen.

`$headerHeight` - height of the system name header.

`$infoWidth` - where the center of the horizontal divider is drawn.


Bugs
====

Hexidecimal colors might be read in backwards. Woops. I'll get around to fixing this eventually.


-Aloshi
http://www.aloshi.com
