Themes
======

EmulationStation allows each system to have its own "theme." A theme is a collection **views** that define some **elements**, each with their own **properties**.

The first place ES will check for a theme is in the system's `<path>` folder, for a theme.xml file:
* `[SYSTEM_PATH]/theme.xml`

If that file doesn't exist, ES will try to find the theme in the current **theme set**.  Theme sets are just a collection of individual system themes arranged in the "themes" folder under some name.  Here's an example:

```
...
   themes/
      my_theme_set/
         snes/
            theme.xml
            my_cool_background.jpg

         nes/
            theme.xml
            my_other_super_cool_background.jpg

         common_resources/
            scroll_sound.wav

      another_theme_set/
         snes/
            theme.xml
            some_resource.svg
```

The theme set system makes it easy for users to try different themes and allows distributions to include multiple theme options.  Users can change the currently active theme set in the "UI Settings" menu.  The option is only visible if at least one theme set exists.

There are two places ES can load theme sets from:
* `[HOME]/.emulationstation/themes/[CURRENT_THEME_SET]/[SYSTEM_THEME]/theme.xml`
* `/etc/emulationstation/themes/[CURRENT_THEME_SET]/[SYSTEM_THEME]/theme.xml`

`[SYSTEM_THEME]` is the `<theme>` tag for the system, as defined in `es_systems.cfg`.  If the `<theme>` tag is not set, ES will use the system's `<name>`.

If both files happen to exist, ES will pick the first one (the one located in the home directory).

Again, the `[CURRENT_THEME_SET]` value is set in the "UI Settings" menu.  If it has not been set yet or the previously selected theme set is missing, the first available theme set will be used as the default.

Simple Example
==============

Here is a very simple theme that changes the description text's color:

```xml
<theme>
	<formatVersion>3</formatVersion>
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

**The `<formatVersion>` tag *must* be specified**.  This is the version of the theming system the theme was designed for.  The current version is 3.



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

"Extra" elements will be drawn in the order they are defined (so define backgrounds first!).  When they get drawn relative to the pre-existing elements depends on the view.  Make sure "extra" element names do not clash with existing element names!  An easy way to protect against this is to just start all your extra element names with some prefix like "e_".



*Properties* control how a particular *element* looks - for example, its position, size, image path, etc.  The type of the property determines what kinds of values you can use.  You can read about the types below in the "Reference" section.  Properties are defined like this:

```xml
		<propertyNameHere>ValueHere</propertyNameHere>
```




Advanced Features
=================

It is recommended that if you are writing a theme you launch EmulationStation with the `--debug` and `--windowed` switches.  This way you can read error messages without having to check the log file.  You can also reload the current gamelist view and system view with `Ctrl-R` if `--debug` is specified.

### The `<include>` tag

You can include theme files within theme files, similar to `#include` in C (though the internal mechanism is different, the effect is the same).  Example:

`~/.emulationstation/all_themes.xml`:
```xml
<theme>
	<formatVersion>3</formatVersion>
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
	<formatVersion>3</formatVersion>
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
	<formatVersion>3</formatVersion>
	<view name="detailed">
		<text name="description">
			<fontPath>./all_themes/myfont.ttf</fontPath>
			<color>FF0000</color>
		</text>
	</view>
</theme>
```

Notice that properties that were not specified got merged (`<fontPath>`) and the `snes/theme.xml` could overwrite the included files' values (`<color>`).  Also notice the included file still needed the `<formatVersion>` tag.



### Theming multiple views simultaneously

Sometimes you want to apply the same properties to the same elements across multiple views.  The `name` attribute actually works as a list (delimited by any characters of `\t\r\n ,` - that is, whitespace and commas).  So, for example, to easily apply the same header to the basic, grid, and system views:

```xml
<theme>
	<formatVersion>3</formatVersion>
	<view name="basic, grid, system">
		<image name="logo">
			<path>./snes_art/snes_header.png</path>
		</image>
	</view>
	<view name="detailed">
		<image name="logo">
			<path>./snes_art/snes_header_detailed.png</path>
		</image>
	</view>
</theme>
```

This is equivalent to:
```xml
<theme>
	<formatVersion>3</formatVersion>
	<view name="basic">
		<image name="logo">
			<path>./snes_art/snes_header.png</path>
		</image>
	</view>
	<view name="detailed">
		<image name="logo">
			<path>./snes_art/snes_header_detailed.png</path>
		</image>
	</view>
	<view name="grid">
		<image name="logo">
			<path>./snes_art/snes_header.png</path>
		</image>
	</view>
	<view name="system">
		<image name="logo">
			<path>./snes_art/snes_header.png</path>
		</image>
	</view>
	... and any other view that might try to look up "logo" ...
</theme>
```



### Theming multiple elements simultaneously

You can theme multiple elements *of the same type* simultaneously.  The `name` attribute actually works as a list (delimited by any characters of `\t\r\n ,` - that is, whitespace and commas), just like it does for views, as long as the elements have the same type.  This is useful if you want to, say, apply the same color to all the metadata labels:

```xml
<theme>
    <formatVersion>3</formatVersion>
    <view name="detailed">
    	<!-- Weird spaces/newline on purpose! -->
    	<text name="md_lbl_rating, md_lbl_releasedate, md_lbl_developer, md_lbl_publisher, 
    	md_lbl_genre,    md_lbl_players,        md_lbl_lastplayed, md_lbl_playcount">
        	<color>48474D</color>
        </text>
    </view>
</theme>
```

