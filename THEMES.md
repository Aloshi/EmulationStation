Themes
======

EmulationStation allows each system to have its own "theme." A theme is a collection **views** that define some **elements**, each with their own **properties**.


Simple Example
==============

Here is a very simple theme that changes the description text's color:

```
<theme>
	<version>3</version>
	<view name="detailed">
		<text name="description">
			<color>00FF00</color>
		</text>
		<image name="my_image" extra="true">
			<pos>0.5 0.5</pos>
			<origin>0.5 0.5</origin>
			<size>0.8 0.8</size>
			<path>./my_art/my_awesome_image.jpg</path>
		</image>
	</view>
</theme>
```

How it works
============

Everything must be inside a `<theme>` tag.

**The `<version>` tag *must* be specified**.  This is the version of the theming system the theme was designed for.  The current version is 3.



A *view* can be thought of as a particular "screen" within EmulationStation.  Views are defined like this:

```
<view name="ViewNameHere">
	... define elements here ...
</view>
```



An *element* is a particular visual element, such as an image or a piece of text.  You can either modify an element that already exists for a particular view (as is done in the "description" example), like this:

```
	<elementTypeHere name="ExistingElementNameHere">
		... define properties here ...
	</elementTypeHere>
```

Or, you can create your own elements by adding `extra="true"` (as is done in the "my_image" example) like this:

```
	<elementTypeHere name="YourUniqueElementNameHere" extra="true">
		... define properties here ...
	</elementTypeHere>
```

"Extra" elements will be drawn in the order they are defined.  When they get drawn relative to the pre-existing elements depends on the view.  Make sure "extra" element names do not clash with existing names.



*Properties* control how a particular *element* looks - for example, its position, size, image path, etc.  There different types of properties that determine what kinds of values you can use - you can read about them below in the "Reference" section.  Properties are defined like this:

```
		<propertyNameHere>ValueHere</propertyNameHere>
```




Advanced Features
=================

### The `<include>` tag

You can include theme files within theme files, similar to `#include` in C (though the mechanism is different, the effect is the same).  Example:

`~/.emulationstation/all_themes.xml`:
```
<theme>
	<version>3</version>
	<view name="detailed">
		<text name="description">
			<fontPath>./all_themes/myfont.ttf</fontPath>
			<color>00FF00</color>
		</text>
	</view>
</theme>
```

`~/.emulationstation/snes/theme.xml`:
```
<theme>
	<version>3</version>
	<include>./../all_themes.xml</include>
	<view name="detailed">
		<text name="description">
			<color>FF0000</color>
		</text>
	</view>
</theme>
```

Is equivalent to this `snes/theme.xml`:
```
<theme>
	<version>3</version>
	<view name="detailed">
		<text name="description">
			<fontPath>./all_themes/myfont.ttf</fontPath>
			<color>FF0000</color>
		</text>
	</view>
</theme>
```

Notice that properties that were not specified got merged (`<fontPath>`) and the `snes/theme.xml` could overwrite the included files' values (`<color>`).  Also notice the included file still needed the `<version>` tag.



### The "common" view

Sometimes you want to apply the same values to the same element across many views.  The "common" view is one way to do this.

```
<theme>
	<version>3</version>
	<view name="common">
		<image name="header">
			<path>./snes_art/snes_header.png</path>
		</image>
	</view>
	<view name="detailed">
		<image name="header">
			<path>./snes_art/snes_header_detailed.png</path>
		</image>
	</view>
</theme>
```

Is equivalent to:
```
<theme>
	<version>3</version>
	<view name="basic">
		<image name="header">
			<path>./snes_art/snes_header.png</path>
		</image>
	</view>
	<view name="detailed">
		<image name="header">
			<path>./snes_art/snes_header_detailed.png</path>
		</image>
	</view>
	<view name="grid">
		<image name="header">
			<path>./snes_art/snes_header.png</path>
		</image>
	</view>
	<view name="system">
		<image name="header">
			<path>./snes_art/snes_header.png</path>
		</image>
	</view>
	... and any other view that might try to look up "header" ...
</theme>
```

