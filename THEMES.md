Themes
======

EmulationStation allows each system to have its own "theme." A theme is a collection **views** that define some **elements**, each with their own **properties**.


Simple Example
==============

Here is a very simple theme that changes the description text's color:

```xml
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

```xml
<view name="ViewNameHere">
	... define elements here ...
</view>
```



An *element* is a particular visual element, such as an image or a piece of text.  You can either modify an element that already exists for a particular view (as is done in the "description" example), like this:

```xml
	<elementTypeHere name="ExistingElementNameHere">
		... define properties here ...
	</elementTypeHere>
```

Or, you can create your own elements by adding `extra="true"` (as is done in the "my_image" example) like this:

```xml
	<elementTypeHere name="YourUniqueElementNameHere" extra="true">
		... define properties here ...
	</elementTypeHere>
```

"Extra" elements will be drawn in the order they are defined.  When they get drawn relative to the pre-existing elements depends on the view.  Make sure "extra" element names do not clash with existing names.



*Properties* control how a particular *element* looks - for example, its position, size, image path, etc.  There different types of properties that determine what kinds of values you can use - you can read about them below in the "Reference" section.  Properties are defined like this:

```xml
		<propertyNameHere>ValueHere</propertyNameHere>
```




Advanced Features
=================

It is recommended that if you are writing a theme you launch EmulationStation with the `--debug` and `--windowed` switches.  This way you can read error messages without having to check the log file.  You can also reload the current gamelist view with `Ctrl-R` if `--debug` is specified.

### The `<include>` tag

You can include theme files within theme files, similar to `#include` in C (though the mechanism is different, the effect is the same).  Example:

`~/.emulationstation/all_themes.xml`:
```xml
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
```xml
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
```xml
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

```xml
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
```xml
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

## Views, their elements, and themable properties:

#### basic
* `image name="background"` - ALL
	- This is a background image that exists for convenience. It goes from (0, 0) to (1, 1).
* `text name="headerText"` - ALL
	- A header text, which displays the name of the system.  Displayed at the top center of the screen.  Centered by default.
* `image name="header"` - ALL
	- A header image.  If a non-empty `path` is specified, `text name="headerText"` will be hidden and this image will be, by default, displayed roughly in its place.
* `textlist name="gamelist"` - ALL
	- The gamelist.  `primaryColor` is for games, `secondaryColor` is for folders.  Centered by default.

---

#### detailed
* `image name="background"` - ALL
	- This is a background image that exists for convenience. It goes from (0, 0) to (1, 1).
* `text name="headerText"` - ALL
	- A header text, which displays the name of the system.  Displayed at the top center of the screen.  Centered by default.
* `image name="header"` - ALL
	- A header image.  If a non-empty `path` is specified, `text name="headerText"` will be hidden and this image will be, by default, displayed roughly in its place.
* `textlist name="gamelist"` - ALL
	- The gamelist.  `primaryColor` is for games, `secondaryColor` is for folders.  Left aligned by default.

* Metadata
	* Labels
		* `text name="md_lbl_rating"` - ALL
		* `text name="md_lbl_releasedate"` - ALL
		* `text name="md_lbl_developer"` - ALL
		* `text name="md_lbl_publisher"` - ALL
		* `text name="md_lbl_genre"` - ALL
		* `text name="md_lbl_players"` - ALL
		* `text name="md_lbl_lastplayed"` - ALL
		* `text name="md_lbl_playcount"` - ALL

	* Values
		* All values will follow to the right of their labels if a position isn't specified.

		* `image name="md_image"` - POSITION | SIZE
			- Path is the "image" metadata for the currently selected game.
		* `rating name="md_rating"` - ALL
			- The "rating" metadata.
		* `datetime name="md_releasedate"` - ALL
			- The "releasedate" metadata.
		* `text name="md_developer"` - ALL
			- The "developer" metadata.
		* `text name="md_publisher"` - ALL
			- The "publisher" metadata.
		* `text name="md_genre"` - ALL
			- The "genre" metadata.
		* `text name="md_players"` - ALL
			- The "players" metadata (number of players the game supports).
		* `datetime name="md_lastplayed"` - ALL
			- The "lastplayed" metadata.  Displayed as a string representing the time relative to "now" (e.g. "3 hours ago").
		* `text name="md_playcount"` - ALL
			- The "playcount" metadata (number of times the game has been played).
		* `text name="md_description"` - POSITION | SIZE | FONT_PATH | FONT_SIZE | COLOR
			- Text is the "desc" metadata.  If no `pos`/`size` is specified, will move and resize to fit under the lowest label and reach to the bottom of the screen.

---

#### grid
* `image name="background"` - ALL
	- This is a background image that exists for convenience. It goes from (0, 0) to (1, 1).
* `text name="headerText"` - ALL
	- A header text, which displays the name of the system.  Displayed at the top center of the screen.  Centered by default.
* `image name="header"` - ALL
	- A header image.  If a non-empty `path` is specified, `text name="headerText"` will be hidden and this image will be, by default, displayed roughly in its place.

---

#### system
* `text name="headerText"` - ALL
	- A header text, which displays the name of the system.  Displayed at the top center of the screen.  Centered by default.
* `image name="header"` - ALL
	- A header image.  If a non-empty `path` is specified, `text name="headerText"` will be hidden and this image will be, by default, displayed roughly in its place.
* image name="system" - ALL
	- A large image representing the system (usually a picture of the system itself).  

---

#### fastSelect
* `ninepatch name="windowBackground"` - PATH
	- Fit around the fast select UI as a background.
* `text name="letter"` - FONT_PATH | COLOR
	- The big letter that shows what letter you'll jump to when you let go of the fast select button.
* `text name="subtext"` - FONT_PATH | COLOR
	- The text that displays the current sort mode.

---

#### menu
* `ninepatch name="windowBackground"` - PATH
	- Background for the menu.  Fit from top-left corner at (0.175, 0.05) to bottom-right corner at (0.825, 0.95).
* `textlist name="menulist"` - FONT_PATH | COLOR | SOUND
	- The list of menu options. `primaryColor` is for most options, `secondaryColor` is for the "shutdown" option.
* `sound name="menuOpen"` - PATH
	- Played when the menu opens.
* `sound name="menuClose"` - PATH
	- Played when the menu closes.


## Types of properties:

* NORMALIZED_PAIR - two decimals, in the range [0..1], delimited by a space.  For example, `0.25 0.5`.  Most commonly used for position (x and y coordinates) and size (width and height).
* PATH - a path.  If the first character is a `~`, it will be expanded into the environment variable for the home path (`$HOME` or `%HOMEPATH%`, depending on platform).  If the first character is a `.`, it will be expanded to the theme file's directory.
* BOOLEAN - `true`/`1` or `false`/`0`.
* COLOR - a hexidecimal RGB or RGBA color (6 or 8 digits).  If 6 digits, will assume the alpha channel is `FF` (not transparent).
* FLOAT - a decimal.
* STRING - a string of text.


## Types of elements and their properties:

Common to almost all elements is a `pos` and `size` property of the NORMALIZED_PAIR type.  They are normalized in terms of their "parent" object's size; 99% of the time, this is just the size of the screen.  In this case, `<pos>0 0</pos>` would correspond to the top left corner, and `<pos>1 1</pos>` the bottom right corner (a positive Y value points further down).  `pos` almost always refers to the top left corner of your element.  You *can* use numbers outside of the [0..1] range if you want to place an element partially or completely off-screen.

The order you define properties in does not matter.
Remember, you do *not* need to specify every property!
*Note that a view may choose to only make only certain properties on a particular element themable!*

#### image

Can be created as an extra.

* `pos` - type: NORMALIZED_PAIR.
* `size` - type: NORMALIZED_PAIR.
	- If only one axis is specified (and the other is zero), the other will be automatically calculated in accordance with the image's aspect ratio.
* `maxSize` - type: NORMALIZED_PAIR.
	- The image will be resized as large as possible so that it fits within this size and maintains its aspect ratio.  Use this instead of `size` when you don't know what kind of image you're using so it doesn't get grossly oversized on one axis (e.g. with a game's image metadata).
* `origin` - type: NORMALIZED_PAIR.
	- Where on the image `pos` refers to.  For example, an origin of `0.5 0.5` and a `pos` of `0.5 0.5` would place the image exactly in the middle of the screen.  If the "POSITION" and "SIZE" attributes are themable, "ORIGIN" is implied.
* `path` - type: PATH.
	- Path to the image file.  Most common extensions are supported (including .jpg, .png, and unanimated .gif).
* `tile` - type: BOOLEAN.
	- If true, the image will be tiled instead of stretched to fit its size.  Useful for backgrounds.
* `color` - type: COLOR.
	- Multiply each pixel's color by this color. For example, an all-white image with `<color>FF0000</color>` would become completely red.

#### text

Can be created as an extra.

* `pos` - type: NORMALIZED_PAIR.
* `size` - type: NORMALIZED_PAIR.
	- Possible combinations:
	- `0 0` - automatically size so text fits on one line (expanding horizontally).
	- `w 0` - automatically wrap text so it doesn't go beyond `w` (expanding vertically).
	- `w h` - works like a "text box."  If `h` is non-zero and `h` <= `fontSize` (implying it should be a single line of text), text that goes beyond `w` will be truncated with an elipses (...).
* `text` - type: STRING.
* `color` - type: COLOR.
* `fontPath` - type: PATH.
	- Path to a truetype font (.ttf).
* `fontSize` - type: FLOAT.
	- Size of the font as a percentage of screen height (e.g. for a value of `0.1`, the text's height would be 10% of the screen height).
* `center` - type: BOOLEAN.
	- True to center, false to left-align.

#### textlist

* `pos` - type: NORMALIZED_PAIR.
* `size` - type: NORMALIZED_PAIR.
* `selectorColor` - type: COLOR.
	- Color of the "selector bar."
* `selectedColor` - type: COLOR.
	- Color of the highlighted entry text.
* `primaryColor` - type: COLOR.
	- Primary color; what this means depends on the text list.  For example, for game lists, it is the color of a game.
* `secondaryColor` - type: COLOR.
	- Secondary color; what this means depends on the text list.  For example, for game lists, it is the color of a folder.
* `fontPath` - type: PATH.
* `fontSize` - type: FLOAT.
* `scrollSound` - type: PATH.
	- Sound that is played when the list is scrolled.
* `alignment` - type: STRING.
	- Valid values are "left", "center", or "right".  Controls alignment on the X axis.
* `horizontalMargin` - type: FLOAT.
	- Horizontal offset for text from the alignment point.  If `alignment` is "left", offsets the text to the right.  If `alignment` is "right", offsets text to the left.  No effect if `alignment` is "center".  Given as a percentage of the element's parent's width (same unit as `size`'s X value).

#### ninepatch

* `pos` - type: NORMALIZED_PAIR.
* `size` - type: NORMALIZED_PAIR.
* `path` - type: PATH.

EmulationStation borrows the concept of "nine patches" from Android (or "9-Slices"). Currently the implementation is very simple and hard-coded to only use 48x48px images (16x16px for each "patch"). Check the `data/resources` directory for some examples (button.png, frame.png).

#### rating

* `pos` - type: NORMALIZED_PAIR.
* `size` - type: NORMALIZED_PAIR.
	- Only one value is actually used. The other value should be zero.  (e.g. specify width OR height, but not both.  This is done to maintain the aspect ratio.)
* `filledPath` - type: PATH.
	- Path to the "filled star" image.  Image must be square (width equals height).
* `unfilledPath` - type: PATH.
	- Path to the "unfilled star" image.  Image must be square (width equals height).

#### datetime

* `pos` - type: NORMALIZED_PAIR.
* `size` - type: NORMALIZED_PAIR.
	- You should probably not set this.  Leave it to `fontSize`.
* `color` - type: COLOR.
* `fontPath` - type: PATH.
* `fontSize` - type: FLOAT.

#### sound

* `path` - type: PATH.
	- Path to the sound file.  Only .wav files are currently supported.



[*Check out the "official" themes for some more examples!*](http://aloshi.com/emulationstation#themes)

-Aloshi
http://www.aloshi.com