Which is equivalent to:
```xml
<theme>
    <formatVersion>3</formatVersion>
    <view name="detailed">
    	<text name="md_lbl_rating">
    		<color>48474D</color>
    	</text>
    	<text name="md_lbl_releasedate">
    		<color>48474D</color>
    	</text>
    	<text name="md_lbl_developer">
    		<color>48474D</color>
    	</text>
    	<text name="md_lbl_publisher">
    		<color>48474D</color>
    	</text>
    	<text name="md_lbl_genre">
    		<color>48474D</color>
    	</text>
    	<text name="md_lbl_players">
    		<color>48474D</color>
    	</text>
    	<text name="md_lbl_lastplayed">
    		<color>48474D</color>
    	</text>
    	<text name="md_lbl_playcount">
    		<color>48474D</color>
    	</text>
    	<text name="md_lbl_favorite">
    		<color>48474D</color>
    	</text>
    </view>
</theme>
```

Just remember, *this only works if the elements have the same type!*


Reference
=========

## Views, their elements, and themable properties:

#### basic
* `helpsystem name="help"` - ALL
	- The help system style for this view.
* `image name="background"` - ALL
	- This is a background image that exists for convenience. It goes from (0, 0) to (1, 1).
* `text name="logoText"` - ALL
	- Displays the name of the system.  Only present if no "logo" image is specified.  Displayed at the top of the screen, centered by default.
* `image name="logo"` - ALL
	- A header image.  If a non-empty `path` is specified, `text name="headerText"` will be hidden and this image will be, by default, displayed roughly in its place.
* `textlist name="gamelist"` - ALL
	- The gamelist.  `primaryColor` is for games, `secondaryColor` is for folders.  Centered by default.

---

#### detailed
* `helpsystem name="help"` - ALL
	- The help system style for this view.
* `image name="background"` - ALL
	- This is a background image that exists for convenience. It goes from (0, 0) to (1, 1).
* `text name="logoText"` - ALL
	- Displays the name of the system.  Only present if no "logo" image is specified.  Displayed at the top of the screen, centered by default.
* `image name="logo"` - ALL
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
      * `text name="md_lbl_favorite"` - ALL

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
		* `text name="md_favorite"` - ALL
			- The "favorite" metadata (is this game a favorite).
		* `text name="md_description"` - POSITION | SIZE | FONT_PATH | FONT_SIZE | COLOR
			- Text is the "desc" metadata.  If no `pos`/`size` is specified, will move and resize to fit under the lowest label and reach to the bottom of the screen.

---

#### grid
* `helpsystem name="help"` - ALL
	- The help system style for this view.
* `image name="background"` - ALL
	- This is a background image that exists for convenience. It goes from (0, 0) to (1, 1).
* `text name="logoText"` - ALL
	- Displays the name of the system.  Only present if no "logo" image is specified.  Displayed at the top of the screen, centered by default.
* `image name="logo"` - ALL
	- A header image.  If a non-empty `path` is specified, `text name="headerText"` will be hidden and this image will be, by default, displayed roughly in its place.

---

#### system
* `helpsystem name="help"` - ALL
	- The help system style for this view.
* `image name="logo"` - PATH
	- A logo image, to be displayed in the system logo carousel.
* You can use extra elements (elements with `extra="true"`) to add your own backgrounds, etc.  They will be displayed behind the carousel, and scroll relative to the carousel.


## Types of properties:

* NORMALIZED_PAIR - two decimals, in the range [0..1], delimited by a space.  For example, `0.25 0.5`.  Most commonly used for position (x and y coordinates) and size (width and height).
* PATH - a path.  If the first character is a `~`, it will be expanded into the environment variable for the home path (`$HOME` for Linux or `%HOMEPATH%` for Windows).  If the first character is a `.`, it will be expanded to the theme file's directory, allowing you to specify resources relative to the theme file, like so: `./../general_art/myfont.ttf`.
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
	- Multiply each pixel's color by this color. For example, an all-white image with `<color>FF0000</color>` would become completely red.  You can also control the transparency of an image with `<color>FFFFFFAA</color>` - keeping all the pixels their normal color and only affecting the alpha channel.

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
* `alignment` - type: STRING.
	- Valid values are "left", "center", or "right".  Controls alignment on the X axis.  "center" will also align vertically.
* `forceUppercase` - type: BOOLEAN.  Draw text in uppercase.
* `lineSpacing` - type: FLOAT.  Controls the space between lines (as a multiple of font height).  Default is 1.5.

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
* `forceUppercase` - type: BOOLEAN.  Draw text in uppercase.
* `lineSpacing` - type: FLOAT.  Controls the space between lines (as a multiple of font height).  Default is 1.5.

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
* `forceUppercase` - type: BOOLEAN.  Draw text in uppercase.

#### sound

* `path` - type: PATH.
	- Path to the sound file.  Only .wav files are currently supported.

#### helpsystem

* `pos` - type: NORMALIZED_PAIR.  Default is "0.012 0.9515"
* `textColor` - type: COLOR.  Default is 777777FF.
* `iconColor` - type: COLOR.  Default is 777777FF.
* `fontPath` - type: PATH.
* `fontSize` - type: FLOAT.

The help system is a special element that displays a context-sensitive list of actions the user can take at any time.  You should try and keep the position constant throughout every screen.  Keep in mind the "default" settings (including position) are used whenever the user opens a menu.

[*Check out the "official" themes for some more examples!*](http://aloshi.com/emulationstation#themes)