Notice that you can still override the "common" view in a specific view definition (as shown in the "detailed" view).

You probably should not use the "common" view for element positioning.  You also should not use it to create "extra" elements.


Reference
=========

## Views, their elements, and accepted properties:

#### basic
	* image name="background" - PATH | TILING
	* image name="header" - POSITION | SIZE | PATH
	* textlist name="gamelist" - ALL

#### detailed
	* image name="background" - PATH | TILING
	* image name="header" - POSITION | SIZE | PATH
	* textlist name="gamelist" - ALL
	* image name="gameimage" - POSITION | SIZE
	* container name="infoPanel" - POSITION | SIZE
	* text name="description" - POSITION | FONT_PATH | FONT_SIZE | COLOR

#### grid
	* image name="background" - PATH | TILING
	* image name="header" - POSITION | SIZE | PATH

#### system
	* image name="header" - ALL
	* image name="system" - ALL

#### menu
	* ninepatch name="background" - PATH
	* textlist name="menulist" - FONT_PATH | COLOR | SOUND
	* sound name="menuOpen" - PATH
	* sound name="menuClose" - PATH

#### fastSelect
	* ninepatch name="background" - PATH
	* text name="letter" - FONT_PATH | COLOR
	* text name="subtext" - FONT_PATH | COLOR


## Types of elements and their properties:

#### image
	* `pos` - type: NORMALIZED_PAIR.
	* `size` - type: NORMALIZED_PAIR.
	* `origin` - type: NORMALIZED_PAIR.
	* `path` - type: PATH.
	* `tile` - type: BOOLEAN.

#### text
	* `pos` - type: NORMALIZED_PAIR.
	* `size` - type: NORMALIZED_PAIR.
	* `text` - type: STRING.
	* `color` - type: COLOR.
	* `fontPath` - type: PATH.
	* `fontSize` - type: FLOAT.
	* `center` - type: BOOLEAN.

#### textlist
	* `pos` - type: NORMALIZED_PAIR.
	* `size` - type: NORMALIZED_PAIR.
	* `selectorColor` - type: COLOR.
	* `selectedColor` - type: COLOR.
	* `primaryColor` - type: COLOR.
	* `secondaryColor` - type: COLOR.
	* `fontPath` - type: PATH.
	* `fontSize` - type: FLOAT.
	* `scrollSound` - type: PATH.

#### container
	* `pos` - type: NORMALIZED_PAIR.
	* `size` - type: NORMALIZED_PAIR.

#### ninepatch
	* `pos` - type: NORMALIZED_PAIR.
	* `size` - type: NORMALIZED_PAIR.
	* `path` - type: PATH.

A quick word on the "ninepatch" type - EmulationStation borrows the concept of "nine patches" from Android (or "9-Slices"). Currently the implementation is very simple and hard-coded to only use 48x48px images (16x16px for each "patch"). Check the `data/resources` directory for some examples (button.png, frame.png).

#### sound
	* `path` - type: PATH.


*Note that a view may choose to only make only certain properties on a particular element themable.*


## Types of properties:

* NORMALIZED_PAIR - two decimals, in the range [0..1].  For example, `0.25 0.5`.
* PATH - a path.  If the first character is a `~`, it will be expanded into the environment variable for the home path (`$HOME` or `%HOMEPATH%`, depending on platform).  If the first character is a `.`, it will be expanded to the theme file's directory.
* BOOLEAN - `true`/`1` or `false`/`0`.
* COLOR - a hexidecimal RGB or RGBA color (6 or 8 digits).  If 6 digits, will assume the alpha channel is `FF` (not transparent).
* FLOAT - a decimal.
* STRING - a string of text.



-Aloshi
http://www.aloshi.com
